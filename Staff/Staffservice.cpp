#include "Staff/StaffService.h"
#include "Staff/Staff.h"
#include "Core/Database.h"
#include "Core/Utils.h"
#include "External/ansi.h"
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <iostream>
#include <iomanip>
#include <regex>
#include <cmath>

using namespace std;

void StaffService::registerStaff()
{
    sql::Connection* con = Database::instance().getConnection();
    string rid, rname, rpos, raddress, remail, rphone;
    try
    {
        system("cls");
        sql::Statement* stmt = con->createStatement();
        sql::ResultSet* res = stmt->executeQuery("select * from staff");
        int idcount = 0;
        while (res->next())
        {
            if (res->isLast())
            {
                string input = res->getString("staffId");
                string output = regex_replace(input, regex("[^0-9]*([0-9]+)"), string("$1"));
                idcount = stoi(output);
            }
        }
        if ((int)log10(idcount + 1) == 0)
            rid = "S000";
        else if ((int)log10(idcount + 1) == 1)
            rid = "S00";
        else if ((int)log10(idcount + 1) == 2)
            rid = "S0";
        else
            rid = "S";
        rid += to_string(idcount + 1);

        cout << bocyan << u8"                        ╔════════════════╗                           \n" << reset;
        cout << bocyan << u8"════════════════════════╣ REGISTER STAFF ╠════════════════════════\n" << reset;
        cout << bocyan << u8"                        ╚════════════════╝                           \n" << reset;
        cout << "Enter your name(@ to cancel)\t: ";
        getline(cin, rname);
        if (rname == "@")
            return;
        cout << "Enter your position\t\t: ";
        getline(cin, rpos);
        cout << "Enter your address\t\t: ";
        getline(cin, raddress);
        cout << "Enter your email\t\t: ";
        getline(cin, remail);
        cout << "Enter your phone number\t\t: ";
        getline(cin, rphone);
        cout << bocyan << u8"\n════════════════════════════════════════════════════════════════\n" << reset;

        sql::PreparedStatement* pstmt = con->prepareStatement(
            "insert into staff value (?, ?, ?, ?, ?, ?, ?, ?)");
        pstmt->setString(1, rid);
        pstmt->setString(2, rname);
        pstmt->setString(3, rpos);
        pstmt->setString(4, raddress);
        pstmt->setString(5, remail);
        pstmt->setString(6, rphone);

        string password;
        for (char c : rname)
        {
            if (!isspace(c))
            {
                password += c;
                if (password.length() == 6)
                    break;
            }
        }
        pstmt->setString(7, password);
        pstmt->setBoolean(8, 1);
        pstmt->executeUpdate();
        cout << "Staff successfully registered!\n";

        delete stmt;
        delete res;
        delete pstmt;
    }
    catch (sql::SQLException& e)
    {
        cout << "Error registering Staff: " << e.what() << endl;
    }
    system("pause");
}

bool StaffService::login()
{
    sql::Connection* con = Database::instance().getConnection();
    string staffid;
    system("cls");
    cout << bocyan << u8"                        ╔════════════╗                           \n" << reset;
    cout << bocyan << u8"════════════════════════╣ LOGIN PAGE ╠════════════════════════\n" << reset;
    cout << bocyan << u8"                        ╚════════════╝                           \n" << reset;
    cout << "Enter your Staff ID: ";
    getline(cin, staffid);
    cout << endl;
    sql::PreparedStatement* pstmt = con->prepareStatement("select * from staff where staffId = ?");
    pstmt->setString(1, staffid);
    sql::ResultSet* res = pstmt->executeQuery();
    while (!res->next())
    {
        cerr << "Staff not found. Please try again: ";
        cin >> staffid;
        pstmt->setString(1, staffid);
        res = pstmt->executeQuery();
        cout << endl;
    }
    cout << "Welcome " << res->getString("staffName") << "(" << res->getString("staffPos") << ")";
    cout << "\n\nEnter your password: ";

    string password = Utils::readMaskedInput();
    while (password != res->getString("staffPassword"))
    {
        cout << "Wrong password. Please try again: ";
        password = Utils::readMaskedInput();
        cout << endl;
    }

    cout << "\nLogin Successfully.\n";
    cout << bocyan << u8"\n══════════════════════════════════════════════════════════════\n" << reset;

    Session& session = Session::instance();
    session.staffId = staffid;
    session.isManager = (password == "MANAGE");

    delete pstmt;
    delete res;
    system("pause");
    return true;
}

void StaffService::displayPaginated(const string& baseQuery)
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
            cerr << "No staff found.\n";
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
            system("cls");
            cout << bocyan << u8"\n═════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════\n" << reset;
            cout << "  (Page " << cpage << " of " << totalp << ")" << endl;
            cout << bocyan << u8"╒═══╤═════╤═══════════════╤══════════════════╤════════════════════════════════════════════════╤════════════════════════════╤═══════════════╤════════╕\n" << reset;
            cout << bocyan << u8"│" << reset << left << setw(3) << "No.";
            cout << bocyan << u8"│" << reset << left << setw(5) << "ID";
            cout << bocyan << u8"│" << reset << left << setw(15) << "Name";
            cout << bocyan << u8"│" << reset << left << setw(18) << "Position";
            cout << bocyan << u8"│" << reset << left << setw(48) << "Address";
            cout << bocyan << u8"│" << reset << left << setw(28) << "Email";
            cout << bocyan << u8"│" << reset << left << setw(15) << "Phone";
            cout << bocyan << u8"│" << reset << left << setw(8) << "Status" << bocyan << u8"│" << reset;
            cout << bocyan << u8"\n╞═══╪═════╪═══════════════╪══════════════════╪════════════════════════════════════════════════╪════════════════════════════╪═══════════════╪════════╡\n" << reset;
            while (res->next())
            {
                cout << bocyan << u8"│" << reset << left << setw(3) << offset + counter + 1;
                cout << bocyan << u8"│" << reset << left << setw(5) << res->getString("staffId");
                cout << bocyan << u8"│" << reset << left << setw(15) << res->getString("staffName");
                cout << bocyan << u8"│" << reset << left << setw(18) << res->getString("staffPos");
                cout << bocyan << u8"│" << reset << left << setw(48) << res->getString("staffAddress");
                cout << bocyan << u8"│" << reset << left << setw(28) << res->getString("staffEmail");
                cout << bocyan << u8"│" << reset << left << setw(15) << res->getString("staffPhone");
                if (res->getBoolean("status") == 1)
                    cout << bocyan << u8"│" << reset << left << setw(8) << "Active";
                else
                    cout << bocyan << u8"│" << reset << left << setw(8) << "Inactive";
                cout << bocyan << u8"│" << reset << endl;
                counter++;
            }
            cout << bocyan << u8"╘═══╧═════╧═══════════════╧══════════════════╧════════════════════════════════════════════════╧════════════════════════════╧═══════════════╧════════╛\n" << reset;
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
        delete pstmt;
        delete res;
    }
    catch (sql::SQLException& e)
    {
        cout << "Error retrieving staff data: " << e.what() << endl;
    }
}

void StaffService::viewAll()
{
    cout << endl;
    cout << bocyan << u8"                                  ╔════════════════╗                           \n" << reset;
    cout << bocyan << u8"══════════════════════════════════╣ VIEW ALL STAFF ╠══════════════════════════════════\n" << reset;
    cout << bocyan << u8"                                  ╚════════════════╝                           \n" << reset;
    displayPaginated("select * from staff");
}

void StaffService::search()
{
    cout << endl;
    system("cls");
    cout << bocyan << u8"                                  ╔══════════════╗                           \n" << reset;
    cout << bocyan << u8"══════════════════════════════════╣ SEARCH STAFF ╠══════════════════════════════════\n" << reset;
    cout << bocyan << u8"                                  ╚══════════════╝                           \n" << reset;
    cout << "Enter Search Value(@@ to cancel): ";
    string searchvalue;
    getline(cin, searchvalue);
    if (searchvalue == "@@")
        return;
    string combine = "select * from staff where staffId like \"%" + searchvalue +
        "%\" or staffPos like \"%" + searchvalue + "%\" or staffName like \"%" + searchvalue +
        "%\" or staffAddress like \"%" + searchvalue + "%\" or staffEmail like \"%" + searchvalue + "%\"";
    displayPaginated(combine);
}

void StaffService::update()
{
    sql::Connection* con = Database::instance().getConnection();
    system("cls");
    int c;
    string staffId, nname, npos, naddress, nemail, nphone, combine;
    bool firstcolumn = true;

    viewAll();
    cout << endl;
    cout << bocyan << u8"                        ╔══════════════╗                           \n" << reset;
    cout << bocyan << u8"════════════════════════╣ UPDATE STAFF ╠════════════════════════\n" << reset;
    cout << bocyan << u8"                        ╚══════════════╝                           \n" << reset;
    cout << "Enter the Staff ID that you want to update(S____)(@ to cancel): ";
    cin >> staffId;

    sql::PreparedStatement* pstmt = con->prepareStatement("select * from staff where staffId = ?");
    pstmt->setString(1, staffId);
    sql::ResultSet* res = pstmt->executeQuery();
    while (!res->next() && staffId != "@")
    {
        cerr << "Staff not found. Please try again: ";
        cin >> staffId;
        pstmt->setString(1, staffId);
        res = pstmt->executeQuery();
    }
    if (staffId == "@")
        return;

    bool selected[5] = { false, false, false, false, false };
    string oldname = res->getString("staffName"), oldpos = res->getString("staffPos"),
        oldadd = res->getString("staffAddress"), oldemail = res->getString("staffEmail"),
        oldphone = res->getString("staffPhone");
    do
    {
        cout << endl;
        system("cls");
        cout << "Staff Name\t: " << oldname << "\nStaff Position\t: " << oldpos
            << "\nStaff Address\t: " << oldadd << "\nStaff Email\t: " << oldemail
            << "\nStaff Phone\t: " << oldphone << endl;
        cout << bocyan << u8"                        ╔═══════════════╗                           \n" << reset;
        cout << bocyan << u8"════════════════════════╣ UPDATE CHOICE ╠════════════════════════\n" << reset;
        cout << bocyan << u8"                        ╚═══════════════╝                           \n" << reset;
        cout << left << setw(20) << "1. Update Name" << (selected[0] ? u8"✔\t" + oldname + " >> " + nname : " ") << "\n";
        cout << left << setw(20) << "2. Update Positon" << (selected[1] ? u8"✔\t" + oldpos + " >> " + npos : " ") << "\n";
        cout << left << setw(20) << "3. Update Address" << (selected[2] ? u8"✔\t" + oldadd + " >> " + naddress : " ") << "\n";
        cout << left << setw(20) << "4. Update Email" << (selected[3] ? u8"✔\t" + oldemail + " >> " + nemail : " ") << "\n";
        cout << left << setw(20) << "5. Update Phone" << (selected[4] ? u8"✔\t" + oldphone + " >> " + nphone : " ") << "\n";
        cout << "6. Apply All Changes\n0. Cancel Update\n\n";
        cout << bocyan << u8"══════════════════════════════════════════════════════════════════\n" << reset;
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
            cout << "Enter new name: ";
            getline(cin, nname);
            if (!firstcolumn) combine += ", ";
            combine += "staffName = \"" + nname + "\"";
            firstcolumn = false;
            selected[0] = true;
        }
        else if (c == 2 && !selected[1])
        {
            cout << "Enter new position: ";
            getline(cin, npos);
            if (!firstcolumn) combine += ", ";
            combine += "staffPos = \"" + npos + "\"";
            firstcolumn = false;
            selected[1] = true;
        }
        else if (c == 3 && !selected[2])
        {
            cout << "Enter new address: ";
            getline(cin, naddress);
            if (!firstcolumn) combine += ", ";
            combine += "staffAddress = \"" + naddress + "\"";
            firstcolumn = false;
            selected[2] = true;
        }
        else if (c == 4 && !selected[3])
        {
            cout << "Enter new email: ";
            getline(cin, nemail);
            if (!firstcolumn) combine += ", ";
            combine += "staffEmail = \"" + nemail + "\"";
            firstcolumn = false;
            selected[3] = true;
        }
        else if (c == 5 && !selected[4])
        {
            cout << "Enter new phone: ";
            getline(cin, nphone);
            if (!firstcolumn) combine += ", ";
            combine += "staffPhone = \"" + nphone + "\"";
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
                string sql = "update staff set " + combine;
                pstmt = con->prepareStatement(sql + " where staffId = \"" + staffId + "\"");
                pstmt->executeUpdate();
                cout << "\nChanges applied!\n";

                pstmt = con->prepareStatement("select * from staff where staffId = ?");
                pstmt->setString(1, staffId);
                res = pstmt->executeQuery();
                while (res->next())
                {
                    cout << "\nUpdated Info: " << endl;
                    cout << "Staff ID\t: " << res->getString("staffId") << endl;
                    cout << "Staff Name\t: " << res->getString("staffName") << endl;
                    cout << "Staff Position\t: " << res->getString("staffPos") << endl;
                    cout << "Staff Address\t: " << res->getString("staffAddress") << endl;
                    cout << "Staff Email\t: " << res->getString("staffEmail") << endl;
                    cout << "Staff Phone\t: " << res->getString("staffPhone") << endl;
                    cout << endl;
                }
                delete pstmt;
            }
            catch (sql::SQLException& e)
            {
                cout << "Error updating staff data: " << e.what() << endl;
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
    } while (c != 6);
    system("pause");
}

void StaffService::deactivate()
{
    sql::Connection* con = Database::instance().getConnection();
    int c;
    bool staffnotfound = false;
    system("cls");
    string rmsid;

    viewAll();
    cout << bocyan << u8"                        ╔══════════════════╗                           \n" << reset;
    cout << bocyan << u8"════════════════════════╣ DEACTIVATE STAFF ╠════════════════════════\n" << reset;
    cout << bocyan << u8"                        ╚══════════════════╝                           \n" << reset;
    cout << "Enter the id of the Staff you want to deactivate(S____)(@ to cancel): ";
    cin.ignore();
    getline(cin, rmsid);

    sql::PreparedStatement* pstmt = con->prepareStatement("select * from staff where staffId = ?");
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
        cout << "\nStaff Info: " << endl;
        cout << "Staff ID\t: " << res->getString("staffId") << endl;
        cout << "Staff Name\t: " << res->getString("staffName") << endl;
        cout << "Staff Position\t: " << res->getString("staffPos") << endl;
        cout << "Staff Address\t: " << res->getString("staffAddress") << endl;
        cout << "Staff Email\t: " << res->getString("staffEmail") << endl;
        cout << "Staff Phone\t: " << res->getString("staffPhone") << endl;
        cout << endl;
    }
    cout << "Are you sure?\n1. Yes\n2. No\n";
    cin >> c;
    c = Utils::integerCheck(c);
    cin.ignore();
    do
    {
        if (c == 1)
        {
            if (staffnotfound)
            {
                cout << "Staff with id " + rmsid + " not found.\n";
                break;
            }
            try
            {
                pstmt = con->prepareStatement("delete from staff where staffId = ?");
                pstmt->setString(1, rmsid);
                pstmt->executeUpdate();
                delete pstmt;
            }
            catch (const std::exception&)
            {
                pstmt = con->prepareStatement("update staff set status = 0 where staffId = ?");
                pstmt->setString(1, rmsid);
                pstmt->executeUpdate();
                delete pstmt;
                cout << "Staff with id " << rmsid << " deactivated successfully!\n";
                break;
            }
            cout << "Staff with id " << rmsid << " deleted successfully!\n";
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