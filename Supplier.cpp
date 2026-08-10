#include "Supplier.h"
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

void SupplierService::registerSupplier()
{
    sql::Connection* con = Database::instance().getConnection();
    string sid, sname, semail, sphone, saddress;
    try
    {
        system("cls");
        sql::Statement* stmt = con->createStatement();
        sql::ResultSet* res = stmt->executeQuery("select * from supplier");
        int idcount = 0;
        while (res->next())
        {
            if (res->isLast())
            {
                string input = res->getString("supplierId");
                string output = regex_replace(input, regex("[^0-9]*([0-9]+)"), string("$1"));
                idcount = stoi(output);
            }
        }
        if ((int)log10(idcount + 1) == 0)
            sid = "B000";
        else if ((int)log10(idcount + 1) == 1)
            sid = "B00";
        else if ((int)log10(idcount + 1) == 2)
            sid = "B0";
        else
            sid = "B";
        sid += to_string(idcount + 1);

        cout << bocyan << u8"                                 ╔═══════════════════╗                           \n" << reset;
        cout << bocyan << u8"═════════════════════════════════╣ REGISTER SUPPLIER ╠═════════════════════════════════\n" << reset;
        cout << bocyan << u8"                                 ╚═══════════════════╝                           \n" << reset;
        cout << "Enter your name(@@ to cancel)\t: ";
        getline(cin, sname);
        if (sname == "@@")
            return;
        cout << "Enter your phone number\t\t: ";
        getline(cin, sphone);
        cout << "Enter your address\t\t: ";
        getline(cin, saddress);
        cout << "Enter your email address\t: ";
        getline(cin, semail);
        cout << bocyan << u8"════════════════════════════════════════════════════════════════════════════════════════\n" << reset;

        sql::PreparedStatement* pstmt = con->prepareStatement("insert into supplier value (?, ?, ?, ?, ?, ?)");
        pstmt->setString(1, sid);
        pstmt->setString(2, sname);
        pstmt->setString(3, sphone);
        pstmt->setString(4, saddress);
        pstmt->setString(5, semail);
        pstmt->setBoolean(6, 1);
        pstmt->executeUpdate();
        cout << "Supplier successfully registered!\n";

        delete pstmt;
        delete stmt;
        delete res;
    }
    catch (sql::SQLException& e)
    {
        cout << "Error registering Staff: " << e.what() << endl;
    }
    system("pause");
}

void SupplierService::displayPaginated(const string& baseQuery)
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
            cerr << "No supplier found.\n";
            return;
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
            cout << bocyan << u8"═════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════\n" << reset;
            cout << " (Page " << cpage << " of " << totalp << ")" << endl;
            cout << bocyan << u8"╒════╤═════╤══════════════════════════════╤═══════════════╤══════════════════════════════════════════════════╤════════════════════════════╤════════╕\n" << reset;
            cout << bocyan << u8"│" << reset << left << setw(4) << "No";
            cout << bocyan << u8"│" << reset << left << setw(5) << "ID";
            cout << bocyan << u8"│" << reset << left << setw(30) << "Supplier Name";
            cout << bocyan << u8"│" << reset << left << setw(15) << "Phone";
            cout << bocyan << u8"│" << reset << left << setw(50) << "Supplier Address";
            cout << bocyan << u8"│" << reset << left << setw(28) << "Supplier Email";
            cout << bocyan << u8"│" << reset << left << setw(8) << "Status" << bocyan << u8"│" << reset << endl;
            cout << bocyan << u8"╞════╪═════╪══════════════════════════════╪═══════════════╪══════════════════════════════════════════════════╪════════════════════════════╪════════╡\n" << reset;
            while (res->next())
            {
                cout << bocyan << u8"│" << reset << left << setw(4) << offset + counter + 1;
                cout << bocyan << u8"│" << reset << left << setw(5) << res->getString("supplierId");
                cout << bocyan << u8"│" << reset << left << setw(30) << res->getString("supplierName");
                cout << bocyan << u8"│" << reset << left << setw(15) << res->getString("supplierPhone");
                cout << bocyan << u8"│" << reset << left << setw(50) << res->getString("supplierAddress");
                cout << bocyan << u8"│" << reset << left << setw(28) << res->getString("supplierEmail");
                if (res->getBoolean("status") == 1)
                    cout << bocyan << u8"│" << reset << left << setw(8) << "Active";
                else
                    cout << bocyan << u8"│" << reset << left << setw(8) << "Inactive";
                cout << bocyan << u8"│" << reset << endl;
                counter++;
            }
            cout << bocyan << u8"╘════╧═════╧══════════════════════════════╧═══════════════╧══════════════════════════════════════════════════╧════════════════════════════╧════════╛\n" << reset;
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
        delete pstmt;
        delete res;
    }
    catch (sql::SQLException& e)
    {
        cout << "Error retrieving supplier data: " << e.what() << endl;
    }
}

void SupplierService::viewAll()
{
    system("cls");
    cout << bocyan << u8"                                 ╔═══════════════════╗                           \n" << reset;
    cout << bocyan << u8"═════════════════════════════════╣ VIEW ALL SUPPLIER ╠═════════════════════════════════\n" << reset;
    cout << bocyan << u8"                                 ╚═══════════════════╝                           \n" << reset;
    displayPaginated("select * from supplier");
}

void SupplierService::search()
{
    cout << bocyan << u8"                                 ╔═════════════════╗                           \n" << reset;
    cout << bocyan << u8"═════════════════════════════════╣ SEARCH SUPPLIER ╠═════════════════════════════════\n" << reset;
    cout << bocyan << u8"                                 ╚═════════════════╝                           \n" << reset;
    cout << "Enter Search Value(@@ to cancel): ";
    string searchvalue;
    getline(cin, searchvalue);
    if (searchvalue == "@@")
        return;
    string combine = "select * from supplier where supplierId like \"%" + searchvalue +
        "%\" or supplierName like \"%" + searchvalue + "%\" or supplierAddress like \"%" + searchvalue +
        "%\" or supplierEmail like \"%" + searchvalue + "%\"";
    displayPaginated(combine);
}

void SupplierService::update()
{
    sql::Connection* con = Database::instance().getConnection();
    int c;
    string supplierId, nname, naddress, nemail, nphone, combine;
    bool firstcolumn = true;

    cout << bocyan << u8"                                 ╔═════════════════╗                           \n" << reset;
    cout << bocyan << u8"═════════════════════════════════╣ UPDATE SUPPLIER ╠═════════════════════════════════\n" << reset;
    cout << bocyan << u8"                                 ╚═════════════════╝                           \n" << reset;
    viewAll();
    cout << "Enter the supplier ID that you want to update(B____)(@ to cancel): ";
    cin.ignore();
    cin >> supplierId;

    sql::PreparedStatement* pstmt = con->prepareStatement("select * from supplier where supplierId = ?");
    pstmt->setString(1, supplierId);
    sql::ResultSet* res = pstmt->executeQuery();
    while (!res->next() && supplierId != "@")
    {
        cerr << "Supplier not found. Please try again: ";
        cin >> supplierId;
        pstmt->setString(1, supplierId);
        res = pstmt->executeQuery();
    }
    if (supplierId == "@")
        return;

    bool selected[4] = { false, false, false, false };
    string oldname = res->getString("supplierName"), oldadd = res->getString("supplierAddress"),
        oldemail = res->getString("supplierEmail"), oldphone = res->getString("supplierPhone");
    do
    {
        system("cls");
        cout << "\nSupplier Name\t: " << oldname << "\nSupplier Address: " << oldadd
            << "\nSupplier Email\t: " << oldemail << "\nSupplier Phone\t: " << oldphone << endl;
        cout << bocyan << u8"                                 ╔═══════════════╗                           \n" << reset;
        cout << bocyan << u8"═════════════════════════════════╣ UPDATE CHOICE ╠═════════════════════════════════\n" << reset;
        cout << bocyan << u8"                                 ╚═══════════════╝                           \n" << reset;
        cout << left << setw(20) << "1. Update Name" << (selected[0] ? u8"✔\t" + oldname + " >> " + nname : " ") << "\n";
        cout << left << setw(20) << "2. Update Address" << (selected[1] ? u8"✔\t" + oldadd + " >> " + naddress : " ") << "\n";
        cout << left << setw(20) << "3. Update Email" << (selected[2] ? u8"✔\t" + oldemail + " >> " + nemail : " ") << "\n";
        cout << left << setw(20) << "4. Update Phone" << (selected[3] ? u8"✔\t" + oldphone + " >> " + nphone : " ") << "\n";
        cout << "5. Apply All Changes\n0. Cancel Update\n\n";
        cout << bocyan << u8"═══════════════════════════════════════════════════════════════════════════════════\n" << reset;
        cout << "Enter your choice: ";
        cin >> c;
        c = Utils::integerCheck(c);
        cin.ignore();
        while (c >= 1 && c <= 4 && selected[c - 1])
        {
            cerr << "Field is updated. Choose again: ";
            cin >> c;
            c = Utils::integerCheck(c);
            cin.ignore();
        }
        if (c == 1 && !selected[0])
        {
            cout << "Enter new name: ";
            getline(cin, nname);
            if (!firstcolumn) combine += ", ";
            combine += "supplierName = \"" + nname + "\"";
            firstcolumn = false;
            selected[0] = true;
        }
        else if (c == 2 && !selected[1])
        {
            cout << "Enter new address: ";
            getline(cin, naddress);
            if (!firstcolumn) combine += ", ";
            combine += "supplierAddress = \"" + naddress + "\"";
            firstcolumn = false;
            selected[1] = true;
        }
        else if (c == 3 && !selected[2])
        {
            cout << "Enter new email: ";
            getline(cin, nemail);
            if (!firstcolumn) combine += ", ";
            combine += "supplierEmail = \"" + nemail + "\"";
            firstcolumn = false;
            selected[2] = true;
        }
        else if (c == 4 && !selected[3])
        {
            cout << "Enter new phone: ";
            getline(cin, nphone);
            if (!firstcolumn) combine += ", ";
            combine += "supplierPhone = \"" + nphone + "\"";
            firstcolumn = false;
            selected[3] = true;
        }
        else if (c == 5)
        {
            if (firstcolumn)
            {
                cout << "No changes applied. Returning to main menu...\n";
                return;
            }
            try
            {
                string sql = "update supplier set " + combine;
                pstmt = con->prepareStatement(sql + " where supplierId = \"" + supplierId + "\"");
                pstmt->executeUpdate();
                cout << "\nChanges applied!\n";

                pstmt = con->prepareStatement("select * from supplier where supplierId = ?");
                pstmt->setString(1, supplierId);
                res = pstmt->executeQuery();
                while (res->next())
                {
                    cout << "\nUpdated Info: " << endl;
                    cout << "Supplier ID\t\t: " << res->getString("supplierId") << endl;
                    cout << "Supplier Name\t\t: " << res->getString("supplierName") << endl;
                    cout << "Supplier Phone\t\t: " << res->getString("supplierPhone") << endl;
                    cout << "Supplier Address\t: " << res->getString("supplierAddress") << endl;
                    cout << "Supplier Email\t\t: " << res->getString("supplierEmail") << endl;
                    cout << endl;
                }
                delete pstmt;
            }
            catch (sql::SQLException& e)
            {
                cout << "Error updating supplier data: " << e.what() << endl;
            }
        }
        else if (c == 0)
        {
            break;
        }
        else
        {
            cout << "Invalid choice.\n";
        }
    } while (c != 5);
    system("pause");
}

void SupplierService::deactivate()
{
    sql::Connection* con = Database::instance().getConnection();
    int c;
    bool suppliernotfound = false;
    system("cls");
    string rmsid;

    viewAll();
    cout << bocyan << u8"                                 ╔═════════════════════╗                           \n" << reset;
    cout << bocyan << u8"═════════════════════════════════╣ DEACTIVATE SUPPLIER ╠═════════════════════════════════\n" << reset;
    cout << bocyan << u8"                                 ╚═════════════════════╝                           \n" << reset;
    cin.ignore();
    cout << "Enter the id of the Supplier you want to deactivate(B____)(@ to cancel): ";
    getline(cin, rmsid);

    sql::PreparedStatement* pstmt = con->prepareStatement("select * from supplier where supplierId = ?");
    pstmt->setString(1, rmsid);
    sql::ResultSet* res = pstmt->executeQuery();
    while (!res->next() && rmsid != "@")
    {
        cerr << "Staff not found. Please try again: ";
        cin >> rmsid;
        pstmt->setString(1, rmsid);
        res = pstmt->executeQuery();
    }
    if (rmsid == "@")
        return;

    res->beforeFirst();
    while (res->next())
    {
        cout << "\nSupplier Info: " << endl;
        cout << "Supplier ID\t\t: " << res->getString("supplierId") << endl;
        cout << "Supplier Name\t\t: " << res->getString("supplierName") << endl;
        cout << "Supplier Phone\t\t: " << res->getString("supplierPhone") << endl;
        cout << "Supplier Address\t: " << res->getString("supplierAddress") << endl;
        cout << "Supplier Email\t\t: " << res->getString("supplierEmail") << endl;
        cout << endl;
    }
    cout << "Are you sure?\n1. Yes\n2. No\n";
    cin >> c;
    c = Utils::integerCheck(c);
    do
    {
        if (c == 1)
        {
            if (suppliernotfound)
            {
                cout << "Supplier with id " + rmsid + " not found.\n";
                break;
            }
            try
            {
                pstmt = con->prepareStatement("delete from supplier where supplierId = ?");
                pstmt->setString(1, rmsid);
                pstmt->executeUpdate();
                delete pstmt;
            }
            catch (const std::exception&)
            {
                pstmt = con->prepareStatement("update supplier set status = 0 where supplierId = ?");
                pstmt->setString(1, rmsid);
                pstmt->executeUpdate();
                delete pstmt;
                cout << "Supplier with id " << rmsid << " deactivated successfully!\n";
                break;
            }
            cout << "Supplier with id " << rmsid << " deleted successfully!\n";
            break;
        }
        else if (c == 2)
        {
            cout << "Deactivation cancelled.\n\n";
            break;
        }
        else
        {
            cout << "Invalid choice. Please try again.";
        }
    } while (true);
    system("pause");
}