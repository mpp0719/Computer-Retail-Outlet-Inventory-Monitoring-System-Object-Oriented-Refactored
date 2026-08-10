#include "Stockorder/StockOrder.h"
#include "Core/Database.h"
#include "Core/Utils.h"
#include "Staff/Staff.h"
#include "External/ansi.h"
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

using namespace std;

void StockOrderService::printCart(const Cart& cart)
{
    sql::Connection* con = Database::instance().getConnection();
    double subtotal = 0;
    cout << endl;
    cout << bocyan << u8"                                                       ╔══════════════════╗                           \n" << reset;
    cout << bocyan << u8"═══════════════════════════════════════════════════════╣ STOCK ORDER CART ╠═══════════════════════════════════════════════════════\n" << reset;
    cout << bocyan << u8"                                                       ╚══════════════════╝                           \n" << reset;
    cout << magenta << u8"╒════╤═════════════╤════════════════════════════════════════════════╤════════════════════╤═════════════╤════════════╤════════════╕\n" << reset;
    cout << magenta << u8"│" << reset << left << setw(4) << "No.";
    cout << magenta << u8"│" << reset << left << setw(13) << "Product ID";
    cout << magenta << u8"│" << reset << left << setw(48) << "Product Name";
    cout << magenta << u8"│" << reset << left << setw(20) << "Product Type";
    cout << magenta << u8"│" << reset << left << setw(13) << "Stock Price";
    cout << magenta << u8"│" << reset << left << setw(12) << "Quantity";
    cout << magenta << u8"│" << reset << left << setw(12) << "Item total" << magenta << u8"│" << reset << endl;
    cout << magenta << u8"╞════╪═════════════╪════════════════════════════════════════════════╪════════════════════╪═════════════╪════════════╪════════════╡\n" << reset;
    if (!cart.empty())
    {
        for (size_t i = 0; i < cart.size(); i++)
        {
            sql::PreparedStatement* pstmt = con->prepareStatement(
                "select productId, productName, productType, basePrice from product where productId = ?");
            pstmt->setString(1, cart.at(i).productId);
            sql::ResultSet* res = pstmt->executeQuery();
            res->next();
            cout << magenta << u8"│" << reset << left << setw(4) << (i + 1);
            cout << magenta << u8"│" << reset << left << setw(13) << res->getString("productId");
            cout << magenta << u8"│" << reset << left << setw(48) << res->getString("productName");
            cout << magenta << u8"│" << reset << left << setw(20) << res->getString("productType");
            cout << magenta << u8"│" << reset << left << setw(13) << res->getDouble("basePrice");
            cout << magenta << u8"│" << reset << left << setw(12) << cart.at(i).quantity;
            cout << magenta << u8"│" << reset << left << setw(12) << (res->getDouble("basePrice") * cart.at(i).quantity) << magenta << u8"│" << reset << endl;
            subtotal += res->getDouble("basePrice") * cart.at(i).quantity;
        }
        cout << magenta << u8"╘════╧═════════════╧════════════════════════════════════════════════╧════════════════════╧═════════════╧════════════╧════════════╛\n" << reset;
        cout << right << setw(100) << "Subtotal: RM" << subtotal << endl;
    }
    else
    {
        cout << magenta << u8"╘════╧═════════════╧════════════════════════════════════════════════╧════════════════════╧═════════════╧════════════╧════════════╛\n" << reset;
        cout << "\n\tCart is empty.\n\n";
    }
}

void StockOrderService::updateCartInteractively(Cart& cart)
{
    system("cls");
    printCart(cart);
    cout << bocyan << u8"                                 ╔═══════════════════╗                           \n" << reset;
    cout << bocyan << u8"═════════════════════════════════╣ UPDATE STOCK CART ╠═════════════════════════════════\n" << reset;
    cout << bocyan << u8"                                 ╚═══════════════════╝                           \n" << reset;
    cout << "1. Remove Product\n2. Update Quantity\n0. Cancel Update\n";
    cout << bocyan << u8"═══════════════════════════════════════════════════════════════════════════════════════\n" << reset;
    int c;
    do
    {
        cout << "Enter your choice: ";
        cin >> c;
        c = Utils::integerCheck(c);
        cin.ignore();
        if (c == 1)
        {
            int remove;
            cout << "Enter the Product No. that you want to remove: ";
            cin >> remove;
            remove = Utils::integerCheck(remove);
            while (remove < 1 || remove >(int)cart.size())
            {
                cerr << "Invalid product number. Try again: ";
                cin >> remove;
                remove = Utils::integerCheck(remove);
            }
            cart.removeAt(remove - 1);
        }
        else if (c == 2)
        {
            int productNo, nquantity;
            cout << "Enter the Product No. that you want to update quantity: ";
            cin >> productNo;
            productNo = Utils::integerCheck(productNo);
            cout << "Enter new quantity: ";
            cin >> nquantity;
            nquantity = Utils::integerCheck(nquantity);
            while (nquantity < 1)
            {
                cerr << "Invalid new quantity. Try again: ";
                cin >> nquantity;
                nquantity = Utils::integerCheck(nquantity);
            }
            cart.at(productNo - 1).quantity = nquantity;
        }
        else if (c == 0)
            break;
        else
            cout << "Invalid choice. Please Re-";
    } while (c < 0 || c > 2);
    cout << "Cart updated.\n\n";
}

void StockOrderService::createOrderDetail(const string& productId, const string& orderId, int quantity, double inditotal)
{
    sql::Connection* con = Database::instance().getConnection();
    string orderDetailId;
    try
    {
        sql::Statement* stmt = con->createStatement();
        sql::ResultSet* res = stmt->executeQuery("select * from order_detail");
        int idcount = 0;
        while (res->next())
        {
            if (res->isLast())
            {
                string input = res->getString("orderDetailId");
                string output = regex_replace(input, regex("[^0-9]*([0-9]+)"), string("$1"));
                idcount = stoi(output);
            }
        }
        if ((int)log10(idcount + 1) == 0) orderDetailId = "D000";
        else if ((int)log10(idcount + 1) == 1) orderDetailId = "D00";
        else if ((int)log10(idcount + 1) == 2) orderDetailId = "D0";
        else orderDetailId = "D";
        orderDetailId += to_string(idcount + 1);

        sql::PreparedStatement* pstmt = con->prepareStatement("insert into order_detail value (?, ?, ?, ?, ?, ?, ?)");
        pstmt->setString(1, orderDetailId);
        pstmt->setInt(2, quantity);
        pstmt->setInt(3, quantity);
        pstmt->setBoolean(4, 0);
        pstmt->setDouble(5, inditotal);
        pstmt->setString(6, orderId);
        pstmt->setString(7, productId);
        pstmt->executeUpdate();
    }
    catch (sql::SQLException& e)
    {
        cout << "Error creating stock order detail: " << e.what() << endl;
    }
}

void StockOrderService::createOrder()
{
    sql::Connection* con = Database::instance().getConnection();
    int c;
    string oid, sodate, supplierId, supplierName;
    double sosubtotal = 0;
    Cart cart;
    sql::Statement* stmt = con->createStatement();
    try
    {
        system("cls");
        cout << bocyan << u8"                                 ╔═════════════════╗                           \n" << reset;
        cout << bocyan << u8"═════════════════════════════════╣ CHOOSE SUPPLIER ╠═════════════════════════════════\n" << reset;
        cout << bocyan << u8"                                 ╚═════════════════╝                           \n" << reset;
        supplierService.viewAll();
        cout << "\nPlease choose supplier u want to order stock from(No)(0 to cancel): ";
        cin >> c;
        c = Utils::integerCheck(c);

        sql::PreparedStatement* pstmt = con->prepareStatement("select * from supplier where status <> 0");
        sql::ResultSet* res = pstmt->executeQuery();
        while (c > res->rowsCount() || c < 0)
        {
            cerr << "Invalid input. Try again: ";
            cin >> c;
            c = Utils::integerCheck(c);
            res = pstmt->executeQuery();
        }
        if (c == 0)
            return;
        int counter = 0;
        do { res->next(); counter++; } while (counter != c);
        supplierId = res->getString("supplierId");
        supplierName = res->getString("supplierName");
        cin.ignore();

        int idcount = 0;
        res = stmt->executeQuery("select * from order_stock");
        while (res->next())
        {
            if (res->isLast())
            {
                string input = res->getString("orderId");
                string output = regex_replace(input, regex("[^0-9]*([0-9]+)"), string("$1"));
                idcount = stoi(output);
            }
        }
        if ((int)log10(idcount + 1) == 0) oid = "R000";
        else if ((int)log10(idcount + 1) == 1) oid = "R00";
        else if ((int)log10(idcount + 1) == 2) oid = "R0";
        else oid = "R";
        oid += to_string(idcount + 1);

        do
        {
            system("cls");
            printCart(cart);
            cout << "Selected supplier: " << supplierName << endl;
            cout << bocyan << u8"                                 ╔════════════════╗                           \n" << reset;
            cout << bocyan << u8"═════════════════════════════════╣ ORDERING STOCK ╠═════════════════════════════════\n" << reset;
            cout << bocyan << u8"                                 ╚════════════════╝                           \n" << reset;
            cout << "1. Add product to cart\n2. Update Cart\n3. Checkout\n0. Cancel\n";
            cout << "Enter your choice: ";
            cin >> c;
            c = Utils::integerCheck(c);
            cin.ignore();
            while (c < 0 || c > 4)
            {
                cerr << "Invalid input. Try again: ";
                cin >> c;
                c = Utils::integerCheck(c);
            }
            if (c == 1)
            {
                sql::ResultSet* pres = productService.searchForSelection();
                pres->beforeFirst();
                cout << bocyan << u8"                                 ╔═════════════════════╗                           \n" << reset;
                cout << bocyan << u8"═════════════════════════════════╣ ADD PRODUCT TO CART ╠═════════════════════════════════\n" << reset;
                cout << bocyan << u8"                                 ╚═════════════════════╝                           \n" << reset;
                int itemchoice = 0;
                cout << "Enter Product No.\t: ";
                cin >> itemchoice;
                itemchoice = Utils::integerCheck(itemchoice);
                while (itemchoice > pres->rowsCount() || itemchoice < 1)
                {
                    cerr << "Invalid input. Try again: ";
                    cin >> itemchoice;
                    itemchoice = Utils::integerCheck(itemchoice);
                }
                int cnt = 0;
                do { pres->next(); cnt++; } while (cnt != itemchoice);

                int existing = cart.find(pres->getString("productId"));
                if (existing >= 0)
                {
                    cout << "Item already in cart. Proceed to update quantity...\n";
                    cout << "Update order quantity: ";
                    cin >> cart.at(existing).quantity;
                    cart.at(existing).quantity = Utils::integerCheck(cart.at(existing).quantity);
                    cout << "Quantity updated.\n";
                    cart.at(existing).indiTotal = pres->getDouble("sellingPrice") * cart.at(existing).quantity;
                }
                else
                {
                    CartItem item;
                    item.productId = pres->getString("productId");
                    cout << "Enter quantity\t: ";
                    cin >> item.quantity;
                    item.quantity = Utils::integerCheck(item.quantity);
                    cout << "Product with ID: " << item.productId << " added to stock order cart.\n";
                    item.indiTotal = pres->getDouble("sellingPrice") * item.quantity;
                    cart.add(item);
                }
            }
            else if (c == 2)
            {
                updateCartInteractively(cart);
            }
            else if (c == 3 && cart.empty())
                cerr << "No item in cart. Buy something or cancel.\n";
            else if (c == 3)
            {
                cout << bocyan << u8"                                 ╔══════════╗                           \n" << reset;
                cout << bocyan << u8"═════════════════════════════════╣ CHECKOUT ╠═════════════════════════════════\n" << reset;
                cout << bocyan << u8"                                 ╚══════════╝                           \n" << reset;
                cout << "Date of this stock order\t: ";
                auto now = chrono::system_clock::now();
                auto time_t_now = chrono::system_clock::to_time_t(now);
                tm tm;
                errno_t err = localtime_s(&tm, &time_t_now);
                if (err != 0)
                    cerr << "Error getting local time" << endl;
                stringstream ss;
                ss << put_time(&tm, "%Y-%m-%d");
                sodate = ss.str();
                cout << sodate << endl;

                sosubtotal = 0;
                for (size_t i = 0; i < cart.size(); i++)
                {
                    res = stmt->executeQuery("select * from product where productId = \"" + cart.at(i).productId + "\"");
                    while (res->next())
                    {
                        if (res->getString("productId") == cart.at(i).productId)
                            cart.at(i).indiTotal = cart.at(i).quantity * res->getDouble("basePrice");
                    }
                    sosubtotal += cart.at(i).indiTotal;
                }

                pstmt = con->prepareStatement("insert into order_stock value (?, ?, ?, ?)");
                pstmt->setString(1, oid);
                pstmt->setString(2, sodate);
                pstmt->setString(3, Session::instance().staffId);
                pstmt->setString(4, supplierId);
                pstmt->executeUpdate();

                for (size_t i = 0; i < cart.size(); i++)
                    createOrderDetail(cart.at(i).productId, oid, cart.at(i).quantity, cart.at(i).indiTotal);

                cout << "Stock order successfully placed. Invoice will be sent when shipment arrived.\n";
                break;
            }
            else if (c == 0)
            {
                cerr << "Operatoin cancelled by user. Proceed to menu...\n";
                system("pause");
                break;
            }
            system("pause");
        } while (true);
    }
    catch (sql::SQLException& e)
    {
        cout << "Error registering stock order: " << e.what() << endl;
    }
    system("pause");
}

void StockOrderService::showOrderDetail(sql::ResultSet* res)
{
    sql::Connection* con = Database::instance().getConnection();
    res->beforeFirst();
    int c;
    cout << "\nEnter No to show it's stock order detail(0 to cancel): ";
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
    do { res->next(); counter++; } while (counter != c);

    string orderId = res->getString("orderId");
    string staffName = res->getString("staffName");
    string supplierName = res->getString("supplierName");
    string orderDate = res->getString("orderDate");

    sql::Statement* stmt = con->createStatement();
    string temp = "";
    bool displayonce = false, haventarrived = false, readytoendl = false;
    res = stmt->executeQuery("select *, od.status from order_detail od join inventory using (orderDetailId) join product using (productId) where orderId = \"" + orderId + "\"" + "order by orderDetailId asc");
    if (res->rowsCount() == 0)
    {
        res = stmt->executeQuery("select * from order_detail join product using (productId) where orderId = \"" + orderId + "\"" + "order by orderDetailId asc");
        haventarrived = true;
    }
    cout << endl;
    system("cls");
    cout << bocyan << u8"═════════════════════════════════════════════════════════════════════════════════════════════════════════════════════\n" << reset;
    cout << left << setw(17) << "Order ID : " << setw(30) << orderId << setw(15) << "Date : " << orderDate << "\n";
    cout << left << setw(17) << "Staff incharge :" << setw(30) << staffName << setw(15) << "Supplied by: " << supplierName << endl;
    cout << bocyan << u8"╒════════════════════════════════════════════════╤══════════════════╤════════════╤═════════╤══════════╤═══════════╤═════════╕\n" << reset;
    cout << bocyan << u8"│" << reset << left << setw(48) << "Product Name";
    cout << bocyan << u8"│" << reset << left << setw(18) << "Order Quantity";
    cout << bocyan << u8"│" << reset << left << setw(12) << "Item Total";
    cout << bocyan << u8"│" << reset << left << setw(9) << "Invoice";
    cout << bocyan << u8"│" << reset << left << setw(10) << "Received";
    cout << bocyan << u8"│" << reset << left << setw(11) << "Remaining";
    cout << bocyan << u8"│" << reset << left << setw(9) << "Status";
    cout << bocyan << u8"│" << reset << endl;
    cout << bocyan << u8"╞════════════════════════════════════════════════╪══════════════════╪════════════╪═════════╪══════════╪═══════════╪═════════╡\n" << reset;
    while (res->next())
    {
        readytoendl = false;
        if (res->getString("orderDetailId") != temp && !displayonce && !haventarrived)
        {
            temp = res->getString("orderDetailId");
            cout << bocyan << u8"│" << reset << left << setw(48) << res->getString("productName");
            cout << bocyan << u8"│" << reset << left << setw(18) << res->getInt("orderQuantity");
            cout << bocyan << u8"│" << reset << left << setw(12) << res->getDouble("itemTotal");
            displayonce = true;
        }
        if (!haventarrived)
        {
            cout << bocyan << u8"│" << reset << left << setw(9) << res->getString("invoiceId");
            cout << bocyan << u8"│" << reset << left << setw(10) << res->getInt("receiveQuantity");
            res->next();
            if (res->isAfterLast() || res->getString("orderDetailId") != temp)
            {
                res->previous();
                cout << bocyan << u8"│" << reset << left << setw(11) << res->getInt("remainingQuantity");
                if (res->getBoolean("status") == 1)
                    cout << bocyan << u8"│" << reset << left << setw(9) << "Completed";
                else
                    cout << bocyan << u8"│" << reset << left << setw(9) << "Pending";
                readytoendl = true;
                cout << bocyan << u8"│" << reset << endl;
            }
            else
            {
                res->previous();
                cout << bocyan << u8"│" << reset << left << setw(11) << " "
                    << bocyan << u8"│" << reset << left << setw(9) << " "
                    << bocyan << u8"│" << reset << endl;
            }
            if (displayonce || res->isAfterLast())
            {
                cout << bocyan << u8"│" << reset << left << setw(48) << ">>>>>> Unit Price: RM " + res->getString("basePrice");
                cout << bocyan << u8"│" << reset << left << setw(18) << " "
                    << bocyan << u8"│" << reset << left << setw(12) << " ";
                displayonce = false;
                res->next();
                if (!res->isAfterLast() && res->getString("orderDetailId") != temp)
                    cout << bocyan << u8"│" << reset << left << setw(9) << " "
                    << bocyan << u8"│" << reset << left << setw(10) << " "
                    << bocyan << u8"│" << reset << left << setw(11) << " "
                    << bocyan << u8"│" << reset << left << setw(9) << " "
                    << bocyan << u8"│" << reset << endl;
                else if (res->isAfterLast())
                    cout << bocyan << u8"│" << reset << left << setw(9) << " "
                    << bocyan << u8"│" << reset << left << setw(10) << " "
                    << bocyan << u8"│" << reset << left << setw(11) << " "
                    << bocyan << u8"│" << reset << left << setw(9) << " "
                    << bocyan << u8"│" << reset << endl;
                res->previous();
            }
            else if (!readytoendl)
                cout << bocyan << u8"│" << reset << left << setw(48) << " "
                << bocyan << u8"│" << reset << left << setw(18) << " "
                << bocyan << u8"│" << reset << left << setw(12) << " ";

            res->next();
            if (readytoendl && !res->isAfterLast())
                cout << bocyan << u8"│" << reset << left << setw(48) << " "
                << bocyan << u8"│" << reset << left << setw(18) << " "
                << bocyan << u8"│" << reset << left << setw(12) << " "
                << bocyan << u8"│" << reset << left << setw(9) << " "
                << bocyan << u8"│" << reset << left << setw(10) << " "
                << bocyan << u8"│" << reset << left << setw(11) << " "
                << bocyan << u8"│" << reset << left << setw(9) << " "
                << bocyan << u8"│" << reset << endl;
            res->previous();
        }
        else
        {
            cout << bocyan << u8"│" << reset << left << setw(48) << res->getString("productName");
            cout << bocyan << u8"│" << reset << left << setw(18) << res->getInt("orderQuantity");
            cout << bocyan << u8"│" << reset << left << setw(12) << res->getDouble("itemTotal");
            cout << bocyan << u8"│" << reset << left << setw(9) << " - ";
            cout << bocyan << u8"│" << reset << left << setw(10) << " - ";
            cout << bocyan << u8"│" << reset << left << setw(11) << res->getInt("remainingQuantity");
            cout << bocyan << u8"│" << reset << left << setw(9) << "Pending";
            cout << bocyan << u8"│" << reset << endl;
            cout << bocyan << u8"│" << reset << left << setw(48) << ">>>>>> Unit Price: RM " + res->getString("basePrice")
                << bocyan << u8"│" << reset << left << setw(18) << " "
                << bocyan << u8"│" << reset << left << setw(12) << " "
                << bocyan << u8"│" << reset << left << setw(9) << " "
                << bocyan << u8"│" << reset << left << setw(10) << " "
                << bocyan << u8"│" << reset << left << setw(11) << " "
                << bocyan << u8"│" << reset << left << setw(9) << " "
                << bocyan << u8"│" << reset << endl;
        }
    }
    cout << bocyan << u8"╘════════════════════════════════════════════════╧══════════════════╧════════════╧═════════╧══════════╧═══════════╧═════════╛\n" << reset;
    system("pause");
}

sql::ResultSet* StockOrderService::filterOrder(sql::ResultSet* res)
{
    sql::Connection* con = Database::instance().getConnection();
    system("cls");
    cout << bocyan << u8"                                 ╔════════════════════╗                           \n" << reset;
    cout << bocyan << u8"═════════════════════════════════╣ FILTER STOCK ORDER ╠═════════════════════════════════\n" << reset;
    cout << bocyan << u8"                                 ╚════════════════════╝                           \n" << reset;
    string start, end;
    regex datepattern("\\d{4}-\\d{2}-\\d{2}");
    do
    {
        cout << "Enter your date(YYYY--MM--DD) range from: ";
        cin >> start;
        while (!regex_match(start, datepattern))
        {
            cerr << "Invalid date format. Try again: ";
            cin >> start;
        }
        cout << "to: ";
        cin >> end;
        while (!regex_match(end, datepattern))
        {
            cerr << "Invalid date format. Try again: ";
            cin >> end;
        }
        if (start > end)
            cout << "Invalid range. Please Re-";
        else
            break;
    } while (true);

    try
    {
        sql::PreparedStatement* pstmt = con->prepareStatement("select * from order_stock where orderDate between ? and ?");
        pstmt->setString(1, start);
        pstmt->setString(2, end);
        res = pstmt->executeQuery();
        cout << endl << endl;
        int counter = 0;
        if (res->next())
        {
            cout << bocyan << u8"╒════╤══════════╤═══════════════╤═════════════════╤══════════╕\n" << reset;
            cout << bocyan << u8"│" << reset << left << setw(4) << "No.";
            cout << bocyan << u8"│" << reset << left << setw(10) << "Order ID";
            cout << bocyan << u8"│" << reset << left << setw(15) << "Order Date";
            cout << bocyan << u8"│" << reset << left << setw(17) << "Staff Incharge";
            cout << bocyan << u8"│" << reset << left << setw(10) << "Supplier";
            cout << bocyan << u8"│" << reset << endl;
            cout << bocyan << u8"╞════╪══════════╪═══════════════╪═════════════════╪══════════╡\n" << reset;
            do
            {
                cout << bocyan << u8"│" << reset << left << setw(4) << counter + 1;
                cout << bocyan << u8"│" << reset << left << setw(10) << res->getString("orderId");
                cout << bocyan << u8"│" << reset << left << setw(15) << res->getString("orderDate");
                cout << bocyan << u8"│" << reset << left << setw(17) << res->getString("staffId");
                cout << bocyan << u8"│" << reset << left << setw(10) << res->getString("supplierId");
                cout << bocyan << u8"│" << reset << endl;
                counter++;
            } while (res->next());
            cout << bocyan << u8"╘════╧══════════╧═══════════════╧═════════════════╧══════════╛\n" << reset;
        }
        else
        {
            cout << "No order found in that date range.\n";
        }
    }
    catch (sql::SQLException& e)
    {
        cout << "Error displaying Order Stock: " << e.what() << endl;
    }
    system("pause");
    return res;
}

sql::ResultSet* StockOrderService::viewOrders(int type)
{
    sql::Connection* con = Database::instance().getConnection();
    sql::ResultSet* res = nullptr;
    int c = -1, cpage = 1, pagesize = 10, totalp = 0;
    cout << endl;
    system("cls");
    cout << bocyan << u8"                               ╔══════════════════════╗                           \n" << reset;
    cout << bocyan << u8"═══════════════════════════════╣ VIEW ALL STOCK ORDER ╠═══════════════════════════════\n" << reset;
    cout << bocyan << u8"                               ╚══════════════════════╝                           \n" << reset;
    try
    {
        string combine = "select * from order_stock join staff using (staffId) join supplier using (supplierId) order by orderId asc";
        sql::PreparedStatement* pstmt = con->prepareStatement(combine);
        res = pstmt->executeQuery();
        int totalrow = res->rowsCount();
        if (totalrow == 0)
        {
            cerr << "No stock order found.\n";
            return nullptr;
        }
        totalp = (totalrow + pagesize - 1) / pagesize;
        while (true)
        {
            int offset = (cpage - 1) * pagesize;
            string pagequery = combine + " limit " + to_string(pagesize) + " offset " + to_string(offset);
            pstmt = con->prepareStatement(pagequery);
            res = pstmt->executeQuery();
            int counter = 0;
            cout << endl;
            system("cls");
            cout << bocyan << u8"══════════════════════════════════════════════════════════════════════════════════\n" << reset;
            cout << " (Page " << cpage << " of " << totalp << ")" << endl;
            cout << bocyan << u8"╒════╤══════════╤═══════════════╤══════════════════════════════╤════════════════════════════════════════╕\n" << reset;
            cout << bocyan << u8"│" << reset << left << setw(4) << "No.";
            cout << bocyan << u8"│" << reset << left << setw(10) << "Order ID";
            cout << bocyan << u8"│" << reset << left << setw(15) << "Order Date";
            cout << bocyan << u8"│" << reset << left << setw(30) << "Staff Name";
            cout << bocyan << u8"│" << reset << left << setw(40) << "Supplier Name";
            cout << bocyan << u8"│" << reset << endl;
            cout << bocyan << u8"╞════╪══════════╪═══════════════╪══════════════════════════════╪════════════════════════════════════════╡\n" << reset;
            while (res->next())
            {
                cout << bocyan << u8"│" << reset << left << setw(4) << counter + 1;
                cout << bocyan << u8"│" << reset << left << setw(10) << res->getString("orderId");
                cout << bocyan << u8"│" << reset << left << setw(15) << res->getString("orderDate");
                cout << bocyan << u8"│" << reset << left << setw(30) << res->getString("staffName");
                cout << bocyan << u8"│" << reset << left << setw(40) << res->getString("supplierName");
                cout << bocyan << u8"│" << reset << endl;
                counter++;
            }
            cout << bocyan << u8"╘════╧══════════╧═══════════════╧══════════════════════════════╧════════════════════════════════════════╛\n" << reset;
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
        if (type == 1)
        {
            cout << "1. Filer stock order by date\n2. Show stock order detail\n0. Cancel\n";
            cout << "Enter your choice: ";
            cin >> c;
            c = Utils::integerCheck(c);
            while (c < 0 || c > 2)
            {
                cerr << "Invalid choice. Try again: ";
                cin >> c;
                c = Utils::integerCheck(c);
            }
            if (c == 1)
                res = filterOrder(res);
            else if (c == 2)
                showOrderDetail(res);
        }
    }
    catch (sql::SQLException& e)
    {
        cout << "Error retrieving orderstock data: " << e.what() << endl;
    }
    return res;
}

void StockOrderService::searchOrder()
{
    sql::Connection* con = Database::instance().getConnection();
    string orderId;
    try
    {
        system("cls");
        cout << bocyan << u8"                               ╔════════════════════╗                           \n" << reset;
        cout << bocyan << u8"═══════════════════════════════╣ SEARCH STOCK ORDER ╠═══════════════════════════════\n" << reset;
        cout << bocyan << u8"                               ╚════════════════════╝                           \n" << reset;
        cout << "Enter Order ID (R____)(@ to cancel): ";
        getline(cin, orderId);

        sql::PreparedStatement* pstmt = con->prepareStatement(
            "select * from order_stock join staff using (staffId) join supplier using (supplierId) where orderId = ?");
        pstmt->setString(1, orderId);
        sql::ResultSet* res = pstmt->executeQuery();
        while (!res->next() && orderId != "@")
        {
            cerr << "Order not found. Please try again: ";
            cin >> orderId;
            pstmt->setString(1, orderId);
            res = pstmt->executeQuery();
        }
        if (orderId == "@")
            return;
        res->beforeFirst();
        while (res->next())
        {
            cout << "\nOrder ID\t: " << orderId << "\t\t\t\tStaff Incharge\t: " << res->getString("staffName") << endl;
            cout << "Order Date\t: " << res->getString("orderDate") << "\t\t\tSupplier\t: " << res->getString("supplierName");
        }

        sql::Statement* stmt = con->createStatement();
        res = stmt->executeQuery("select * from order_detail join product using (productId) where orderId = \"" + orderId + "\"");
        cout << "\nOrder detail are as below: ";
        cout << endl;
        cout << bocyan << u8"╒════════════════════╤═══════════════╤════════════════════════════════════════════════╤════════════════════╤════════════════════╤════════════╤═════════╕\n" << reset;
        cout << bocyan << u8"│" << reset << left << setw(20) << "Order Detail ID";
        cout << bocyan << u8"│" << reset << left << setw(15) << "Order ID";
        cout << bocyan << u8"│" << reset << left << setw(48) << "Product Name";
        cout << bocyan << u8"│" << reset << left << setw(20) << "Order Quantity";
        cout << bocyan << u8"│" << reset << left << setw(20) << "Remaining Quantity";
        cout << bocyan << u8"│" << reset << left << setw(12) << "Item Total";
        cout << bocyan << u8"│" << reset << left << setw(9) << "Status";
        cout << bocyan << u8"│" << reset << endl;
        cout << bocyan << u8"╞════════════════════╪═══════════════╪════════════════════════════════════════════════╪════════════════════╪════════════════════╪════════════╪═════════╡\n" << reset;
        while (res->next())
        {
            cout << bocyan << u8"│" << reset << left << setw(20) << res->getString("orderDetailId");
            cout << bocyan << u8"│" << reset << left << setw(15) << res->getString("orderId");
            cout << bocyan << u8"│" << reset << left << setw(48) << res->getString("productName");
            cout << bocyan << u8"│" << reset << left << setw(20) << res->getInt("orderQuantity");
            cout << bocyan << u8"│" << reset << left << setw(20) << res->getInt("remainingQuantity");
            cout << bocyan << u8"│" << reset << left << setw(12) << res->getDouble("itemTotal");
            if (res->getBoolean("status") == 0)
                cout << bocyan << u8"│" << reset << left << setw(9) << "Pending";
            else if (res->getBoolean("status") == 1)
                cout << bocyan << u8"│" << reset << left << setw(9) << "Completed";
            cout << bocyan << u8"│" << reset << endl;
        }
        cout << bocyan << u8"╘════════════════════╧═══════════════╧════════════════════════════════════════════════╧════════════════════╧════════════════════╧════════════╧═════════╛\n" << reset;
    }
    catch (sql::SQLException& e)
    {
        cerr << "Error retrieving orderstock data: " << e.what() << endl;
    }
    system("pause");
}

void StockOrderService::printRefundCart(const Cart& cart)
{
    sql::Connection* con = Database::instance().getConnection();
    try
    {
        cout << endl;
        cout << bocyan << u8"                                           ╔═══════════════════╗                           \n" << reset;
        cout << bocyan << u8"═══════════════════════════════════════════╣ REFUND STOCK CART ╠═════════════════════════════════════════════\n" << reset;
        cout << bocyan << u8"                                           ╚═══════════════════╝                           \n" << reset;
        cout << magenta << u8"╒════╤════════════════════════════════════════════════╤════════════════╤════════════════════╤═══════════════╕\n" << reset;
        cout << magenta << u8"│" << reset << left << setw(4) << "No.";
        cout << magenta << u8"│" << reset << left << setw(48) << "Product Name";
        cout << magenta << u8"│" << reset << left << setw(16) << "Order Quantity";
        cout << magenta << u8"│" << reset << left << setw(20) << "Remaining Quantity";
        cout << magenta << u8"│" << reset << left << setw(15) << "Refund Quantity";
        cout << magenta << u8"│" << reset << endl;
        cout << magenta << u8"╞════╪════════════════════════════════════════════════╪════════════════╪════════════════════╪═══════════════╡\n" << reset;
        if (!cart.empty())
        {
            for (size_t i = 0; i < cart.size(); i++)
            {
                sql::PreparedStatement* pstmt = con->prepareStatement(
                    "select p.productName, d.orderQuantity, d.remainingQuantity from product p join order_detail d using (productId) where productId = ?");
                pstmt->setString(1, cart.at(i).productId);
                sql::ResultSet* res = pstmt->executeQuery();
                res->next();
                cout << magenta << u8"│" << reset << left << setw(4) << (i + 1);
                cout << magenta << u8"│" << reset << left << setw(48) << res->getString("productName");
                cout << magenta << u8"│" << reset << left << setw(16) << res->getInt("orderQuantity");
                cout << magenta << u8"│" << reset << left << setw(20) << res->getInt("remainingQuantity");
                cout << magenta << u8"│" << reset << left << setw(15) << cart.at(i).quantity;
                cout << magenta << u8"│" << reset << endl;
            }
            cout << magenta << u8"╘════╧════════════════════════════════════════════════╧════════════════╧════════════════════╧═══════════════╛\n" << reset;
        }
        else
        {
            cout << magenta << u8"╘════╧════════════════════════════════════════════════╧════════════════╧════════════════════╧═══════════════╛\n" << reset;
            cout << "\n\tCart is empty.\n\n";
        }
    }
    catch (sql::SQLException& e)
    {
        cerr << e.what();
    }
}

void StockOrderService::refundOrder()
{
    sql::Connection* con = Database::instance().getConnection();
    cout << fixed << setprecision(2);
    int c;
    Cart refundCart;
    system("cls");
    cout << bocyan << u8"                                 ╔══════════════╗                           \n" << reset;
    cout << bocyan << u8"═════════════════════════════════╣ REFUND STOCK ╠═════════════════════════════════\n" << reset;
    cout << bocyan << u8"                                 ╚══════════════╝                           \n" << reset;

    sql::ResultSet* res1 = viewOrders(2);
    cout << bocyan << u8"══════════════════════════════════════════════════════════════════════════════════\n" << reset;
    cout << "Choose the stock order you want to refund(0 to cancel): ";
    cin >> c;
    c = Utils::integerCheck(c);
    res1->beforeFirst();
    while (c > res1->rowsCount() || c < 0)
    {
        cerr << "Invalid input. Try again: ";
        cin >> c;
        c = Utils::integerCheck(c);
    }
    if (c == 0)
        return;
    int counter = 0;
    do { res1->next(); counter++; } while (counter != c);
    c = 0;
    string orderId = res1->getString("orderId");

    sql::Statement* stmt = con->createStatement();
    res1 = stmt->executeQuery("select orderId, orderDate, staffId, supplierName from order_stock join staff using (staffId) join supplier using (supplierId) where orderId = \"" + orderId + "\"");
    res1->next();
    string display[3] = { res1->getString("supplierName"), res1->getString("orderDate"), res1->getString("staffId") };

    res1 = stmt->executeQuery("select orderDetailId from order_detail where orderQuantity <> remainingQuantity and orderId = \"" + orderId + "\"");
    if (res1->rowsCount() == 0)
    {
        cerr << "No shipment arrived yet. Try refunding another order...\n";
        system("pause");
        return;
    }

    do
    {
        system("cls");
        printRefundCart(refundCart);
        cout << "\n\nOrder ID\t\t: " << orderId;
        cout << "\t\t\tSupplied by\t: " << display[0] << "\n";
        cout << "Order Date\t\t: " << display[1];
        cout << "\t\tStaff Incharge\t: " << display[2];
        cout << endl;

        stmt = con->createStatement();
        sql::ResultSet* res = stmt->executeQuery(
            "select p.productId, p.productName, d.orderQuantity, d.remainingQuantity, d.itemTotal from order_detail d join product p using (productId) where orderQuantity <> remainingQuantity and orderId = \"" + orderId + "\"");
        int rowNo = 0;
        res->beforeFirst();
        cout << endl;
        cout << bocyan << u8"╒═════╤═════════════╤════════════════════════════════════════════════╤════════════════╤════════════════════╤══════════════╕\n" << reset;
        cout << bocyan << u8"│" << reset << left << setw(5) << "No. ";
        cout << bocyan << u8"│" << reset << left << setw(13) << "Product ID";
        cout << bocyan << u8"│" << reset << left << setw(48) << "Product Name";
        cout << bocyan << u8"│" << reset << left << setw(16) << "Order Quantity";
        cout << bocyan << u8"│" << reset << left << setw(20) << "Redundable Quantity";
        cout << bocyan << u8"│" << reset << right << setw(14) << "Total per item";
        cout << bocyan << u8"│" << reset << endl;
        cout << bocyan << u8"╞═════╪═════════════╪════════════════════════════════════════════════╪════════════════╪════════════════════╪══════════════╡\n" << reset;
        while (res->next())
        {
            cout << bocyan << u8"│" << reset << left << setw(5) << rowNo + 1;
            cout << bocyan << u8"│" << reset << left << setw(13) << res->getString("productId");
            cout << bocyan << u8"│" << reset << left << setw(48) << res->getString("productName");
            cout << bocyan << u8"│" << reset << left << setw(16) << res->getInt("orderQuantity");
            cout << bocyan << u8"│" << reset << left << setw(20) << res->getInt("orderQuantity") - res->getInt("remainingQuantity");
            cout << bocyan << u8"│" << reset << right << setw(14) << res->getDouble("itemTotal");
            cout << bocyan << u8"│" << reset << endl;
            rowNo++;
        }
        cout << bocyan << u8"╘═════╧═════════════╧════════════════════════════════════════════════╧════════════════╧════════════════════╧══════════════╛\n" << reset;
        cout << "\n1. Select item and quantity to refund\n2. Update Selection\n3. Checkout\n0. Cancel Refund\n\nEnter your choice: ";
        cin >> c;
        c = Utils::integerCheck(c);

        if (c == 1)
        {
            res->beforeFirst();
            int itemchoice = 0;
            cout << "Enter Product No.\t: ";
            cin >> itemchoice;
            itemchoice = Utils::integerCheck(itemchoice);
            while (itemchoice > res->rowsCount())
            {
                cerr << "Invalid input. Try again: ";
                cin >> itemchoice;
                itemchoice = Utils::integerCheck(itemchoice);
            }
            int cnt = 0;
            do { res->next(); cnt++; } while (cnt != itemchoice);

            int existing = refundCart.find(res->getString("productId"));
            if (existing >= 0)
            {
                cout << "Item already selected. Proceed to update quantity...\n";
                cout << "Update refund quantity: ";
                cin >> refundCart.at(existing).quantity;
                refundCart.at(existing).quantity = Utils::integerCheck(refundCart.at(existing).quantity);
                while (refundCart.at(existing).quantity > (res->getInt("orderQuantity") - res->getInt("remainingQuantity")))
                {
                    cerr << "Invalid refund quantity. Please try again: ";
                    cin >> refundCart.at(existing).quantity;
                    refundCart.at(existing).quantity = Utils::integerCheck(refundCart.at(existing).quantity);
                }
                cout << "Quantity updated.\n";
            }
            else
            {
                CartItem item;
                item.productId = res->getString("productId");
                cout << "Enter refund quantity\t: ";
                cin >> item.quantity;
                item.quantity = Utils::integerCheck(item.quantity);
                while (item.quantity > (res->getInt("orderQuantity") - res->getInt("remainingQuantity")))
                {
                    cerr << "Invalid refund quantity. Please try again: ";
                    cin >> item.quantity;
                    item.quantity = Utils::integerCheck(item.quantity);
                }
                cout << "Product Named: " << res->getString("productName") << " added to selection.\n";
                refundCart.add(item);
            }
        }
        else if (c == 2)
        {
            system("cls");
            printRefundCart(refundCart);
            cout << bocyan << u8"                                 ╔════════════════════╗                           \n" << reset;
            cout << bocyan << u8"═════════════════════════════════╣ UPDATE REFUND CART ╠═════════════════════════════════\n" << reset;
            cout << bocyan << u8"                                 ╚════════════════════╝                           \n" << reset;
            cout << "1. Remove Product\n2. Update Quantity\n0. Cancel Update\n";
            cout << bocyan << u8"════════════════════════════════════════════════════════════════════════════════════════\n" << reset;
            int choice;
            do
            {
                cout << "Enter your choice: ";
                cin >> choice;
                choice = Utils::integerCheck(choice);
                cin.ignore();
                if (choice == 1)
                {
                    int remove;
                    cout << "Enter the Product No. that you want to remove: ";
                    cin >> remove;
                    remove = Utils::integerCheck(remove);
                    while (remove < 1 || remove >(int)refundCart.size())
                    {
                        cerr << "Invalid product number. Try again: ";
                        cin >> remove;
                        remove = Utils::integerCheck(remove);
                    }
                    refundCart.removeAt(remove - 1);
                }
                else if (choice == 2)
                {
                    int productNo, nquantity;
                    cout << "Enter the Product No. that you want to update quantity: ";
                    cin >> productNo;
                    productNo = Utils::integerCheck(productNo);
                    cout << "Enter new quantity: ";
                    cin >> nquantity;
                    nquantity = Utils::integerCheck(nquantity);
                    res->beforeFirst();
                    while (res->next())
                    {
                        if (refundCart.at(productNo - 1).productId == res->getString("productId"))
                            break;
                    }
                    while (nquantity > (res->getInt("orderQuantity") - res->getInt("remainingQuantity")) || nquantity < 1)
                    {
                        cerr << "Invalid new quantity. Try again: ";
                        cin >> nquantity;
                        nquantity = Utils::integerCheck(nquantity);
                    }
                    refundCart.at(productNo - 1).quantity = nquantity;
                }
                else if (choice == 0)
                    break;
                else
                    cout << "Invalid choice. Please Re-";
            } while (choice < 0 || choice > 2);
            cout << "Cart updated.\n\n";
        }
        else if (c == 3)
        {
            system("cls");
            cout << bocyan << u8"                                 ╔══════════╗                           \n" << reset;
            cout << bocyan << u8"═════════════════════════════════╣ CHECKOUT ╠═════════════════════════════════\n" << reset;
            cout << bocyan << u8"                                 ╚══════════╝                           \n" << reset;
            try
            {
                sql::PreparedStatement* pstmt = con->prepareStatement(
                    "update order_detail set remainingQuantity = ? , status = 0 where orderId = ? and productId = ?");
                for (size_t i = 0; i < refundCart.size(); i++)
                {
                    pstmt->setInt(1, refundCart.at(i).quantity);
                    pstmt->setString(2, orderId);
                    pstmt->setString(3, refundCart.at(i).productId);
                    pstmt->executeUpdate();
                    productService.updateProduct(refundCart.at(i).productId, true, 1, refundCart.at(i).quantity);
                }
                cout << "Inventory updated.\n";
            }
            catch (sql::SQLException& e)
            {
                cout << "Error updating order_detail: " << e.what() << endl;
            }
            break;
        }
        else if (c == 0)
        {
            cout << "Refund cancelled.\n";
            break;
        }
        else
            cout << "Invalid choice. Please try again.\n\n";
        system("pause");
    } while (true);
    system("pause");
}