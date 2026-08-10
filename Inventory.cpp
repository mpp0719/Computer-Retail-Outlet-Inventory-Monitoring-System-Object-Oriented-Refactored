#include "Inventory.h"
#include "Database.h"
#include "Utils.h"
#include "ansi.h"
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <iostream>
#include <iomanip>
#include <regex>
#include <cmath>
#include <chrono>
#include <sstream>
#include <ctime>
#include <fstream>
#include <vector>

using namespace std;

string InventoryService::createInvoiceRecord(double subtotal, const string& supplierId)
{
    sql::Connection* con = Database::instance().getConnection();
    string invoiceId, invoiceDate;
    try
    {
        sql::Statement* stmt = con->createStatement();
        sql::ResultSet* res = stmt->executeQuery("select * from invoice");
        int idcount = 0;
        while (res->next())
        {
            if (res->isLast())
            {
                string input = res->getString("invoiceId");
                string output = regex_replace(input, regex("[^0-9]*([0-9]+)"), string("$1"));
                idcount = stoi(output);
            }
        }
        if ((int)log10(idcount + 1) == 0) invoiceId = "V000";
        else if ((int)log10(idcount + 1) == 1) invoiceId = "V00";
        else if ((int)log10(idcount + 1) == 2) invoiceId = "V0";
        else invoiceId = "V";
        invoiceId += to_string(idcount + 1);

        auto now = chrono::system_clock::now();
        auto time_t_now = chrono::system_clock::to_time_t(now);
        tm tm;
        errno_t err = localtime_s(&tm, &time_t_now);
        if (err != 0)
            cerr << "Error getting local time" << endl;
        stringstream ss;
        ss << put_time(&tm, "%Y-%m-%d");
        invoiceDate = ss.str();

        sql::PreparedStatement* pstmt = con->prepareStatement("insert into invoice value (?, ?, ?, ?)");
        pstmt->setString(1, invoiceId);
        pstmt->setString(2, invoiceDate);
        pstmt->setDouble(3, subtotal);
        pstmt->setString(4, supplierId);
        pstmt->executeUpdate();
    }
    catch (sql::SQLException& e)
    {
        cout << "Error creating invoice: " << e.what() << "\n";
    }
    return invoiceId;
}

void InventoryService::createInventoryRecord(const string& invoiceId, const string& orderDetailId, int receiveQty)
{
    sql::Connection* con = Database::instance().getConnection();
    string receiveId;
    try
    {
        sql::Statement* stmt = con->createStatement();
        sql::ResultSet* res = stmt->executeQuery("select * from inventory");
        int idcount = 0;
        while (res->next())
        {
            if (res->isLast())
            {
                string input = res->getString("receiveId");
                string output = regex_replace(input, regex("[^0-9]*([0-9]+)"), string("$1"));
                idcount = stoi(output);
            }
        }
        if ((int)log10(idcount + 1) == 0) receiveId = "L000";
        else if ((int)log10(idcount + 1) == 1) receiveId = "L00";
        else if ((int)log10(idcount + 1) == 2) receiveId = "l0"; // preserved from original (lowercase 'l')
        else receiveId = "L";
        receiveId += to_string(idcount + 1);

        sql::PreparedStatement* pstmt = con->prepareStatement("insert into inventory value (?, ?, ?, ?)");
        pstmt->setString(1, receiveId);
        pstmt->setInt(2, receiveQty);
        pstmt->setString(3, orderDetailId);
        pstmt->setString(4, invoiceId);
        pstmt->executeUpdate();
    }
    catch (sql::SQLException& e)
    {
        cerr << "Error creating inventory: " << e.what() << "\n";
    }
}

void InventoryService::generateInvoiceFile(const string& orderId, const string& invoiceId)
{
    sql::Connection* con = Database::instance().getConnection();
    try
    {
        ofstream rfile("Invoices\\invoice_" + invoiceId + ".txt");
        if (!rfile.is_open())
        {
            cerr << "Error creating invoice file!" << endl;
            return;
        }
        sql::PreparedStatement* pstmt = con->prepareStatement(
            "select * from invoice join inventory using (invoiceId) join supplier using (supplierId) join order_detail using (orderDetailId) join order_stock using (orderId) join staff using (staffId) where orderId = ?");
        pstmt->setString(1, orderId);
        sql::ResultSet* res1 = pstmt->executeQuery();
        if (!res1->next())
        {
            cout << "Order stock not found!" << endl;
            return;
        }
        pstmt = con->prepareStatement(
            "select * from order_detail join product using (productId) join inventory using (orderDetailId) join invoice using (invoiceId) where invoiceId = ?");
        pstmt->setString(1, invoiceId);
        sql::ResultSet* res2 = pstmt->executeQuery();

        rfile << fixed << setprecision(2);
        rfile << "----------------------------------------------------------------------------------\n";
        rfile << "|          COMPUTER RETAIL OUTLET INVENTORY MONITORING SYSTEM - INVOICE          |\n";
        rfile << "----------------------------------------------------------------------------------\n";
        rfile << "| Invoice  :" << left << setw(54) << invoiceId << "               |\n";
        rfile << "| Order ID :" << left << setw(54) << orderId << "               |\n";
        rfile << "| Date     :" << left << setw(54) << res1->getString("orderDate") << "               |\n";
        rfile << "| Supplier :" << left << setw(54) << res1->getString("supplierName") << "               |\n";
        rfile << "| Staff    :" << left << setw(54) << res1->getString("staffName") << "               |\n";
        rfile << "----------------------------------------------------------------------------------\n";
        rfile << "|                                 ITEM DETAILS                                   |\n";
        rfile << "----------------------------------------------------------------------------------\n";
        rfile << "| No |             Product Description                  | Qty | Arr | Amount     |\n";
        rfile << "----------------------------------------------------------------------------------\n";
        int counter = 1;
        double subtotal = 0;
        while (res2->next())
        {
            string productName = res2->getString("productName");
            int qtt = res2->getInt("orderQuantity");
            int arr = res2->getInt("receiveQuantity");
            double unitPrice = res2->getDouble("sellingPrice");
            double total = res2->getInt("receiveQuantity") * unitPrice;
            subtotal += total;
            rfile << "| " << setw(2) << right << counter++ << " | " << left << setw(48) << productName << " | " << setw(2) << right << qtt << "  | " << setw(2) << right << arr << "  | " << "RM" << setw(8) << left << total << " |\n";
            rfile << "|    |   Unit: RM" << setw(30) << left << unitPrice << "         |     |     |            |\n";
        }
        rfile << "----------------------------------------------------------------------------------\n";
        rfile << "|                                                                                |\n";
        rfile << "| Subtotal:                                                         RM" << setw(10) << subtotal << " |\n";
        rfile << "|                                                                                |\n";
        rfile << "----------------------------------------------------------------------------------\n";
        rfile << "\n";
        rfile << "  Thank you for ordering stock from us!\n  For refunds, present this invoice to us via email.\n\n";
        rfile << "----------------------------------------------------------------------------------\n";
        rfile.close();
        cout << "Invoice Saved.\n";
    }
    catch (sql::SQLException& e)
    {
        cerr << "Error creating invoice: " << e.what() << endl;
    }
}

void InventoryService::viewInventory()
{
    sql::Connection* con = Database::instance().getConnection();
    int c = -1, cpage = 1, pagesize = 10, totalp = 0;
    try
    {
        string combine = "select * from invoice join supplier using (supplierId) order by invoiceId asc";
        sql::PreparedStatement* pstmt = con->prepareStatement(combine);
        sql::ResultSet* res = pstmt->executeQuery();
        int totalrow = res->rowsCount();
        totalp = (totalrow + pagesize - 1) / pagesize;
        while (true)
        {
            int offset = (cpage - 1) * pagesize;
            string pagequery = combine + " limit " + to_string(pagesize) + " offset " + to_string(offset);
            pstmt = con->prepareStatement(pagequery);
            res = pstmt->executeQuery();
            int counter = 0;
            system("cls");
            cout << bocyan << u8"                                ╔═════════════════╗                           \n" << reset;
            cout << bocyan << u8"════════════════════════════════╣ LIST OF INVOICE ╠════════════════════════════════\n" << reset;
            cout << bocyan << u8"                                ╚═════════════════╝                           \n" << reset;
            cout << endl;
            cout << bocyan << u8"════════════════════════════════════════════════════════════════════════════════\n" << reset;
            cout << " (Page " << cpage << " of " << totalp << ")" << endl;
            cout << bocyan << u8"╒═════╤════════════╤═══════════════╤════════════╤══════════════════════════════╕\n" << reset;
            cout << bocyan << u8"│" << reset << left << setw(5) << "No. ";
            cout << bocyan << u8"│" << reset << left << setw(12) << "Invoice ID";
            cout << bocyan << u8"│" << reset << left << setw(15) << "Date";
            cout << bocyan << u8"│" << reset << left << setw(12) << "Subtotal";
            cout << bocyan << u8"│" << reset << left << setw(30) << "Supplier Name";
            cout << bocyan << u8"│" << reset << endl;
            cout << bocyan << u8"╞═════╪════════════╪═══════════════╪════════════╪══════════════════════════════╡\n" << reset;
            while (res->next())
            {
                cout << bocyan << u8"│" << reset << left << setw(5) << counter + 1;
                cout << bocyan << u8"│" << reset << left << setw(12) << res->getString("invoiceId");
                cout << bocyan << u8"│" << reset << left << setw(15) << res->getString("invoiceDate");
                cout << bocyan << u8"│" << reset << left << setw(12) << res->getDouble("subtotal");
                cout << bocyan << u8"│" << reset << left << setw(30) << res->getString("supplierName");
                cout << bocyan << u8"│" << reset << endl;
                counter++;
            }
            cout << bocyan << u8"╘═════╧════════════╧═══════════════╧════════════╧══════════════════════════════╛\n" << reset;
            if (cpage > 1)
                cout << "1. Previous page\n";
            if (cpage < totalp)
                cout << "2. Next page\n";
            cout << "0. Stop filtering\nEnter choice: ";
            int pagechoice;
            cin >> pagechoice;
            pagechoice = Utils::integerCheck(pagechoice);
            while (pagechoice > 2 || pagechoice < 0)
            {
                cerr << "Invalid choice. Try again: ";
                cin >> pagechoice;
                pagechoice = Utils::integerCheck(pagechoice);
            }
            if (pagechoice == 0)
                break;
            else if (pagechoice == 1 && cpage > 1)
                cpage--;
            else if (pagechoice == 2 && cpage < totalp)
                cpage++;
        }
        cout << "Enter No to view detail(0 to cancel): ";
        cin >> c;
        c = Utils::integerCheck(c);
        while (c > res->rowsCount() || c < 0)
        {
            cerr << "Invalid input. Try again: ";
            cin >> c;
            c = Utils::integerCheck(c);
        }
        if (c == 0)
            return;
        int counter = 0;
        res->beforeFirst();
        do { res->next(); counter++; } while (counter != c);

        string invoiceId = res->getString("invoiceId");
        system("cls");
        cout << "Inventory detail for invoice\t: " << invoiceId << "\t\t\tDate\t: " << res->getString("invoiceDate");
        pstmt = con->prepareStatement(
            "select * from inventory join order_detail using (orderDetailId) join product using (productId) join invoice using (invoiceId) where invoiceId = ?");
        pstmt->setString(1, invoiceId);
        res = pstmt->executeQuery();
        cout << endl;
        cout << bocyan << u8"╒════════════╤════════════════════════════════════════════════╤══════════════════╤══════════════════╤═══════════════╕\n" << reset;
        cout << bocyan << u8"│" << reset << left << setw(12) << "Product ID";
        cout << bocyan << u8"│" << reset << left << setw(48) << "Product Name";
        cout << bocyan << u8"│" << reset << left << setw(18) << "Order Quantity";
        cout << bocyan << u8"│" << reset << left << setw(18) << "Receive Quantity";
        cout << bocyan << u8"│" << reset << left << setw(15) << "Item Total";
        cout << bocyan << u8"│" << reset << endl;
        cout << bocyan << u8"╞════════════╪════════════════════════════════════════════════╪══════════════════╪══════════════════╪═══════════════╡\n" << reset;
        while (res->next())
        {
            cout << bocyan << u8"│" << reset << left << setw(12) << res->getString("productId");
            cout << bocyan << u8"│" << reset << left << setw(48) << res->getString("productName");
            cout << bocyan << u8"│" << reset << left << setw(18) << res->getInt("orderQuantity");
            cout << bocyan << u8"│" << reset << left << setw(18) << res->getString("receiveQuantity");
            cout << bocyan << u8"│" << reset << left << setw(15) << res->getDouble("itemTotal");
            cout << bocyan << u8"│" << reset << endl;
        }
        cout << bocyan << u8"╘════════════╧════════════════════════════════════════════════╧══════════════════╧══════════════════╧═══════════════╛\n" << reset;
    }
    catch (sql::SQLException& e)
    {
        cerr << "Error displaying invoice: " << e.what() << endl;
    }
    system("pause");
}

void InventoryService::notifyLowStock()
{
    sql::Connection* con = Database::instance().getConnection();
    try
    {
        sql::Statement* stmt = con->createStatement();
        sql::ResultSet* res = stmt->executeQuery("select * from product where quantity <= 5");
        if (!res->next())
            cout << green << "[ ] NO NOTIFICATIONS ~ \n\n" << reset;
        else
        {
            cout << red << "[!] NOTIFICATIONS\n\n" << reset;
            res->beforeFirst();
            while (res->next())
            {
                cout << "[" << res->getString("productId") << "]  " << left << setw(48) << res->getString("productName") << " - ";
                if (res->getInt("quantity") <= 1)
                    cout << "Critical Stock (" << res->getInt("quantity") << " units left)";
                else
                    cout << "Low Stock (" << res->getInt("quantity") << " units left)";
                cout << endl;
            }
        }
        cout << "---------------------------------------------------------------------------------------\n\n";
    }
    catch (sql::SQLException& e)
    {
        cerr << "Error " << endl;
    }
}

void InventoryService::receiveStock()
{
    sql::Connection* con = Database::instance().getConnection();
    string orderId, supplierId;
    double subtotal = 0;
    int counter = 0, c = -1;
    vector<ReceivingItem> receiving;
    try
    {
        system("cls");
        cout << bocyan << u8"                               ╔═════════════════╗                           \n" << reset;
        cout << bocyan << u8"═══════════════════════════════╣ STOCK RECEIVING ╠═══════════════════════════════\n" << reset;
        cout << bocyan << u8"                               ╚═════════════════╝                           \n" << reset;
        sql::Statement* stmt = con->createStatement();
        sql::ResultSet* res = stmt->executeQuery(
            "select *, order_detail.status as \"orderStatus\" from order_stock join order_detail using (orderId) join supplier using (supplierId) join product using (productId) where order_detail.status = false");
        if (res->next())
        {
            string temp = "";
            cout << endl;
            cout << bocyan << u8"╒════╤══════════╤════════════╤══════════════════════════════╤════════════════════════════════════════════════╤══════════════════╤════════════╤════════╕\n" << reset;
            cout << bocyan << u8"│" << reset << left << setw(4) << "No.";
            cout << bocyan << u8"│" << reset << left << setw(10) << "Order ID";
            cout << bocyan << u8"│" << reset << left << setw(12) << "Order Date";
            cout << bocyan << u8"│" << reset << left << setw(30) << "Supplier Name";
            cout << bocyan << u8"│" << reset << left << setw(48) << "Product Name";
            cout << bocyan << u8"│" << reset << left << setw(18) << "Order Quantity";
            cout << bocyan << u8"│" << reset << left << setw(12) << "Remaining";
            cout << bocyan << u8"│" << reset << left << setw(8) << "Status";
            cout << bocyan << u8"│" << reset << endl;
            cout << bocyan << u8"╞════╪══════════╪════════════╪══════════════════════════════╪════════════════════════════════════════════════╪══════════════════╪════════════╪════════╡\n" << reset;
            do
            {
                if (res->getString("orderId") != temp)
                {
                    if (counter > 0)
                        cout << bocyan << u8"│" << reset << left << setw(4) << " "
                        << bocyan << u8"│" << reset << left << setw(10) << " "
                        << bocyan << u8"│" << reset << left << setw(12) << " "
                        << bocyan << u8"│" << reset << left << setw(30) << " "
                        << bocyan << u8"│" << reset << left << setw(48) << " "
                        << bocyan << u8"│" << reset << left << setw(18) << " "
                        << bocyan << u8"│" << reset << left << setw(12) << " "
                        << bocyan << u8"│" << reset << left << setw(8) << " "
                        << bocyan << u8"│" << reset << endl;
                    cout << bocyan << u8"│" << reset << left << setw(4) << counter + 1;
                    cout << bocyan << u8"│" << reset << left << setw(10) << res->getString("orderId");
                    temp = res->getString("orderId");
                    cout << bocyan << u8"│" << reset << left << setw(12) << res->getString("orderDate");
                    cout << bocyan << u8"│" << reset << left << setw(30) << res->getString("supplierName");
                    counter++;
                }
                else
                {
                    cout << bocyan << u8"│" << reset << left << setw(4) << " "
                        << bocyan << u8"│" << reset << left << setw(10) << " "
                        << bocyan << u8"│" << reset << left << setw(12) << " "
                        << bocyan << u8"│" << reset << left << setw(30) << " ";
                }
                cout << bocyan << u8"│" << reset << left << setw(48) << res->getString("productName");
                cout << bocyan << u8"│" << reset << left << setw(18) << res->getInt("orderQuantity");
                cout << bocyan << u8"│" << reset << left << setw(12) << res->getInt("remainingQuantity");
                if (res->getInt("orderStatus") == 0)
                    cout << bocyan << u8"│" << reset << left << setw(8) << "Pending";
                else
                    cout << bocyan << u8"│" << reset << left << setw(8) << "Completed";
                cout << bocyan << u8"│" << reset << endl;
            } while (res->next());
            cout << bocyan << u8"╘════╧══════════╧════════════╧══════════════════════════════╧════════════════════════════════════════════════╧══════════════════╧════════════╧════════╛\n" << reset;
        }
        else
            cout << "No pending order.\n";

        cout << "\nPlease choose which order's shipment have arrived(0 to cancel): ";
        cin >> c;
        c = Utils::integerCheck(c);
        while (c > counter || c < 0)
        {
            cerr << "Invalid input. Try again: ";
            cin >> c;
            c = Utils::integerCheck(c);
        }
        if (c == 0)
            return;
        counter = 0;
        res->beforeFirst();
        if (res->next())
        {
            string current = res->getString("orderId");
            if (c == 1)
            {
            }
            else
            {
                do
                {
                    string temp = current;
                    while (res->next() && res->getString("orderId") == temp) {}
                    if (!res->isAfterLast())
                    {
                        current = res->getString("orderId");
                        counter++;
                    }
                } while (counter < (c - 1) && !res->isAfterLast());
            }
        }
        orderId = res->getString("orderId");
        supplierId = res->getString("supplierId");

        sql::PreparedStatement* pstmt = con->prepareStatement(
            "select * from order_detail join product using (productId) where orderId = ? and order_detail.status = 0");
        pstmt->setString(1, orderId);
        res = pstmt->executeQuery();

        while (true)
        {
            string displayinvoiceid;
            int idcount = -1;
            stmt = con->createStatement();
            sql::ResultSet* res1 = stmt->executeQuery("select * from invoice");
            while (res1->next())
            {
                if (res1->isLast())
                {
                    string input = res1->getString("invoiceId");
                    string output = regex_replace(input, regex("[^0-9]*([0-9]+)"), string("$1"));
                    idcount = stoi(output);
                }
            }
            if ((int)log10(idcount + 1) == 0) displayinvoiceid = "V000";
            else if ((int)log10(idcount + 1) == 1) displayinvoiceid = "V00";
            else if ((int)log10(idcount + 1) == 2) displayinvoiceid = "V0";
            else displayinvoiceid = "V";
            displayinvoiceid += to_string(idcount + 1);

            res->beforeFirst();
            counter = 0;
            system("cls");
            cout << "Creating invoice: " << displayinvoiceid << "...\nOrder ID: " << orderId << "\n";
            cout << bocyan << u8"╒═════╤════════════════════════════════════════════════╤════════════════════╤════════════╕\n" << reset;
            cout << bocyan << u8"│" << reset << left << setw(5) << "No.";
            cout << bocyan << u8"│" << reset << left << setw(48) << "Product Name";
            cout << bocyan << u8"│" << reset << left << setw(20) << "Remaining Quantity";
            cout << bocyan << u8"│" << reset << left << setw(12) << "Received";
            cout << bocyan << u8"│" << reset << endl;
            cout << bocyan << u8"╞═════╪════════════════════════════════════════════════╪════════════════════╪════════════╡\n" << reset;
            vector<int> temparr;
            while (res->next())
            {
                bool found = false;
                cout << bocyan << u8"│" << reset << left << setw(5) << counter + 1;
                cout << bocyan << u8"│" << reset << left << setw(48) << res->getString("productName");
                temparr.push_back(res->getInt("remainingQuantity"));
                for (const auto& item : receiving)
                {
                    if (item.productId == res->getString("productId"))
                    {
                        cout << green;
                        temparr[counter] -= item.receivedQty;
                        cout << bocyan << u8"│" << reset << left << setw(20) << temparr[counter];
                        cout << bocyan << u8"│" << reset << left << setw(12) << item.receivedQty;
                        cout << reset;
                        found = true;
                        break;
                    }
                }
                if (!found)
                {
                    cout << bocyan << u8"│" << reset << left << setw(20) << temparr[counter];
                    cout << bocyan << u8"│" << reset << left << setw(12) << "0";
                }
                cout << bocyan << u8"│" << reset << endl;
                counter++;
            }
            cout << bocyan << u8"╘═════╧════════════════════════════════════════════════╧════════════════════╧════════════╛\n" << reset;
            counter = 0;
            cout << "\nPlease choose which item(s) have arrived(0 to validate, -1 to cancel): ";
            cin >> c;
            c = Utils::integerCheck(c);
            if (c == 0)
                break;
            if (c == -1)
                return;
            while (c > res->rowsCount() || c < -1)
            {
                cerr << "Invalid input. Try again: ";
                cin >> c;
                c = Utils::integerCheck(c);
            }
            counter = 0;
            res->beforeFirst();
            do { res->next(); counter++; } while (counter != c);

            bool repeat = false;
            for (auto& item : receiving)
            {
                if (item.orderDetailId == res->getString("orderDetailId"))
                {
                    repeat = true;
                    cout << "Item is selected. Proceed to update received quantity...";
                    cout << "\nUpdate received quantity: ";
                    cin >> item.receivedQty;
                    item.receivedQty = Utils::integerCheck(item.receivedQty);
                    while (item.receivedQty > res->getInt("remainingQuantity"))
                    {
                        cerr << "Invalid input. Try again: ";
                        cin >> item.receivedQty;
                        item.receivedQty = Utils::integerCheck(item.receivedQty);
                    }
                }
            }
            if (!repeat)
            {
                ReceivingItem item;
                item.orderDetailId = res->getString("orderDetailId");
                item.productId = res->getString("productId");
                item.remainingQuantity = res->getInt("remainingQuantity");
                cout << "Enter received quantity: ";
                cin >> item.receivedQty;
                item.receivedQty = Utils::integerCheck(item.receivedQty);
                while (item.receivedQty > res->getInt("remainingQuantity"))
                {
                    cerr << "Invalid input. Try again: ";
                    cin >> item.receivedQty;
                    item.receivedQty = Utils::integerCheck(item.receivedQty);
                }
                receiving.push_back(item);
            }
        }

        for (const auto& item : receiving)
        {
            pstmt = con->prepareStatement("select basePrice from product where productId = ?");
            pstmt->setString(1, item.productId);
            res = pstmt->executeQuery();
            res->next();
            subtotal += res->getDouble("basePrice") * item.receivedQty;
        }

        string invoiceId = createInvoiceRecord(subtotal, supplierId);
        for (const auto& item : receiving)
        {
            createInventoryRecord(invoiceId, item.orderDetailId, item.receivedQty);
            pstmt = con->prepareStatement("update order_detail set remainingQuantity = ? , status = ? where orderDetailId = ?");
            pstmt->setInt(1, item.remainingQuantity - item.receivedQty);
            pstmt->setBoolean(2, (item.remainingQuantity - item.receivedQty) == 0);
            pstmt->setString(3, item.orderDetailId);
            pstmt->executeUpdate();
            productService.updateProduct(item.productId, true, 2, item.receivedQty);
        }

        system("cls");
        generateInvoiceFile(orderId, invoiceId);
        cout << "\nInventory auto updated!\n";
        cout << "Updated product detail:\n";
        cout << bocyan << u8"╒═════════════╤════════════════════════════════════════════════╤════════════════════╤════════╤══════╕\n" << reset;
        cout << bocyan << u8"│" << reset << left << setw(13) << "Product ID";
        cout << bocyan << u8"│" << reset << left << setw(48) << "Product Name";
        cout << bocyan << u8"│" << reset << left << setw(20) << "Product Type";
        cout << bocyan << u8"│" << reset << left << setw(8) << "Before";
        cout << bocyan << u8"│" << reset << left << setw(6) << "After" << bocyan << u8"│" << reset << endl;
        cout << bocyan << u8"╞═════════════╪════════════════════════════════════════════════╪════════════════════╪════════╪══════╡\n" << reset;
        pstmt = con->prepareStatement("select * from product where productId = ?");
        for (const auto& item : receiving)
        {
            pstmt->setString(1, item.productId);
            res = pstmt->executeQuery();
            res->next();
            cout << bocyan << u8"│" << reset << left << setw(13) << res->getString("productId");
            cout << bocyan << u8"│" << reset << left << setw(48) << res->getString("productName");
            cout << bocyan << u8"│" << reset << left << setw(20) << res->getString("productType");
            cout << bocyan << u8"│" << reset << left << setw(8) << res->getInt("quantity") - item.receivedQty;
            cout << bocyan << u8"│" << reset << left << setw(6) << res->getInt("quantity") << bocyan << u8"│" << reset << endl;
        }
        cout << bocyan << u8"╘═════════════╧════════════════════════════════════════════════╧════════════════════╧════════╧══════╛\n" << reset;
        system("pause");
        reportService.showInvoiceFile(invoiceId);
    }
    catch (sql::SQLException& e)
    {
        cerr << "Error retrieving orderstock data: " << e.what() << endl;
    }
}