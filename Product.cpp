#include "Product.h"
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

using namespace std;

bool ProductService::displayPaginated(const string& baseQuery)
{
    sql::Connection* con = Database::instance().getConnection();
    int cpage = 1, pagesize = 10, totalp = 0;
    try
    {
        sql::PreparedStatement* pstmt = con->prepareStatement(baseQuery);
        sql::ResultSet* res = pstmt->executeQuery();
        int totalrow = res->rowsCount();
        if (totalrow == 0)
        {
            cerr << "No product found.\n";
            return false;
        }
        totalp = (totalrow + pagesize - 1) / pagesize;
        while (true)
        {
            int offset = (cpage - 1) * pagesize;
            string pagequery = baseQuery + " limit " + to_string(pagesize) + " offset " + to_string(offset);
            pstmt = con->prepareStatement(pagequery);
            res = pstmt->executeQuery();
            int counter = 0;
            cout << endl;
            system("cls");
            cout << bocyan << u8"══════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════\n" << reset;
            cout << " (Page " << cpage << " of " << totalp << ")" << endl;
            cout << bocyan << u8"╒════╤═════╤════════════════════════════════════════════════╤════════════════════╤══════════╤════════╤══════════╤════════╕\n" << reset;
            cout << bocyan << u8"│" << reset << left << setw(4) << "No.";
            cout << bocyan << u8"│" << reset << left << setw(5) << "ID";
            cout << bocyan << u8"│" << reset << left << setw(48) << "Product Name";
            cout << bocyan << u8"│" << reset << left << setw(20) << "Product Type";
            cout << bocyan << u8"│" << reset << left << setw(10) << "Base Price";
            cout << bocyan << u8"│" << reset << left << setw(8) << "Quantity";
            cout << bocyan << u8"│" << reset << left << setw(10) << "Sell Price";
            cout << bocyan << u8"│" << reset << left << setw(8) << "Status" << bocyan << u8"│" << reset << endl;
            cout << bocyan << u8"╞════╪═════╪════════════════════════════════════════════════╪════════════════════╪══════════╪════════╪══════════╪════════╡\n" << reset;
            while (res->next())
            {
                cout << bocyan << u8"│" << reset << left << setw(4) << offset + counter + 1;
                cout << bocyan << u8"│" << reset << left << setw(5) << res->getString("productId");
                cout << bocyan << u8"│" << reset << left << setw(48) << res->getString("productName");
                cout << bocyan << u8"│" << reset << left << setw(20) << res->getString("productType");
                cout << bocyan << u8"│" << reset << left << setw(10) << res->getDouble("basePrice");
                cout << bocyan << u8"│" << reset << left << setw(8) << res->getInt("quantity");
                cout << bocyan << u8"│" << reset << left << setw(10) << res->getDouble("sellingPrice");
                if (res->getBoolean("status") == 1)
                    cout << bocyan << u8"│" << reset << left << setw(8) << "Active";
                else
                    cout << bocyan << u8"│" << reset << left << setw(8) << "Inactive";
                cout << bocyan << u8"│" << reset << endl;
                counter++;
            }
            cout << bocyan << u8"╘════╧═════╧════════════════════════════════════════════════╧════════════════════╧══════════╧════════╧══════════╧════════╛\n" << reset;
            if (cpage > 1)
                cout << "1. Previous page\n";
            if (cpage < totalp)
                cout << "2. Next page\n";
            cout << "0. Stop viewing\nEnter choice: ";
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
        return true;
    }
    catch (sql::SQLException& e)
    {
        cout << "Error retrieving product data: " << e.what() << endl;
        return false;
    }
}

void ProductService::registerProduct()
{
    sql::Connection* con = Database::instance().getConnection();
    string rpid, rpname, rptype;
    double rpbaseprice, rpsellprice;
    int rpquantity;
    try
    {
        system("cls");
        sql::Statement* stmt = con->createStatement();
        sql::ResultSet* res = stmt->executeQuery("select * from product");
        int idcount = 0;
        while (res->next())
        {
            if (res->isLast())
            {
                string input = res->getString("productId");
                string output = regex_replace(input, regex("[^0-9]*([0-9]+)"), string("$1"));
                idcount = stoi(output);
            }
        }
        if ((int)log10(idcount + 1) == 0)
            rpid = "P000";
        else if ((int)log10(idcount + 1) == 1)
            rpid = "P00";
        else if ((int)log10(idcount + 1) == 2)
            rpid = "P0";
        else
            rpid = "P";
        rpid += to_string(idcount + 1);

        cout << bocyan << u8"                                 ╔══════════════════╗                           \n" << reset;
        cout << bocyan << u8"═════════════════════════════════╣ REGISTER PRODUCT ╠═════════════════════════════════\n" << reset;
        cout << bocyan << u8"                                 ╚══════════════════╝                           \n" << reset;
        cout << "Enter product name(@ to cancel)\t: ";
        getline(cin, rpname);
        if (rpname == "@")
            return;
        cout << "Enter product type\t\t: ";
        getline(cin, rptype);
        cout << "Enter product's base price\t: ";
        cin >> rpbaseprice;
        rpbaseprice = Utils::doubleCheck(rpbaseprice);
        cout << "Enter product quantity\t\t: ";
        cin >> rpquantity;
        rpquantity = Utils::integerCheck(rpquantity);
        cout << "Enter product's selling price\t: ";
        cin >> rpsellprice;
        rpsellprice = Utils::doubleCheck(rpsellprice);
        cout << bocyan << u8"══════════════════════════════════════════════════════════════════════════════════════\n" << reset;

        sql::PreparedStatement* pstmt = con->prepareStatement("insert into product value (?, ?, ?, ?, ?, ?, ?)");
        pstmt->setString(1, rpid);
        pstmt->setString(2, rpname);
        pstmt->setString(3, rptype);
        pstmt->setDouble(4, rpbaseprice);
        pstmt->setInt(5, rpquantity);
        pstmt->setDouble(6, rpsellprice);
        pstmt->setBoolean(7, 1);
        pstmt->executeUpdate();
        cout << "Product successfully registered!\n";

        delete pstmt;
        delete stmt;
        delete res;
    }
    catch (sql::SQLException& e)
    {
        cerr << "Error registering Product: " << e.what() << endl;
    }
    system("pause");
}

void ProductService::searchSortProduct()
{
    system("cls");
    cout << bocyan << u8"                                 ╔════════════════╗                           \n" << reset;
    cout << bocyan << u8"═════════════════════════════════╣ SEARCH PRODUCT ╠═════════════════════════════════\n" << reset;
    cout << bocyan << u8"                                 ╚════════════════╝                           \n" << reset;
    cout << "Enter Search Value(@@ to cancel): ";
    string searchvalue;
    getline(cin, searchvalue);
    if (searchvalue == "@@")
        return;

    string combine = "select * from product where (productId like \"%" + searchvalue +
        "%\" or productType like \"%" + searchvalue + "%\" or productName like \"%" + searchvalue +
        "%\") and status = 1";

    if (!displayPaginated(combine))
        return;

    cout << "Sort?\n1. Yes\n2. No\n";
    int c;
    do
    {
        cout << "Enter your choice : ";
        cin >> c;
        c = Utils::integerCheck(c);
        cin.ignore();
        if (c < 1 || c > 2)
            cout << "Invalid choice. Please Re-";
        else
            break;
    } while (true);

    if (c != 1)
        return;

    cout << "Sort type:\n1. By Quantity\n2. By Base Price\n3. By Selling Price\n0. Cancel Search\n";
    string sorttype;
    int c1, st;
    do
    {
        cout << "Enter your choice: ";
        cin >> c1;
        c1 = Utils::integerCheck(c1);
        cout << "\n1. Ascending Order\n2. Descending Order\nEnter Sorting Order: ";
        cin >> st;
        st = Utils::integerCheck(st);
        cin.ignore();
        if (c1 == 1)
        {
            if (st == 1) sorttype = "quantity asc";
            else if (st == 2) sorttype = "quantity desc";
            else cout << "Invalid order choice. Please Re-";
        }
        else if (c1 == 2)
        {
            if (st == 1) sorttype = "basePrice asc";
            else if (st == 2) sorttype = "basePrice desc";
            else cout << "Invalid order choice. Please Re-";
        }
        else if (c1 == 3)
        {
            if (st == 1) sorttype = "sellingPrice asc";
            else if (st == 2) sorttype = "sellingPrice desc";
            else cout << "Invalid order choice. Please Re-";
        }
    } while (sorttype == "");

    combine += " order by " + sorttype;
    displayPaginated(combine);
}

sql::ResultSet* ProductService::searchForSelection()
{
    sql::Connection* con = Database::instance().getConnection();
    int cpage = 1, pagesize = 10, totalp = 0;
    string combine, searchvalue;
    sql::PreparedStatement* pstmt;
    sql::ResultSet* res = nullptr;

    cout << "Enter Search Value: ";
    getline(cin, searchvalue);
    combine = "select * from product where (productId like \"%" + searchvalue +
        "%\" or productType like \"%" + searchvalue + "%\" or productName like \"%" + searchvalue + "%\") and status = 1";
    try
    {
        pstmt = con->prepareStatement(combine);
        res = pstmt->executeQuery();
        int totalrow = res->rowsCount();
        while (res->rowsCount() == 0)
        {
            cerr << "No product found. Try again: ";
            getline(cin, searchvalue);
            combine = "select * from product where productId like \"%" + searchvalue +
                "%\" or productType like \"%" + searchvalue + "%\" or productName like \"%" + searchvalue + "%\"";
            pstmt = con->prepareStatement(combine);
            res = pstmt->executeQuery();
            totalrow = res->rowsCount();
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
            cout << bocyan << u8"══════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════\n" << reset;
            cout << " (Page " << cpage << " of " << totalp << ")" << endl;
            cout << bocyan << u8"╒════╤═════╤════════════════════════════════════════════════╤════════════════════╤══════════╤════════╤══════════╤════════╕\n" << reset;
            cout << bocyan << u8"│" << reset << left << setw(4) << "No.";
            cout << bocyan << u8"│" << reset << left << setw(5) << "ID";
            cout << bocyan << u8"│" << reset << left << setw(48) << "Product Name";
            cout << bocyan << u8"│" << reset << left << setw(20) << "Product Type";
            cout << bocyan << u8"│" << reset << left << setw(10) << "Base Price";
            cout << bocyan << u8"│" << reset << left << setw(8) << "Quantity";
            cout << bocyan << u8"│" << reset << left << setw(10) << "Sell Price";
            cout << bocyan << u8"│" << reset << left << setw(8) << "Status" << bocyan << u8"│" << reset << endl;
            cout << bocyan << u8"╞════╪═════╪════════════════════════════════════════════════╪════════════════════╪══════════╪════════╪══════════╪════════╡\n" << reset;
            while (res->next())
            {
                cout << bocyan << u8"│" << reset << left << setw(4) << counter + 1;
                cout << bocyan << u8"│" << reset << left << setw(5) << res->getString("productId");
                cout << bocyan << u8"│" << reset << left << setw(48) << res->getString("productName");
                cout << bocyan << u8"│" << reset << left << setw(20) << res->getString("productType");
                cout << bocyan << u8"│" << reset << left << setw(10) << res->getDouble("basePrice");
                cout << bocyan << u8"│" << reset << left << setw(8) << res->getInt("quantity");
                cout << bocyan << u8"│" << reset << left << setw(10) << res->getDouble("sellingPrice");
                if (res->getBoolean("status") == 1)
                    cout << bocyan << u8"│" << reset << left << setw(8) << "Active";
                else
                    cout << bocyan << u8"│" << reset << left << setw(8) << "Inactive";
                cout << bocyan << u8"│" << reset << endl;
                counter++;
            }
            cout << bocyan << u8"╘════╧═════╧════════════════════════════════════════════════╧════════════════════╧══════════╧════════╧══════════╧════════╛\n" << reset;
            if (cpage > 1)
                cout << "1. Previous page\n";
            if (cpage < totalp)
                cout << "2. Next page\n";
            cout << "0. Stop searching\nEnter choice: ";
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
    }
    catch (sql::SQLException& e)
    {
        cout << "Error searching product: " << e.what() << endl;
    }
    // NOTE: preserved from the original — `res` here is whichever page was
    // displayed last (at most `pagesize` rows), not the full match set.
    // Callers (SaleService/StockOrderService) index into it by "Product No."
    // the same way the original createsales/createstockorder did. This is a
    // pre-existing quirk of the original code, not something introduced by
    // the refactor — flagging it here for whoever ports those two next.
    return res;
}

void ProductService::removeProduct()
{
    sql::Connection* con = Database::instance().getConnection();
    int c;
    system("cls");
    listAllForSelection();
    cin.ignore();
    string rmpid;
    cout << bocyan << u8"                                 ╔════════════════╗                           \n" << reset;
    cout << bocyan << u8"═════════════════════════════════╣ REMOVE PRODUCT ╠═════════════════════════════════\n" << reset;
    cout << bocyan << u8"                                 ╚════════════════╝                           \n" << reset;
    cout << "Enter the id of the product you want to remove(@ to cancel): ";
    getline(cin, rmpid);

    sql::PreparedStatement* pstmt = con->prepareStatement("select * from product where productId = ?");
    pstmt->setString(1, rmpid);
    sql::ResultSet* res = pstmt->executeQuery();
    while (!res->next() && rmpid != "@")
    {
        cerr << "Product not found. Please try again: ";
        getline(cin, rmpid);
        pstmt->setString(1, rmpid);
        res = pstmt->executeQuery();
    }
    if (rmpid == "@")
        return;

    res->beforeFirst();
    while (res->next())
    {
        cout << "\nProduct Info: " << endl;
        cout << "Product ID\t: " << res->getString("productId") << endl;
        cout << "Product Name\t: " << res->getString("productName") << endl;
        cout << "Product Type\t: " << res->getString("productType") << endl;
        cout << "Base Price\t: " << res->getString("basePrice") << endl;
        cout << "Quantity\t: " << res->getString("quantity") << endl;
        cout << "Sell Price\t: " << res->getString("sellingPrice") << endl;
        cout << endl;
    }
    cout << bocyan << u8"════════════════════════════════════════════════════════════════════════════════════\n" << reset;
    cout << "Are you sure?\n1. Yes\n2. No\n";
    cin >> c;
    c = Utils::integerCheck(c);
    cin.ignore();
    do
    {
        if (c == 1)
        {
            try
            {
                pstmt = con->prepareStatement("delete from product where productId = ?");
                pstmt->setString(1, rmpid);
                pstmt->executeUpdate();
                delete pstmt;
                cout << "Product with id " << rmpid << " removed successfully!\n";
            }
            catch (sql::SQLException& e)
            {
                pstmt = con->prepareStatement("update product set status = 0 where productId = ?");
                pstmt->setString(1, rmpid);
                pstmt->executeUpdate();
                delete pstmt;
                cout << "Product with id " << rmpid << " deactivated successfully!\n";
                break;
            }
            break;
        }
        else if (c == 2)
        {
            cout << "Remove Operation cancelled.\n\n";
            break;
        }
        else
        {
            cout << "Invalid choice. Please try again.";
        }
    } while (true);
    system("pause");
}

void ProductService::updateProduct(const string& productId, bool autoUpdate, int updateType, int quantity)
{
    sql::Connection* con = Database::instance().getConnection();
    int c;
    string npname, nptype;
    int npquantity = 0;
    double npbaseprice = 0, npsellingprice = 0;
    string combine;
    bool firstcolumn = true;

    if (!autoUpdate)
    {
        sql::PreparedStatement* pstmt = con->prepareStatement("select * from product where productId = ?");
        pstmt->setString(1, productId);
        sql::ResultSet* res = pstmt->executeQuery();
        res->next();
        bool selected[5] = { false, false, false, false, false };
        string oldname = res->getString("productName"), oldtype = res->getString("productType");
        double oldbprice = res->getDouble("basePrice"), oldsprice = res->getDouble("sellingPrice");
        int oldqtt = res->getInt("quantity");
        do
        {
            system("cls");
            cout << "Product Name\t: " << oldname << "\nProduct Type\t: " << oldtype
                << "\nBase Price\t: " << oldbprice << "\nQuantity\t: " << oldqtt
                << "\nSell Price\t: " << oldsprice << endl;
            cout << bocyan << u8"                                 ╔═══════════════╗                           \n" << reset;
            cout << bocyan << u8"═════════════════════════════════╣ UPDATE CHOICE ╠═════════════════════════════════\n" << reset;
            cout << bocyan << u8"                                 ╚═══════════════╝                           \n" << reset;
            cout << fixed << setprecision(2);
            cout << left << setw(24) << "1. Update Product Name" << (selected[0] ? u8"✔\t" + oldname + " >> " + npname : " ") << "\n";
            cout << left << setw(24) << "2. Update Product Type" << (selected[1] ? u8"✔\t" + oldtype + " >> " + nptype : " ") << "\n";
            cout << left << setw(24) << "3. Update Base Price" << (selected[2] ? u8"✔\t" + to_string(oldbprice) + " >> " + to_string(npbaseprice) : " ") << "\n";
            cout << left << setw(24) << "4. Update Quantity" << (selected[3] ? u8"✔\t" + to_string(oldqtt) + " >> " + to_string(npquantity) : " ") << "\n";
            cout << left << setw(24) << "5. Update Sell Price" << (selected[4] ? u8"✔\t" + to_string(oldsprice) + " >> " + to_string(npsellingprice) : " ") << "\n";
            cout << "6. Apply All Changes\n0. Cancel Update\n\n";
            cout << bocyan << u8"═══════════════════════════════════════════════════════════════════════════════════\n" << reset;
            cout << "Enter your choice: ";
            cin >> c;
            c = Utils::integerCheck(c);
            cin.ignore();
            while (c >= 1 && c <= 5 && selected[c - 1])
            {
                cerr << "Field is updated. Choose again: ";
                cin >> c;
                c = Utils::integerCheck(c);
                cin.ignore();
            }
            if (c == 1 && !selected[0])
            {
                cout << "Enter new Product Name: ";
                getline(cin, npname);
                if (!firstcolumn) combine += ", ";
                combine += "productName = \"" + npname + "\"";
                firstcolumn = false;
                selected[0] = true;
            }
            else if (c == 2 && !selected[1])
            {
                cout << "Enter new Product Type: ";
                getline(cin, nptype);
                if (!firstcolumn) combine += ", ";
                combine += "productType = \"" + nptype + "\"";
                firstcolumn = false;
                selected[1] = true;
            }
            else if (c == 3 && !selected[2])
            {
                cout << "Enter new Product's Base Price(RM): ";
                cin >> npbaseprice;
                if (!firstcolumn) combine += ", ";
                combine += "basePrice = " + to_string(npbaseprice);
                firstcolumn = false;
                selected[2] = true;
            }
            else if (c == 4 && !selected[3])
            {
                cout << "Enter new Product's Quantity: ";
                cin >> npquantity;
                if (!firstcolumn) combine += ", ";
                combine += "quantity = " + to_string(npquantity);
                firstcolumn = false;
                selected[3] = true;
            }
            else if (c == 5 && !selected[4])
            {
                cout << "Enter new Product's Selling Price: ";
                cin >> npsellingprice;
                if (!firstcolumn) combine += ", ";
                combine += "sellingPrice = " + to_string(npsellingprice);
                firstcolumn = false;
                selected[4] = true;
            }
            else if (c == 6)
            {
                if (firstcolumn)
                {
                    cout << "No changes applied. Returning to main menu...\n";
                    return;
                }
                try
                {
                    string sql = "update product set " + combine;
                    pstmt = con->prepareStatement(sql + " where productId = \"" + productId + "\"");
                    pstmt->executeUpdate();
                    cout << "\nChanges applied!\n";
                    pstmt = con->prepareStatement("select * from product where productId = ?");
                    pstmt->setString(1, productId);
                    res = pstmt->executeQuery();
                    while (res->next())
                    {
                        cout << "\nUpdated Info: " << endl;
                        cout << "Product ID\t: " << res->getString("productId") << endl;
                        cout << "Product Name\t: " << res->getString("productName") << endl;
                        cout << "Product Type\t: " << res->getString("productType") << endl;
                        cout << "Base Price\t: " << res->getString("basePrice") << endl;
                        cout << "Quantity\t: " << res->getString("quantity") << endl;
                        cout << "Sell Price\t: " << res->getString("sellingPrice") << endl;
                        cout << endl;
                    }
                    delete pstmt;
                }
                catch (sql::SQLException& e)
                {
                    cout << "Error updating Product data: " << e.what() << endl;
                }
                system("pause");
            }
            else if (c == 0)
            {
                cout << "Update cancelled by user. Proceed back to menu...\n";
                system("pause");
                break;
            }
            else
            {
                cout << "Invalid choice.\n";
            }
        } while (c != 6);
    }
    else
    {
        if (updateType == 1)
        {
            // insert sale autoupdate; also refund stockorder autoupdate
            sql::PreparedStatement* pstmt = con->prepareStatement("select quantity from product where productId = ?");
            pstmt->setString(1, productId);
            sql::ResultSet* res = pstmt->executeQuery();
            res->next();
            int newquantity = res->getInt("quantity") - quantity;
            try
            {
                pstmt = con->prepareStatement("update product set quantity = ? where productId = ?");
                pstmt->setInt(1, newquantity);
                pstmt->setString(2, productId);
                pstmt->executeUpdate();
                delete pstmt;
            }
            catch (sql::SQLException& e)
            {
                cout << "Error updating Product data: " << e.what() << endl;
            }
        }
        else if (updateType == 2)
        {
            // cancel sale autoupdate; also stock receive autoupdate
            sql::PreparedStatement* pstmt = con->prepareStatement("select quantity from product where productId = ?");
            pstmt->setString(1, productId);
            sql::ResultSet* res = pstmt->executeQuery();
            res->next();
            int newquantity = res->getInt("quantity") + quantity;
            try
            {
                pstmt = con->prepareStatement("update product set quantity = ? where productId = ?");
                pstmt->setInt(1, newquantity);
                pstmt->setString(2, productId);
                pstmt->executeUpdate();
                delete pstmt;
            }
            catch (sql::SQLException& e)
            {
                cout << "Error updating Product data: " << e.what() << endl;
            }
        }
        sql::Statement* stmt = con->createStatement();
        sql::ResultSet* res = stmt->executeQuery("select * from product");
        while (res->next())
        {
            if (res->getInt("quantity") == 0)
            {
                sql::PreparedStatement* pstmt = con->prepareStatement("update product set status = 0 where productId = ?");
                pstmt->setString(1, res->getString("productId"));
                pstmt->executeUpdate();
            }
        }
    }
}

void ProductService::viewAll()
{
    sql::Connection* con = Database::instance().getConnection();
    int c1 = -1, c2 = -1;
    system("cls");
    cout << bocyan << u8"                                 ╔══════════════╗                           \n" << reset;
    cout << bocyan << u8"═════════════════════════════════╣ VIEW PRODUCT ╠═════════════════════════════════\n" << reset;
    cout << bocyan << u8"                                 ╚══════════════╝                           \n" << reset;
    cout << "1. Filter by price range\n2. Sort products\n3. View All Product\n0. Cancel\n\n";
    cout << bocyan << u8"══════════════════════════════════════════════════════════════════════════════════\n" << reset;
    do
    {
        cout << "Enter your choice: ";
        cin >> c1;
        c1 = Utils::integerCheck(c1);
        cin.ignore();
        if (c1 == 0)
            return;
        if (c1 < 1 || c1 > 3)
            cout << "Invalid choice. Please Re-";
        else
            break;
    } while (true);

    if (c1 == 1)
    {
        cout << "Price range:\n1. Selling Price\n2. Base Price\nEnter your choice: ";
        cin >> c2;
        c2 = Utils::integerCheck(c2);
        while (c2 < 1 || c2 > 2)
        {
            cerr << "Invalid choice. Try again: ";
            cin >> c2;
            c2 = Utils::integerCheck(c2);
        }
        string rangetype = (c2 == 1) ? "sellingPrice" : "basePrice";
        double start, end;
        do
        {
            cout << "Enter your price range from: ";
            cin >> start;
            start = Utils::doubleCheck(start);
            cout << "to: ";
            cin >> end;
            end = Utils::doubleCheck(end);
            if (start > end)
                cout << "Invalid range. Please Re-";
            else
                break;
        } while (true);
        string combine = "select * from product where " + rangetype + " between " + to_string(start) + " and " + to_string(end);
        displayPaginated(combine);
    }
    else if (c1 == 2)
    {
        cout << "Sort type:\n1. By Quantity\n2. By Base Price\n3. By Selling Price\n4. By Product Type\n0. Cancel Sort\n";
        string sorttype;
        int c, st;
        do
        {
            cout << "Enter your choice: ";
            cin >> c;
            c = Utils::integerCheck(c);
            cout << "\n1. Ascending Order\n2. Descending Order\nEnter Sorting Order: ";
            cin >> st;
            st = Utils::integerCheck(st);
            cin.ignore();
            if (c == 1)
            {
                if (st == 1) sorttype = "quantity asc";
                else if (st == 2) sorttype = "quantity desc";
                else cout << "Invalid order choice. Please Re-";
            }
            else if (c == 2)
            {
                if (st == 1) sorttype = "basePrice asc";
                else if (st == 2) sorttype = "basePrice desc";
                else cout << "Invalid order choice. Please Re-";
            }
            else if (c == 3)
            {
                if (st == 1) sorttype = "sellingPrice asc";
                else if (st == 2) sorttype = "sellingPrice desc";
                else cout << "Invalid order choice. Please Re-";
            }
            else if (c == 4)
            {
                if (st == 1) sorttype = "productType asc";
                else if (st == 2) sorttype = "productType desc";
                else cout << "Invalid order choice. Please Re-";
            }
        } while (sorttype == "");
        string combine = "select * from product order by " + sorttype;
        displayPaginated(combine);
    }
    else if (c1 == 3)
    {
        displayPaginated("select * from product");
    }
}

void ProductService::listAllForSelection()
{
    displayPaginated("select * from product");
}