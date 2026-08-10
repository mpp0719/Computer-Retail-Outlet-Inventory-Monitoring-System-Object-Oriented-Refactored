#include "Sale.h"
#include "Database.h"
#include "Utils.h"
#include "Staff.h"
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

using namespace std;

void SaleService::printCart(const Cart& cart)
{
    sql::Connection* con = Database::instance().getConnection();
    system("cls");
    double subtotal = 0;
    cout << bocyan << u8"                                                      ╔════════════════╗                           \n" << reset;
    cout << bocyan << u8"══════════════════════════════════════════════════════╣      CART      ╠══════════════════════════════════════════════════════\n" << reset;
    cout << bocyan << u8"                                                      ╚════════════════╝                           \n" << reset;
    cout << magenta << u8"╒════╤═════════════╤════════════════════════════════════════════════╤════════════════════╤═════════╤════════════╤════════════╕\n" << reset;
    cout << magenta << u8"│" << reset << left << setw(4) << "No.";
    cout << magenta << u8"│" << reset << left << setw(13) << "Product ID";
    cout << magenta << u8"│" << reset << left << setw(48) << "Product Name";
    cout << magenta << u8"│" << reset << left << setw(20) << "Product Type";
    cout << magenta << u8"│" << reset << left << setw(9) << "Price";
    cout << magenta << u8"│" << reset << left << setw(12) << "Quantity";
    cout << magenta << u8"│" << reset << left << setw(12) << "Item total" << magenta << u8"│" << reset << endl;
    cout << magenta << u8"╞════╪═════════════╪════════════════════════════════════════════════╪════════════════════╪═════════╪════════════╪════════════╡\n" << reset;
    if (!cart.empty())
    {
        for (size_t i = 0; i < cart.size(); i++)
        {
            sql::PreparedStatement* pstmt = con->prepareStatement(
                "select productId, productName, productType, sellingPrice from product where productId = ?");
            pstmt->setString(1, cart.at(i).productId);
            sql::ResultSet* res = pstmt->executeQuery();
            res->next();
            cout << magenta << u8"│" << reset << left << setw(4) << (i + 1);
            cout << magenta << u8"│" << reset << left << setw(13) << res->getString("productId");
            cout << magenta << u8"│" << reset << left << setw(48) << res->getString("productName");
            cout << magenta << u8"│" << reset << left << setw(20) << res->getString("productType");
            cout << magenta << u8"│" << reset << left << setw(9) << res->getDouble("sellingPrice");
            cout << magenta << u8"│" << reset << left << setw(12) << cart.at(i).quantity;
            cout << magenta << u8"│" << reset << left << setw(12) << (res->getDouble("sellingPrice") * cart.at(i).quantity) << magenta << u8"│" << reset << endl;
            subtotal += res->getDouble("sellingPrice") * cart.at(i).quantity;
        }
        cout << magenta << u8"╘════╧═════════════╧════════════════════════════════════════════════╧════════════════════╧═════════╧════════════╧════════════╛\n" << reset;
        cout << right << setw(100) << "Subtotal: RM" << subtotal << endl;
    }
    else
    {
        cout << magenta << u8"╘════╧═════════════╧════════════════════════════════════════════════╧════════════════════╧═════════╧════════════╧════════════╛\n" << reset;
        cout << "\n\tCart is empty.\n\n";
    }
}

void SaleService::updateCartInteractively(Cart& cart)
{
    sql::Connection* con = Database::instance().getConnection();
    system("cls");
    printCart(cart);
    cout << bocyan << u8"                                 ╔═════════════╗                           \n" << reset;
    cout << bocyan << u8"═════════════════════════════════╣ UPDATE CART ╠═════════════════════════════════\n" << reset;
    cout << bocyan << u8"                                 ╚═════════════╝                           \n" << reset;
    cout << "1. Remove Product\n2. Update Quantity\n0. Cancel Update\n";
    cout << bocyan << u8"═════════════════════════════════════════════════════════════════════════════════\n" << reset;
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
            while (productNo < 1 || productNo >(int)cart.size())
            {
                cerr << "Invalid product number. Try again: ";
                cin >> productNo;
                productNo = Utils::integerCheck(productNo);
            }
            sql::PreparedStatement* pstmt = con->prepareStatement("select quantity from product where productId = ?");
            pstmt->setString(1, cart.at(productNo - 1).productId);
            sql::ResultSet* res = pstmt->executeQuery();
            res->next();
            cout << "Enter new quantity: ";
            cin >> nquantity;
            nquantity = Utils::integerCheck(nquantity);
            while (nquantity < 1 || nquantity > res->getInt("quantity"))
            {
                cerr << "Insufficient stock. Try again: ";
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

void SaleService::createSaleDetail(const string& productId, const string& saleId, int quantity, double total)
{
    sql::Connection* con = Database::instance().getConnection();
    try
    {
        sql::PreparedStatement* pstmt = con->prepareStatement("insert into sale_detail value (?, ?, ?, ?, ?, ?)");
        pstmt->setString(1, productId);
        pstmt->setString(2, saleId);
        pstmt->setInt(3, quantity);
        pstmt->setDouble(4, total);
        pstmt->setInt(5, 0);
        pstmt->setBoolean(6, 0);
        pstmt->executeUpdate();
        productService.updateProduct(productId, true, 1, quantity);
    }
    catch (sql::SQLException& e)
    {
        cout << "Error creating sales detail: " << e.what() << endl;
    }
}

void SaleService::createSale()
{
    sql::Connection* con = Database::instance().getConnection();
    int c = 0;
    string slid, slpayment, sldate;
    double slsubtotal = 0, pa = 0;
    Cart cart;
    srand((unsigned)time(0));
    try
    {
        int idcount = 0;
        sql::Statement* stmt = con->createStatement();
        sql::ResultSet* res = stmt->executeQuery("select * from sale");
        while (res->next())
        {
            if (res->isLast())
            {
                string input = res->getString("saleId");
                string output = regex_replace(input, regex("[^0-9]*([0-9]+)"), string("$1"));
                idcount = stoi(output);
            }
        }
        if ((int)log10(idcount + 1) == 0) slid = "Q000";
        else if ((int)log10(idcount + 1) == 1) slid = "Q00";
        else if ((int)log10(idcount + 1) == 2) slid = "Q0";
        else slid = "Q";
        slid += to_string(idcount + 1);

        do
        {
            system("cls");
            printCart(cart);
            cout << bocyan << u8"                                 ╔══════════════╗                           \n" << reset;
            cout << bocyan << u8"═════════════════════════════════╣ CREATE SALES ╠═════════════════════════════════\n" << reset;
            cout << bocyan << u8"                                 ╚══════════════╝                           \n" << reset;
            cout << "1. Add product to cart\n2. Update Cart\n3. Checkout\n0. Cancel\n";
            cout << "Enter your choice: ";
            cin >> c;
            c = Utils::integerCheck(c);
            cin.ignore();
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
                int counter = 0;
                do { pres->next(); counter++; } while (counter != itemchoice);

                int existing = cart.find(pres->getString("productId"));
                if (existing >= 0)
                {
                    cout << "Item already in cart. Proceed to update quantity...\n";
                    cout << "Update selling quantity: ";
                    cin >> cart.at(existing).quantity;
                    cart.at(existing).quantity = Utils::integerCheck(cart.at(existing).quantity);
                    while (cart.at(existing).quantity > pres->getInt("quantity"))
                    {
                        cerr << "Insufficient amount in stock. Please try again: ";
                        cin >> cart.at(existing).quantity;
                        cart.at(existing).quantity = Utils::integerCheck(cart.at(existing).quantity);
                    }
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
                    while (item.quantity > pres->getInt("quantity"))
                    {
                        cerr << "Insufficient amount in stock. Please try again: ";
                        cin >> item.quantity;
                        item.quantity = Utils::integerCheck(item.quantity);
                    }
                    cout << "Product with ID: " << item.productId << " added to cart.\n";
                    item.indiTotal = pres->getDouble("sellingPrice") * item.quantity;
                    cart.add(item);
                }
            }
            else if (c == 2)
            {
                updateCartInteractively(cart);
            }
            else if (c == 3 && cart.empty())
                cerr << "Nothing in cart. Buy something or cancel.\n";
            else if (c == 3)
            {
                cout << bocyan << u8"                                 ╔══════════╗                           \n" << reset;
                cout << bocyan << u8"═════════════════════════════════╣ CHECHOUT ╠═════════════════════════════════\n" << reset;
                cout << bocyan << u8"                                 ╚══════════╝                           \n" << reset;
                cout << "Date of this sale\t: ";
                auto now = chrono::system_clock::now();
                auto time_t_now = chrono::system_clock::to_time_t(now);
                tm tm;
                errno_t err = localtime_s(&tm, &time_t_now);
                if (err != 0)
                    cerr << "Error getting local time" << endl;
                stringstream ss;
                ss << put_time(&tm, "%Y-%m-%d");
                sldate = ss.str();
                cout << sldate;

                slsubtotal = 0;
                for (size_t i = 0; i < cart.size(); i++)
                {
                    res = stmt->executeQuery("select * from product where productId = \"" + cart.at(i).productId + "\"");
                    while (res->next())
                    {
                        if (res->getString("productId") == cart.at(i).productId)
                            cart.at(i).indiTotal = cart.at(i).quantity * res->getDouble("sellingPrice");
                    }
                    slsubtotal += cart.at(i).indiTotal;
                }

                int paymentchoice = -1;
                cout << "\nPayment method:\n1. Cash\n2. Card\n3. Digital\nEnter choice: ";
                cin >> paymentchoice;
                paymentchoice = Utils::integerCheck(paymentchoice);
                while (paymentchoice < 1 || paymentchoice > 3)
                {
                    cerr << "Invalid choice. Try again: ";
                    cin >> paymentchoice;
                    paymentchoice = Utils::integerCheck(paymentchoice);
                }
                int cardend = -1, qrid = -1;
                if (paymentchoice == 1)
                {
                    slpayment = "Cash";
                    cout << "Subtotal: RM" << slsubtotal << "\n";
                    cout << "Enter Payment amount: RM";
                    cin >> pa;
                    pa = Utils::doubleCheck(pa);
                    while (pa < slsubtotal)
                    {
                        cerr << "Insufficient amount. Try again: ";
                        cin >> pa;
                        pa = Utils::doubleCheck(pa);
                    }
                    cout << "Balance: RM" << pa - slsubtotal << endl;
                }
                else if (paymentchoice == 2)
                {
                    slpayment = "Card";
                    cardend = (rand() % 9000) + 1000;
                    cout << "Payment of RM" << slsubtotal << " is paid using card with ending " << cardend << ".\n";
                }
                else if (paymentchoice == 3)
                {
                    slpayment = "Digital";
                    qrid = (rand() % 9000000) + 1000000;
                    cout << "Payment of RM" << slsubtotal << " is paid using QR with ID " << qrid << ".\n";
                }
                system("pause");

                sql::PreparedStatement* pstmt = con->prepareStatement("insert into sale value (?, ?, ?, ?, ?, ?)");
                pstmt->setString(1, slid);
                pstmt->setString(2, slpayment);
                pstmt->setString(3, sldate);
                pstmt->setDouble(4, slsubtotal);
                pstmt->setString(5, Session::instance().staffId);
                pstmt->setDouble(6, 0);
                pstmt->executeUpdate();

                for (size_t i = 0; i < cart.size(); i++)
                    createSaleDetail(cart.at(i).productId, slid, cart.at(i).quantity, cart.at(i).indiTotal);

                cout << "Sale successfully registered!\n";
                if (slpayment == "Cash")
                    reportService.createReceiptFile(slid, pa, "", 1);
                else if (slpayment == "Card")
                    reportService.createReceiptFile(slid, 0, to_string(cardend), 2);
                else if (slpayment == "Digital")
                    reportService.createReceiptFile(slid, 0, to_string(qrid), 3);
                reportService.showReceiptFile(slid);
                break;
            }
            else if (c == 0)
            {
                cerr << "Operation cancelled by user. Proceed to menu...\n";
                system("pause");
                break;
            }
            else
                cerr << "Invalid imput. Try again: ";
            system("pause");
        } while (true);
    }
    catch (sql::SQLException& e)
    {
        cout << "Error registering Sale: " << e.what() << endl;
    }
}

void SaleService::showSaleDetail(sql::ResultSet* res)
{
    sql::Connection* con = Database::instance().getConnection();
    res->beforeFirst();
    int c;
    cout << fixed << setprecision(2);
    cout << "\nEnter No to show it's sale detail(0 to cancel): ";
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

    string staffId = res->getString("staffId");
    string saleId = res->getString("saleId");
    double subtotal = res->getDouble("subtotal");
    double refundTotal = res->getDouble("refundTotal");

    sql::Statement* stmt = con->createStatement();
    sql::ResultSet* detailRes = stmt->executeQuery(
        "select *, sale_detail.quantity as \"saleQuantity\" from sale_detail join product using (productId) where saleId = \"" + saleId + "\"");
    cout << "\n\nSale ID\t: " << saleId;
    cout << "\t\tStaff Incharge\t: " << staffId;
    cout << endl;
    cout << bocyan << u8"╒═════════════╤════════════════════════════════════════════════╤══════════════════╤════════════════════╤═══════════════╤═════════════════╕\n" << reset;
    cout << bocyan << u8"│" << reset << left << setw(13) << "Product ID";
    cout << bocyan << u8"│" << reset << left << setw(48) << "Product Name";
    cout << bocyan << u8"│" << reset << left << setw(18) << "Quantity Sold";
    cout << bocyan << u8"│" << reset << right << setw(20) << "Total per item(RM)";
    cout << bocyan << u8"│" << reset << right << setw(15) << "Refund Status";
    cout << bocyan << u8"│" << reset << right << setw(17) << "Refund Quantity";
    cout << bocyan << u8"│" << reset << endl;
    cout << bocyan << u8"╞═════════════╪════════════════════════════════════════════════╪══════════════════╪════════════════════╪═══════════════╪═════════════════╡\n" << reset;
    while (detailRes->next())
    {
        cout << bocyan << u8"│" << reset << left << setw(13) << detailRes->getString("productId");
        cout << bocyan << u8"│" << reset << left << setw(48) << detailRes->getString("productName");
        cout << bocyan << u8"│" << reset << left << setw(18) << detailRes->getInt("saleQuantity");
        cout << bocyan << u8"│" << reset << right << setw(20) << detailRes->getDouble("total");
        cout << bocyan << u8"│" << reset << right << setw(15) << detailRes->getBoolean("refundStatus");
        cout << bocyan << u8"│" << reset << right << setw(17) << detailRes->getInt("refundQtt");
        cout << bocyan << u8"│" << reset << endl;
    }
    cout << bocyan << u8"╘═════════════╧════════════════════════════════════════════════╧══════════════════╧════════════════════╧═══════════════╧═════════════════╛\n" << reset;
    cout << right << setw(83) << "Subtotal|Refunded: RM " << right << setw(20) << subtotal << right << setw(32) << refundTotal << "\n";
    system("pause");
}

sql::ResultSet* SaleService::filterSale(sql::ResultSet* res)
{
    sql::Connection* con = Database::instance().getConnection();
    int cpage = 1, pagesize = 10, totalp = 0;
    cout << fixed << setprecision(2);
    cout << bocyan << u8"                                 ╔═════════════╗                           \n" << reset;
    cout << bocyan << u8"═════════════════════════════════╣ FILTER SALE ╠═════════════════════════════════\n" << reset;
    cout << bocyan << u8"                                 ╚═════════════╝                           \n" << reset;
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

    string combine = "select * from sale where saleDate between \"" + start + "\" and \"" + end + "\"";
    try
    {
        sql::PreparedStatement* pstmt = con->prepareStatement(combine);
        res = pstmt->executeQuery();
        int totalrow = res->rowsCount();
        if (totalrow == 0)
        {
            cerr << "No product found.\n";
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
            cout << bocyan << u8"═══════════════════════════════════════════════════════════════════════════════════════════\n" << reset;
            cout << " (Page " << cpage << " of " << totalp << ")" << endl;
            cout << bocyan << u8"╒════╤══════════╤════════════════════╤═══════════════╤══════════╤══════════════╤══════════════╕\n" << reset;
            cout << bocyan << u8"│" << reset << left << setw(4) << "No.";
            cout << bocyan << u8"│" << reset << left << setw(10) << "Sale ID";
            cout << bocyan << u8"│" << reset << left << setw(20) << "Payment Method";
            cout << bocyan << u8"│" << reset << left << setw(15) << "Sale Date";
            cout << bocyan << u8"│" << reset << left << setw(10) << "Subtotal";
            cout << bocyan << u8"│" << reset << left << setw(14) << "Refund Total";
            cout << bocyan << u8"│" << reset << left << setw(14) << "Staff Incharge";
            cout << bocyan << u8"│" << reset << endl;
            cout << bocyan << u8"╞════╪══════════╪════════════════════╪═══════════════╪══════════╪══════════════╪══════════════╡\n" << reset;
            while (res->next())
            {
                cout << bocyan << u8"│" << reset << left << setw(4) << counter + 1;
                cout << bocyan << u8"│" << reset << left << setw(10) << res->getString("saleId");
                cout << bocyan << u8"│" << reset << left << setw(20) << res->getString("paymentMethod");
                cout << bocyan << u8"│" << reset << left << setw(15) << res->getString("saleDate");
                cout << bocyan << u8"│" << reset << left << setw(10) << res->getDouble("subtotal");
                cout << bocyan << u8"│" << reset << left << setw(14) << res->getDouble("refundTotal");
                cout << bocyan << u8"│" << reset << left << setw(14) << res->getString("staffId");
                cout << bocyan << u8"│" << reset << endl;
                counter++;
            }
            cout << bocyan << u8"╘════╧══════════╧════════════════════╧═══════════════╧══════════╧══════════════╧══════════════╛\n" << reset;
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
        cout << "\n\nSale Analysis within date range " << start << " to " << end << ": \n\nTotal number of sales\t: " << totalrow << " sale(s)\n\n";
        sql::Statement* stmt = con->createStatement();
        res = stmt->executeQuery("select sum(subtotal - refundTotal) as revenueDateRange from sale where saleDate between \"" + start + "\" and \"" + end + "\"");
        res->next();
        cout << "Total Amount of revenue\t: RM " << res->getDouble("revenueDateRange") << "\n\n";
    }
    catch (sql::SQLException& e)
    {
        cout << "Error displaying sale: " << e.what() << endl;
    }
    system("pause");
    return res;
}

sql::ResultSet* SaleService::viewSales(int type)
{
    sql::Connection* con = Database::instance().getConnection();
    int c = -1, cpage = 1, pagesize = 10, totalp = 0;
    cout << fixed << setprecision(2);
    cout << endl;
    sql::ResultSet* res = nullptr;
    try
    {
        string combine = "select * from sale";
        sql::PreparedStatement* pstmt = con->prepareStatement(combine);
        res = pstmt->executeQuery();
        int totalrow = res->rowsCount();
        if (totalrow == 0)
        {
            cerr << "No sale found.\n";
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
            cout << bocyan << u8"                                   ╔═══════════════╗                           \n" << reset;
            cout << bocyan << u8"═══════════════════════════════════╣ VIEW ALL SALE ╠═══════════════════════════════════\n" << reset;
            cout << bocyan << u8"                                   ╚═══════════════╝                           \n" << reset;
            cout << bocyan << u8"═══════════════════════════════════════════════════════════════════════════════════════\n" << reset;
            cout << " (Page " << cpage << " of " << totalp << ")" << endl;
            cout << bocyan << u8"╒════╤══════════╤════════════════════╤═══════════════╤══════════╤══════════════╤══════════════╕\n" << reset;
            cout << bocyan << u8"│" << reset << left << setw(4) << "No.";
            cout << bocyan << u8"│" << reset << left << setw(10) << "Sale ID";
            cout << bocyan << u8"│" << reset << left << setw(20) << "Payment Method";
            cout << bocyan << u8"│" << reset << left << setw(15) << "Sale Date";
            cout << bocyan << u8"│" << reset << left << setw(10) << "Subtotal";
            cout << bocyan << u8"│" << reset << left << setw(14) << "Refund Total";
            cout << bocyan << u8"│" << reset << left << setw(14) << "Staff Incharge";
            cout << bocyan << u8"│" << reset << endl;
            cout << bocyan << u8"╞════╪══════════╪════════════════════╪═══════════════╪══════════╪══════════════╪══════════════╡\n" << reset;
            while (res->next())
            {
                cout << bocyan << u8"│" << reset << left << setw(4) << counter + 1;
                cout << bocyan << u8"│" << reset << left << setw(10) << res->getString("saleId");
                cout << bocyan << u8"│" << reset << left << setw(20) << res->getString("paymentMethod");
                cout << bocyan << u8"│" << reset << left << setw(15) << res->getString("saleDate");
                cout << bocyan << u8"│" << reset << left << setw(10) << res->getDouble("subtotal");
                cout << bocyan << u8"│" << reset << left << setw(14) << res->getString("refundTotal");
                cout << bocyan << u8"│" << reset << left << setw(14) << res->getString("staffId");
                cout << bocyan << u8"│" << reset << endl;
                counter++;
            }
            cout << bocyan << u8"╘════╧══════════╧════════════════════╧═══════════════╧══════════╧══════════════╧══════════════╛\n" << reset;
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
            cout << "1. Filter Sale by date\n2. Show sale detail\n0. Cancel\n";
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
                res = filterSale(res);
            else if (c == 2)
                showSaleDetail(res);
        }
    }
    catch (sql::SQLException& e)
    {
        cout << "Error retrieving staff data: " << e.what() << endl;
    }
    return res;
}

void SaleService::applyRefundToDetail(const string& productId, const string& saleId, int quantity, int /*max*/)
{
    sql::Connection* con = Database::instance().getConnection();
    try
    {
        productService.updateProduct(productId, true, 2, quantity);
        sql::PreparedStatement* pstmt = con->prepareStatement(
            "update sale_detail set refundQtt = ?, refundStatus = ? where saleId = ? and productId = ?");
        pstmt->setInt(1, quantity);
        pstmt->setBoolean(2, true);
        pstmt->setString(3, saleId);
        pstmt->setString(4, productId);
        pstmt->executeUpdate();
    }
    catch (sql::SQLException& e)
    {
        cout << "Error deleting sales detail: " << e.what() << endl;
    }
}

void SaleService::printRefundCart(const Cart& cart)
{
    sql::Connection* con = Database::instance().getConnection();
    try
    {
        cout << endl;
        cout << bocyan << u8"                                                     ╔══════════════════╗                           \n" << reset;
        cout << bocyan << u8"═════════════════════════════════════════════════════╣ REFUND SALE CART ╠═════════════════════════════════════════════════════\n" << reset;
        cout << bocyan << u8"                                                     ╚══════════════════╝                           \n" << reset;
        cout << magenta << u8"╒════╤════════════════════════════════════════════════╤═════════╤══════════════════╤════════════╤═════════════╕\n" << reset;
        cout << magenta << u8"│" << reset << left << setw(4) << "No.";
        cout << magenta << u8"│" << reset << left << setw(48) << "Product Name";
        cout << magenta << u8"│" << reset << left << setw(9) << "Price";
        cout << magenta << u8"│" << reset << left << setw(18) << "Sold Quantity";
        cout << magenta << u8"│" << reset << left << setw(12) << "To Refund";
        cout << magenta << u8"│" << reset << right << setw(13) << "Refund Amount" << magenta << u8"│" << reset << endl;
        cout << magenta << u8"╞════╪════════════════════════════════════════════════╪═════════╪══════════════════╪════════════╪═════════════╡\n" << reset;
        if (!cart.empty())
        {
            for (size_t i = 0; i < cart.size(); i++)
            {
                sql::PreparedStatement* pstmt = con->prepareStatement(
                    "select p.productName, p.sellingPrice, d.quantity, d.total from product p join sale_detail d using (productId) where productId = ?");
                pstmt->setString(1, cart.at(i).productId);
                sql::ResultSet* res = pstmt->executeQuery();
                res->next();
                cout << magenta << u8"│" << reset << left << setw(4) << (i + 1);
                cout << magenta << u8"│" << reset << left << setw(48) << res->getString("productName");
                cout << magenta << u8"│" << reset << left << setw(9) << res->getDouble("sellingPrice");
                cout << magenta << u8"│" << reset << left << setw(18) << res->getInt("quantity");
                cout << magenta << u8"│" << reset << left << setw(12) << cart.at(i).quantity;
                cout << magenta << u8"│" << reset << right << setw(13) << res->getDouble("sellingPrice") * cart.at(i).quantity;
                cout << magenta << u8"│" << reset << endl;
            }
            cout << magenta << u8"╘════╧════════════════════════════════════════════════╧═════════╧══════════════════╧════════════╧═════════════╛\n" << reset;
        }
        else
        {
            cout << magenta << u8"╘════╧════════════════════════════════════════════════╧═════════╧══════════════════╧════════════╧═════════════╛\n" << reset;
            cout << "\n\tCart is empty.\n\n";
        }
    }
    catch (sql::SQLException& e)
    {
        cerr << e.what();
    }
}

void SaleService::refundSale()
{
    sql::Connection* con = Database::instance().getConnection();
    cout << fixed << setprecision(2);
    int c;
    double refundTotal = 0;
    Cart refundCart;
    system("cls");
    cout << bocyan << u8"                                 ╔════════════════════════╗                           \n" << reset;
    cout << bocyan << u8"═════════════════════════════════╣ REFUND SALES (PRODUCT) ╠═════════════════════════════════\n" << reset;
    cout << bocyan << u8"                                 ╚════════════════════════╝                           \n" << reset;

    sql::ResultSet* res = viewSales(2);
    cout << "Choose the sale you want to refund(0 to cancel): ";
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
    c = 0;
    string rmsid = res->getString("saleId");
    string display[3] = { res->getString("paymentMethod"), res->getString("saleDate"), res->getString("staffId") };

    do
    {
        system("cls");
        printRefundCart(refundCart);
        cout << "\nSale ID\t\t: " << rmsid;
        cout << "\t\t\tPayment Method\t: " << display[0] << "\n";
        cout << "Sale Date\t: " << display[1];
        cout << "\t\tStaff Incharge\t: " << display[2];
        cout << endl;

        sql::Statement* stmt = con->createStatement();
        sql::ResultSet* detailRes = stmt->executeQuery(
            "select p.productId, p.productName, p.sellingPrice, d.saleId, d.quantity, d.total from sale_detail d join product p using (productId) where saleId = \"" + rmsid + "\"");
        int rowNo = 0;
        detailRes->beforeFirst();
        cout << endl;
        cout << bocyan << u8"╒═════╤════════════════════════════════════════════════╤══════════╤══════════════════╤══════════════╕\n" << reset;
        cout << bocyan << u8"│" << reset << left << setw(5) << "No. ";
        cout << bocyan << u8"│" << reset << left << setw(48) << "Product Name";
        cout << bocyan << u8"│" << reset << left << setw(10) << "Sale ID";
        cout << bocyan << u8"│" << reset << left << setw(18) << "Quantity Sold";
        cout << bocyan << u8"│" << reset << left << setw(14) << "Total per item";
        cout << bocyan << u8"│" << reset << endl;
        cout << bocyan << u8"╞═════╪════════════════════════════════════════════════╪══════════╪══════════════════╪══════════════╡\n" << reset;
        while (detailRes->next())
        {
            cout << bocyan << u8"│" << reset << left << setw(5) << rowNo + 1;
            cout << bocyan << u8"│" << reset << left << setw(48) << detailRes->getString("productName");
            cout << bocyan << u8"│" << reset << left << setw(10) << detailRes->getString("saleId");
            cout << bocyan << u8"│" << reset << left << setw(18) << detailRes->getInt("quantity");
            cout << bocyan << u8"│" << reset << right << setw(14) << detailRes->getDouble("total");
            cout << bocyan << u8"│" << reset << endl;
            rowNo++;
        }
        cout << bocyan << u8"╘═════╧════════════════════════════════════════════════╧══════════╧══════════════════╧══════════════╛\n" << reset;
        cout << "\n1. Select item and quantity to refund\n2. Update Selection\n3. Checkout\n0. Cancel Refund\n\nEnter your choice: ";
        cin >> c;
        c = Utils::integerCheck(c);

        if (c == 1)
        {
            detailRes->beforeFirst();
            int itemchoice = 0;
            cout << "Enter Product No.\t: ";
            cin >> itemchoice;
            itemchoice = Utils::integerCheck(itemchoice);
            while (itemchoice > detailRes->rowsCount())
            {
                cerr << "Invalid input. Try again: ";
                cin >> itemchoice;
                itemchoice = Utils::integerCheck(itemchoice);
            }
            int cnt = 0;
            do { detailRes->next(); cnt++; } while (cnt != itemchoice);

            int existing = refundCart.find(detailRes->getString("productId"));
            if (existing >= 0)
            {
                cout << "Item already selected. Proceed to update quantity...\n";
                cout << "Update refund quantity: ";
                cin >> refundCart.at(existing).quantity;
                refundCart.at(existing).quantity = Utils::integerCheck(refundCart.at(existing).quantity);
                while (refundCart.at(existing).quantity > detailRes->getInt("quantity"))
                {
                    cerr << "Invalid refund quantity. Please try again: ";
                    cin >> refundCart.at(existing).quantity;
                    refundCart.at(existing).quantity = Utils::integerCheck(refundCart.at(existing).quantity);
                }
                refundCart.at(existing).indiTotal = detailRes->getDouble("sellingPrice") * refundCart.at(existing).quantity;
                cout << "Quantity updated.\n";
            }
            else
            {
                CartItem item;
                item.productId = detailRes->getString("productId");
                cout << "Enter refund quantity\t: ";
                cin >> item.quantity;
                item.quantity = Utils::integerCheck(item.quantity);
                while (item.quantity > detailRes->getInt("quantity"))
                {
                    cerr << "Invalid refund quantity. Please try again: ";
                    cin >> item.quantity;
                    item.quantity = Utils::integerCheck(item.quantity);
                }
                item.indiTotal = detailRes->getDouble("sellingPrice") * item.quantity;
                cout << "Product Named: " << detailRes->getString("productName") << " added to selection.\n";
                refundCart.add(item);
            }
        }
        else if (c == 2)
        {
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
                    detailRes->beforeFirst();
                    while (detailRes->next())
                    {
                        if (refundCart.at(productNo - 1).productId == detailRes->getString("productId"))
                            break;
                    }
                    while (nquantity > detailRes->getInt("quantity") || nquantity < 1)
                    {
                        cerr << "Invalid new quantity. Try again: ";
                        cin >> nquantity;
                        nquantity = Utils::integerCheck(nquantity);
                    }
                    refundCart.at(productNo - 1).quantity = nquantity;
                    refundCart.at(productNo - 1).indiTotal = nquantity * detailRes->getDouble("sellingPrice");
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
            try
            {
                for (size_t i = 0; i < refundCart.size(); i++)
                {
                    sql::PreparedStatement* pstmt = con->prepareStatement("select * from sale_detail where saleId = ? and productId = ?");
                    pstmt->setString(1, rmsid);
                    pstmt->setString(2, refundCart.at(i).productId);
                    sql::ResultSet* r = pstmt->executeQuery();
                    r->next();
                    int max = r->getInt("quantity");
                    applyRefundToDetail(refundCart.at(i).productId, rmsid, refundCart.at(i).quantity, max);
                    refundTotal += refundCart.at(i).indiTotal;
                }
                sql::PreparedStatement* pstmt = con->prepareStatement("select * from sale_detail where saleId = ?");
                pstmt->setString(1, rmsid);
                sql::ResultSet* r = pstmt->executeQuery();
                if (!r->next())
                {
                    pstmt = con->prepareStatement("delete from sale where saleId = ?");
                    pstmt->setString(1, rmsid);
                    pstmt->executeUpdate();
                    cout << "Sale with id " << rmsid << " removed successfully!\n";
                }
                else
                {
                    pstmt = con->prepareStatement("update sale set refundTotal = ? where saleId = ?");
                    pstmt->setDouble(1, refundTotal);
                    pstmt->setString(2, rmsid);
                    pstmt->executeUpdate();
                    cout << "Sale with id " << rmsid << " updated successfully!\n";
                }
            }
            catch (sql::SQLException& e)
            {
                cout << "Error removing sale with id " << rmsid << ": " << e.what() << endl;
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