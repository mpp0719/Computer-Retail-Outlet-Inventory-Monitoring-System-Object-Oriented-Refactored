#include "ReportService.h"
#include "Database.h"
#include "Utils.h"
#include "ansi.h"
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <regex>
#include <vector>

using namespace std;

void ReportService::createReceiptFile(const string& saleId, double pay, const string& cardnum, int type)
{
    sql::Connection* con = Database::instance().getConnection();
    try
    {
        ofstream rfile("Receipts\\receipt_" + saleId + ".txt");
        if (!rfile.is_open())
        {
            cerr << "Error creating receipt file!" << endl;
            return;
        }
        sql::PreparedStatement* pstmt = con->prepareStatement("select * from sale join staff using (staffId) where saleId = ?");
        pstmt->setString(1, saleId);
        sql::ResultSet* res1 = pstmt->executeQuery();
        if (!res1->next())
        {
            cout << "Sale not found!" << endl;
            return;
        }
        pstmt = con->prepareStatement("select * from sale_detail join product using (productId) where saleId = ?");
        pstmt->setString(1, saleId);
        sql::ResultSet* res2 = pstmt->executeQuery();

        rfile << fixed << setprecision(2);
        rfile << "----------------------------------------------------------------------------\n";
        rfile << "|       COMPUTER RETAIL OUTLET INVENTORY MONITORING SYSTEM - RECEIPT       |\n";
        rfile << "----------------------------------------------------------------------------\n";
        rfile << "| Sale ID: " << left << setw(55) << saleId << "         |\n";
        rfile << "| Date:    " << left << setw(55) << res1->getString("saleDate") << "         |\n";
        rfile << "| Payment: " << left << setw(55) << res1->getString("paymentMethod") << "         |\n";
        rfile << "| Staff:   " << left << setw(55) << res1->getString("staffName") << "         |\n";
        rfile << "----------------------------------------------------------------------------\n";
        rfile << "|                              ITEM DETAILS                                |\n";
        rfile << "----------------------------------------------------------------------------\n";
        rfile << "| No |                Product Description               | Qty | Amount     |\n";
        rfile << "----------------------------------------------------------------------------\n";
        int counter = 1;
        double subtotal = 0;
        while (res2->next())
        {
            string productName = res2->getString("productName");
            int qtt = res2->getInt("quantity");
            double total = res2->getDouble("total");
            subtotal += total;
            rfile << "| " << setw(2) << right << counter++ << " | " << left << setw(48) << productName << " | " << setw(2) << right << qtt << "  | " << "RM" << setw(8) << left << total << " |\n";
            double unitPrice = res2->getDouble("sellingPrice");
            rfile << "|    |   Unit: RM" << setw(30) << left << unitPrice << "         |     |            |\n";
        }
        rfile << "----------------------------------------------------------------------------\n";
        rfile << "|                                                                          |\n";
        rfile << "| Subtotal:                                                   RM" << setw(10) << subtotal << " |\n";
        if (type == 1)
        {
            rfile << "| Payment:                                                    RM" << setw(10) << pay << " |\n";
            rfile << "| Balance:                                                    RM" << setw(10) << pay - subtotal << " |\n";
        }
        else if (type == 2)
            rfile << "| Card ending: " + cardnum + "                                           RM" << setw(10) << subtotal << " | \n";
        else if (type == 3)
            rfile << "| QR ID: " + cardnum + "                                              RM" << setw(10) << subtotal << " | \n";
        rfile << "|                                                                          |\n";
        rfile << "----------------------------------------------------------------------------\n";
        rfile << "\n";
        rfile << "  Thank you for shopping with us!\n  For refunds, present this receipt to us physically.\n\n";
        rfile << "----------------------------------------------------------------------------\n";
        rfile.close();
        cout << "Receipt Saved.\n";

        delete res1;
        delete res2;
    }
    catch (sql::SQLException& e)
    {
        cerr << "Error creating receipt: " << e.what() << endl;
    }
}

void ReportService::showReceiptFile(const string& saleId)
{
    system("cls");
    ifstream ifile("Receipts\\receipt_" + saleId + ".txt");
    string l;
    if (ifile.is_open())
    {
        cout << "\nDisplaying: " << saleId << "\n\n";
        while (getline(ifile, l))
            cout << l << endl;
        ifile.close();
    }
    cout << "End of :" << saleId << "\n";
    system("pause");
}

void ReportService::showInvoiceFile(const string& invoiceId)
{
    system("cls");
    ifstream ifile("Invoices\\invoice_" + invoiceId + ".txt");
    string l;
    if (ifile.is_open())
    {
        cout << "\nDisplaying: " << invoiceId << "\n\n";
        while (getline(ifile, l))
            cout << l << endl;
        ifile.close();
    }
    cout << "End of :" << invoiceId << "\n";
    system("pause");
}

void ReportService::viewReceipts()
{
    sql::Connection* con = Database::instance().getConnection();
    system("cls");
    cout << bocyan << u8"                                 ╔═════════════════╗                           \n" << reset;
    cout << bocyan << u8"═════════════════════════════════╣ DISPLAY RECEIPT ╠═════════════════════════════════\n" << reset;
    cout << bocyan << u8"                                 ╚═════════════════╝                           \n" << reset;
    string filename;
    cout << "Enter receipt num(receipt_Q____)(@ to cancel)";
    cin >> filename;
    cin.ignore();
    if (filename == "@")
        return;
    filename = "Q" + filename;
    sql::PreparedStatement* pstmt = con->prepareStatement("select * from sale where saleId = ?");
    pstmt->setString(1, filename);
    sql::ResultSet* res = pstmt->executeQuery();
    cout << filename << "\n\n";
    while (res->rowsCount() == 0)
    {
        filename = "";
        cerr << "Receipt not found. Try again: ";
        cin >> filename;
        cin.ignore();
        filename = "Q" + filename;
        pstmt->setString(1, filename);
        res = pstmt->executeQuery();
    }
    showReceiptFile(filename);
}

void ReportService::viewInvoices()
{
    sql::Connection* con = Database::instance().getConnection();
    system("cls");
    cout << bocyan << u8"                                 ╔═════════════════╗                           \n" << reset;
    cout << bocyan << u8"═════════════════════════════════╣ DISPLAY INVOICE ╠═════════════════════════════════\n" << reset;
    cout << bocyan << u8"                                 ╚═════════════════╝                           \n" << reset;
    string filename;
    cout << "Enter receipt num(invoice_V____)(@ to cancel)";
    cin >> filename;
    cin.ignore();
    if (filename == "@")
        return;
    filename = "V" + filename;
    sql::PreparedStatement* pstmt = con->prepareStatement("select * from invoice where invoiceId = ?");
    pstmt->setString(1, filename);
    sql::ResultSet* res = pstmt->executeQuery();
    while (res->rowsCount() == 0)
    {
        filename = "";
        cerr << "Receipt not found. Try again: ";
        cin >> filename;
        cin.ignore();
        filename = "V" + filename;
        pstmt->setString(1, filename);
        res = pstmt->executeQuery();
    }
    showInvoiceFile(filename);
}

void ReportService::monthlySalesReport()
{
    sql::Connection* con = Database::instance().getConnection();
    int c = -1;
    string query;
    system("cls");
    cout << bocyan << u8"                                 ╔═════════════════════╗                           \n" << reset;
    cout << bocyan << u8"═════════════════════════════════╣ DISPLAY SALES CHART ╠═════════════════════════════════\n" << reset;
    cout << bocyan << u8"                                 ╚═════════════════════╝                           \n" << reset;
    cout << "1. Display sales group by month in certain year\n2. Display yearly sales\n3. Display each day sale between date range\n4. Display 3 years monthly sales\n0. Cancel\n";
    cout << "Enter your choice: ";
    cin >> c;
    c = Utils::integerCheck(c);
    while (c < 0 || c > 4)
    {
        cerr << "Invalid choice. Try again: ";
        cin >> c;
        c = Utils::integerCheck(c);
    }
    if (c == 1)
    {
        sql::ResultSet* res = nullptr;
        int smonth = -1, emonth = -1, year = -1;
        sql::PreparedStatement* pstmt;
        while (res == nullptr || res->rowsCount() == 0)
        {
            cout << "Enter your starting month in number(1-12): ";
            cin >> smonth;
            smonth = Utils::integerCheck(smonth);
            while (smonth < 1 || smonth > 12)
            {
                cerr << "Invalid month selection. Try again: ";
                cin >> smonth;
                smonth = Utils::integerCheck(smonth);
            }
            cout << "Enter your starting month in number(1-12): ";
            cin >> emonth;
            emonth = Utils::integerCheck(emonth);
            while ((emonth < 1 || emonth > 12) || emonth < smonth)
            {
                cerr << "Invalid month selection. Try again: ";
                cin >> emonth;
                emonth = Utils::integerCheck(emonth);
            }
            query = "select year(saleDate) as year, month(saleDate) as month, sum(subtotal - refundTotal) as monthlySale from sale where year(saleDate) = ? and month(saleDate) between ? and ? group by year, month order by year, month";
            pstmt = con->prepareStatement(query);
            cout << "Enter your year choice: ";
            cin >> year;
            year = Utils::integerCheck(year);
            pstmt->setInt(1, year);
            pstmt->setInt(2, smonth);
            pstmt->setInt(3, emonth);
            res = pstmt->executeQuery();
            cerr << "No data found in this timeframe. Try again.\n";
        }
        res->next();
        system("cls");
        cout << endl;
        cout << bocyan << u8"══════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════\n" << reset;
        cout << left << setw(53) << " " << " " << year << " " << Utils::checkmonth(smonth) << " to " << Utils::checkmonth(emonth) << " SALES BAR CHART  " << left << setw(53) << " ";
        cout << endl;
        cout << bocyan << u8"══════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════\n\n" << reset;
        cout << "\n\n";
        while (!res->isAfterLast())
        {
            int boxnum = res->getDouble("monthlySale") / 1000;
            cout << left << setw(16) << "   " + Utils::checkmonth(res->getInt("month"));
            for (int i = 0; i < boxnum; i++)
                cout << u8"▇";
            cout << right << setw(109 - boxnum) << "RM " << res->getDouble("monthlySale") << endl;
            res->next();
        }
        cout << u8"\n\t\t├─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┤" << endl;
        cout << "\tRM(k)\t0         10        20        30        40        50        60        70        80        90       100" << endl << endl;
        cout << bocyan << u8"══════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════\n" << reset;
    }
    else if (c == 2)
    {
        int year = -1;
        query = "select year(saleDate) as year, month(saleDate) as month, sum(subtotal - refundTotal) as monthlySale from sale where year(saleDate) = ? group by year, month order by year, month";
        sql::PreparedStatement* pstmt = con->prepareStatement(query);
        cout << "Enter your year choice: ";
        cin >> year;
        year = Utils::integerCheck(year);
        pstmt->setInt(1, year);
        sql::ResultSet* res = pstmt->executeQuery();
        while (res->rowsCount() == 0)
        {
            cerr << "No data found in this year. Try again: ";
            cin >> year;
            year = Utils::integerCheck(year);
            pstmt->setInt(1, year);
            res = pstmt->executeQuery();
        }
        double totalSales = 0;
        while (res->next())
            totalSales += res->getDouble("monthlySale");
        res->beforeFirst();
        system("cls");
        cout << endl;
        cout << bocyan << u8"══════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════\n" << reset;
        cout << left << setw(62) << " " << " " << year << " SALES BAR CHART  " << left << setw(62) << " ";
        cout << endl;
        cout << bocyan << u8"══════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════\n\n" << reset;
        cout << right << setw(125) << "Total Sales: RM " << totalSales << endl << endl;
        while (res->next())
        {
            int boxnum = res->getDouble("monthlySale") / 1000;
            cout << left << setw(16) << "   " + Utils::checkmonth(res->getInt("month"));
            for (int i = 0; i < boxnum; i++)
                cout << u8"▇";
            cout << right << setw(109 - boxnum) << "RM " << res->getDouble("monthlySale") << endl;
        }
        cout << u8"\n\t\t├─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┤" << endl;
        cout << "\tRM(k)\t0         10        20        30        40        50        60        70        80        90       100" << endl << endl;
        cout << bocyan << u8"══════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════\n" << reset;
    }
    else if (c == 3)
    {
        cout << "\n";
        string start, end;
        regex datepattern("\\d{4}-\\d{2}-\\d{2}");
        query = "select saleDate, (subtotal - refundTotal) as daySale from sale where saleDate between ? and ?";
        sql::PreparedStatement* pstmt = con->prepareStatement(query);
        sql::ResultSet* res;
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
            pstmt->setString(1, start);
            pstmt->setString(2, end);
            res = pstmt->executeQuery();
            if (res->rowsCount() == 0)
                cerr << "No data found in this date range. Try Re-";
        } while (res->rowsCount() == 0);
        double totalSales = 0;
        while (res->next())
            totalSales += res->getDouble("daySale");
        res->beforeFirst();
        system("cls");
        cout << endl;
        cout << bocyan << u8"══════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════\n" << reset;
        cout << left << setw(52) << " " << start << " to " << end << " SALES BAR CHART  " << left << setw(52) << " ";
        cout << endl;
        cout << bocyan << u8"══════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════\n\n" << reset;
        while (res->next())
        {
            int boxnum = res->getDouble("daySale") / 1000;
            cout << left << setw(16) << "   " + res->getString("saleDate");
            for (int i = 0; i < boxnum; i++)
                cout << u8"▇";
            cout << right << setw(109 - boxnum) << "RM " << res->getDouble("daySale") << endl;
        }
        cout << u8"\n\t\t├─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┤" << endl;
        cout << "\tRM(k)\t0         10        20        30        40        50        60        70        80        90       100" << endl << endl;
        cout << right << setw(21) << " Total num of sale: " << res->rowsCount() << "\n";
        cout << right << setw(25) << " Total Sales: RM " << totalSales << "\n";
        cout << right << setw(25) << " Average Sale: RM " << (totalSales / res->rowsCount()) << "\n\n";
        cout << bocyan << u8"══════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════\n" << reset;
    }
    else if (c == 4)
    {
        sql::Statement* stmt = con->createStatement();
        struct YearSale { string year; double total = 0; } ys[3];
        int syear = -1, eyear = -1;
        cout << "Enter start year(-1 to cancel): ";
        cin >> syear;
        syear = Utils::integerCheck(syear);
        if (syear == -1)
            return;
        sql::ResultSet* res = stmt->executeQuery("select year(saleDate) as year from sale where year(saleDate) = " + to_string(syear) + " group by year order by year asc");
        while (res->rowsCount() == 0)
        {
            if (syear == -1)
                return;
            cerr << "No data found in this year. Try again: ";
            cin >> syear;
            syear = Utils::integerCheck(syear);
            res = stmt->executeQuery("select year(saleDate) as year from sale where year(saleDate) = " + to_string(syear) + " group by year order by year asc");
        }
        cout << "Enter end year(-1 to cancel): ";
        cin >> eyear;
        eyear = Utils::integerCheck(eyear);
        if (eyear == -1)
            return;
        res = stmt->executeQuery("select year(saleDate) as year from sale where year(saleDate) = " + to_string(eyear) + " group by year order by year asc");
        while (res->rowsCount() == 0 || eyear < syear || (eyear - syear) > 2)
        {
            if (eyear == -1)
                return;
            if (res->rowsCount() == 0)
                cerr << "No data found in this year. Try again: ";
            else if (eyear < syear)
                cerr << "Invalid Input. Try again: ";
            else
                cerr << "Year selection too far away. Try again where difference within 3 years: ";
            cin >> eyear;
            eyear = Utils::integerCheck(eyear);
            res = stmt->executeQuery("select year(saleDate) as year from sale where year(saleDate) = " + to_string(eyear) + " group by year order by year asc");
        }
        query = "select year(saleDate) as year, month(saleDate) as month, sum(subtotal - refundTotal) as monthlySale from sale where year(saleDate) between ? and ? group by month, year order by month, year asc";
        sql::PreparedStatement* pstmt = con->prepareStatement(query);
        pstmt->setInt(1, syear);
        pstmt->setInt(2, eyear);
        res = pstmt->executeQuery();
        int x;
        for (x = 0; x <= (eyear - syear); x++)
            ys[x].year = to_string(syear + x);
        while (res->next())
        {
            for (int i = 0; i < 3; i++)
            {
                if (ys[i].year == "")
                    break;
                if (res->getInt("year") == stoi(ys[i].year))
                    ys[i].total += res->getDouble("monthlySale");
            }
        }
        res->beforeFirst();
        system("cls");
        cout << endl;
        cout << bocyan << u8"══════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════\n" << reset;
        cout << left << setw(64) << " " << " ALL YEAR SALES BAR CHART  " << left << setw(64) << " ";
        cout << endl;
        cout << bocyan << u8"══════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════\n\n" << reset;
        string temp;
        while (res->next())
        {
            int boxnum = res->getDouble("monthlySale") / 1000;
            if (Utils::checkmonth(res->getInt("month")) != temp)
            {
                cout << left << setw(16) << "   " + Utils::checkmonth(res->getInt("month"));
                temp = Utils::checkmonth(res->getInt("month"));
            }
            else
                cout << left << setw(16) << " ";
            for (int i = 0; i < boxnum; i++)
            {
                if (res->getInt("year") == syear + 0)
                    cout << red << u8"▇" << reset;
                else if (res->getInt("year") == syear + 1)
                    cout << yellow << u8"▇" << reset;
                else if (res->getInt("year") == eyear)
                    cout << cyan << u8"▇" << reset;
            }
            cout << right << setw(109 - boxnum) << "RM " << res->getDouble("monthlySale") << endl;
        }
        cout << u8"\n\t\t├─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┤" << endl;
        cout << "\tRM(k)\t0         10        20        30        40        50        60        70        80        90       100" << endl << endl;
        for (int i = 0; i < 3; i++)
        {
            if (ys[i].year == "")
                break;
            if (i == 0)
                cout << red << u8"\n\t\t▇ = " << ys[i].year << reset;
            else if (i == 1)
                cout << yellow << u8"\n\t\t▇ = " << ys[i].year << reset;
            else if (i == 2)
                cout << cyan << u8"\n\t\t▇ = " << ys[i].year << reset;
        }
        cout << "\n\n";
        for (int i = 0; i < 3; i++)
        {
            if (ys[i].year == "")
                break;
            cout << right << setw(25) << ys[i].year + " Total Sales: RM " << ys[i].total << "\n";
        }
        cout << endl;
        cout << bocyan << u8"══════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════\n" << reset;
    }
    cout << "\n";
    system("pause");
}

void ReportService::monthlyOrderReport()
{
    sql::Connection* con = Database::instance().getConnection();
    int c = -1;
    string query;
    system("cls");
    cout << bocyan << u8"                                 ╔═══════════════════════════╗                           \n" << reset;
    cout << bocyan << u8"═════════════════════════════════╣ DISPLAY STOCK ORDER CHART ╠═════════════════════════════════\n" << reset;
    cout << bocyan << u8"                                 ╚═══════════════════════════╝                           \n" << reset;
    cout << "1. Display stock ordering cost group by month in certain year\n2. Display yearly stock ordering cost\n3. Display each day stock ordering expenses between date range\n4. Display 3 years monthly stock ordering cost\n0. Cancel\n";
    cout << "Enter your choice: ";
    cin >> c;
    c = Utils::integerCheck(c);
    while (c < 0 || c > 4)
    {
        cerr << "Invalid choice. Try again: ";
        cin >> c;
        c = Utils::integerCheck(c);
    }
    if (c == 1)
    {
        sql::ResultSet* res = nullptr;
        int smonth = -1, emonth = -1, year = -1;
        sql::PreparedStatement* pstmt;
        while (res == nullptr || res->rowsCount() == 0)
        {
            cout << "Enter your starting month in number(1-12): ";
            cin >> smonth;
            smonth = Utils::integerCheck(smonth);
            while (smonth < 1 || smonth > 12)
            {
                cerr << "Invalid month selection. Try again: ";
                cin >> smonth;
                smonth = Utils::integerCheck(smonth);
            }
            cout << "Enter your starting month in number(1-12): ";
            cin >> emonth;
            emonth = Utils::integerCheck(emonth);
            while ((emonth < 1 || emonth > 12) || emonth < smonth)
            {
                cerr << "Invalid month selection. Try again: ";
                cin >> emonth;
                emonth = Utils::integerCheck(emonth);
            }
            query = "select year(invoiceDate) as year, month(invoiceDate) as month, sum(subtotal) as monthlyOrder from invoice where year(invoiceDate) = ? and month(invoiceDate) between ? and ? group by month, year order by month, year asc";
            pstmt = con->prepareStatement(query);
            cout << "Enter your year choice: ";
            cin >> year;
            year = Utils::integerCheck(year);
            pstmt->setInt(1, year);
            pstmt->setInt(2, smonth);
            pstmt->setInt(3, emonth);
            res = pstmt->executeQuery();
            cerr << "No data found in this year. Try again.\n";
        }
        res->next();
        system("cls");
        cout << endl;
        cout << bocyan << u8"══════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════\n" << reset;
        cout << left << setw(47) << " " << " " << year << " " << Utils::checkmonth(smonth) << " to " << Utils::checkmonth(emonth) << " STOCK ORDERING COSTS BAR CHART  " << left << setw(47) << " ";
        cout << endl;
        cout << bocyan << u8"══════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════\n\n" << reset;
        cout << "\n\n";
        while (!res->isAfterLast())
        {
            int boxnum = res->getDouble("monthlyOrder") / 5000;
            cout << left << setw(16) << "   " + Utils::checkmonth(res->getInt("month"));
            for (int i = 0; i < boxnum; i++)
                cout << u8"▇";
            cout << right << setw(109 - boxnum) << "RM " << res->getDouble("monthlyOrder") << endl;
            res->next();
        }
        cout << u8"\n\t\t├─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┤" << endl;
        cout << "\tRM(k)\t0         50       100       150       200       250       300       350       400       450       500" << endl << endl;
        cout << bocyan << u8"══════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════\n" << reset;
    }
    else if (c == 2)
    {
        int year = -1;
        query = "select year(invoiceDate) as year, month(invoiceDate) as month, sum(subtotal) as monthlyOrder from invoice where year(invoiceDate) = ? group by month, year order by month, year asc";
        sql::PreparedStatement* pstmt = con->prepareStatement(query);
        cout << "Enter your year choice: ";
        cin >> year;
        year = Utils::integerCheck(year);
        pstmt->setInt(1, year);
        sql::ResultSet* res = pstmt->executeQuery();
        while (res->rowsCount() == 0)
        {
            cerr << "No data found in this year. Try again: ";
            cin >> year;
            year = Utils::integerCheck(year);
            pstmt->setInt(1, year);
            res = pstmt->executeQuery();
        }
        double totalSales = 0;
        while (res->next())
            totalSales += res->getDouble("monthlyOrder");
        res->beforeFirst();
        system("cls");
        cout << endl;
        cout << bocyan << u8"══════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════\n" << reset;
        cout << left << setw(58) << " " << " " << year << " STOCK ORDERING BAR CHART  " << left << setw(58) << " ";
        cout << endl;
        cout << bocyan << u8"══════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════\n\n" << reset;
        cout << right << setw(125) << "Total Sales: RM " << totalSales << endl << endl;
        while (res->next())
        {
            int boxnum = res->getDouble("monthlyOrder") / 5000;
            cout << left << setw(16) << "   " + Utils::checkmonth(res->getInt("month"));
            for (int i = 0; i < boxnum; i++)
                cout << u8"▇";
            cout << right << setw(109 - boxnum) << "RM " << res->getDouble("monthlyOrder") << endl;
        }
        cout << u8"\n\t\t├─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┤" << endl;
        cout << "\tRM(k)\t0         50       100       150       200       250       300       350       400       450       500" << endl << endl;
        cout << bocyan << u8"══════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════\n" << reset;
    }
    else if (c == 3)
    {
        cout << "\n";
        string start, end;
        regex datepattern("\\d{4}-\\d{2}-\\d{2}");
        query = "select invoiceDate, (subtotal) as dayOrder from invoice where invoiceDate between ? and ?";
        sql::PreparedStatement* pstmt = con->prepareStatement(query);
        sql::ResultSet* res;
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
            pstmt->setString(1, start);
            pstmt->setString(2, end);
            res = pstmt->executeQuery();
            if (res->rowsCount() == 0)
                cerr << "No data found in this date range. Try Re-";
        } while (res->rowsCount() == 0);
        double totalSales = 0;
        while (res->next())
            totalSales += res->getDouble("dayOrder");
        res->beforeFirst();
        system("cls");
        cout << endl;
        cout << bocyan << u8"══════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════\n" << reset;
        cout << left << setw(52) << " " << start << " to " << end << " STOCK ORDER BAR CHART  " << left << setw(52) << " ";
        cout << endl;
        cout << bocyan << u8"══════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════\n\n" << reset;
        while (res->next())
        {
            int boxnum = res->getDouble("dayOrder") / 5000;
            cout << left << setw(16) << "   " + res->getString("invoiceDate");
            for (int i = 0; i < boxnum; i++)
                cout << u8"▇";
            cout << right << setw(109 - boxnum) << "RM " << res->getDouble("dayOrder") << endl;
        }
        cout << u8"\n\t\t├─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┤" << endl;
        cout << "\tRM(k)\t0         50       100       150       200       250       300       350       400       450       500" << endl << endl;
        cout << right << setw(21) << " Total num of stock order: " << res->rowsCount() << "\n";
        cout << right << setw(25) << " Total Cost: RM " << totalSales << "\n";
        cout << right << setw(25) << " Average Cost: RM " << (totalSales / res->rowsCount()) << "\n\n";
        cout << bocyan << u8"══════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════\n" << reset;
    }
    else if (c == 4)
    {
        sql::Statement* stmt = con->createStatement();
        struct YearSale { string year; double total = 0; } ys[3];
        int syear = -1, eyear = -1;
        cout << "Enter start year(-1 to cancel): ";
        cin >> syear;
        syear = Utils::integerCheck(syear);
        if (syear == -1)
            return;
        sql::ResultSet* res = stmt->executeQuery("select year(invoiceDate) as year from invoice group by year order by year asc");
        while (res->rowsCount() == 0)
        {
            if (syear == -1)
                return;
            cerr << "No data found in this year. Try again: ";
            cin >> syear;
            syear = Utils::integerCheck(syear);
            res = stmt->executeQuery("select year(invoiceDate) as year from invoice group by year order by year asc");
        }
        cout << "Enter end year(-1 to cancel): ";
        cin >> eyear;
        eyear = Utils::integerCheck(eyear);
        if (eyear == -1)
            return;
        res = stmt->executeQuery("select year(invoiceDate) as year from invoice group by year order by year asc");
        while (res->rowsCount() == 0 || eyear < syear || (eyear - syear) > 2)
        {
            if (eyear == -1)
                return;
            if (res->rowsCount() == 0)
                cerr << "No data found in this year. Try again: ";
            else if (eyear < syear)
                cerr << "Invalid Input. Try again: ";
            else
                cerr << "Year selection too far away. Try again where difference within 3 years: ";
            cin >> eyear;
            eyear = Utils::integerCheck(eyear);
            res = stmt->executeQuery("select year(invoiceDate) as year from invoice group by year order by year asc");
        }
        query = "select year(invoiceDate) as year, month(invoiceDate) as month, sum(subtotal) as monthlyOrder from invoice where year(invoiceDate) between ? and ? group by month, year order by month, year asc";
        sql::PreparedStatement* pstmt = con->prepareStatement(query);
        pstmt->setInt(1, syear);
        pstmt->setInt(2, eyear);
        res = pstmt->executeQuery();
        int x;
        for (x = 0; x <= (eyear - syear); x++)
            ys[x].year = to_string(syear + x);
        while (res->next())
        {
            for (int i = 0; i < 3; i++)
            {
                if (ys[i].year == "")
                    break;
                if (res->getString("year") == ys[i].year)
                    ys[i].total += res->getDouble("monthlyOrder");
            }
        }
        res->beforeFirst();
        system("cls");
        cout << endl;
        cout << bocyan << u8"══════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════\n" << reset;
        cout << left << setw(55) << " " << " ALL YEAR STOCK ORDERING COSTS BAR CHART  " << left << setw(55) << " ";
        cout << endl;
        cout << bocyan << u8"══════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════\n\n" << reset;
        string temp;
        while (res->next())
        {
            int boxnum = res->getDouble("monthlyOrder") / 5000;
            if (Utils::checkmonth(res->getInt("month")) != temp)
            {
                cout << left << setw(16) << "   " + Utils::checkmonth(res->getInt("month"));
                temp = Utils::checkmonth(res->getInt("month"));
            }
            else
                cout << left << setw(16) << " ";
            for (int i = 0; i < boxnum; i++)
            {
                if (res->getInt("year") == syear + 0)
                    cout << red << u8"▇" << reset;
                else if (res->getInt("year") == syear + 1)
                    cout << yellow << u8"▇" << reset;
                else if (res->getInt("year") == eyear)
                    cout << cyan << u8"▇" << reset;
            }
            cout << right << setw(109 - boxnum) << "RM " << res->getDouble("monthlyOrder") << endl;
        }
        cout << u8"\n\t\t├─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┤" << endl;
        cout << "\tRM(k)\t0         50       100       150       200       250       300       350       400       450       500" << endl << endl;
        for (int i = 0; i < 3; i++)
        {
            if (ys[i].year == "")
                break;
            if (i == 0)
                cout << red << u8"\n\t\t▇ = " << ys[i].year << reset;
            else if (i == 1)
                cout << yellow << u8"\n\t\t▇ = " << ys[i].year << reset;
            else if (i == 2)
                cout << cyan << u8"\n\t\t▇ = " << ys[i].year << reset;
        }
        cout << "\n\n";
        for (int i = 0; i < 3; i++)
        {
            if (ys[i].year == "")
                break;
            cout << right << setw(25) << ys[i].year + " Total Cost: RM " << ys[i].total << "\n";
        }
        cout << endl;
        cout << bocyan << u8"══════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════════\n" << reset;
    }
    cout << "\n";
    system("pause");
}

void ReportService::productPopularityReport()
{
    sql::Connection* con = Database::instance().getConnection();
    system("cls");
    cout << bocyan << u8"                                 ╔══════════════════╗                           \n" << reset;
    cout << bocyan << u8"═════════════════════════════════╣ PRODUCT ANALYSIS ╠═════════════════════════════════\n" << reset;
    cout << bocyan << u8"                                 ╚══════════════════╝                           \n" << reset;

    sql::Statement* stmt = con->createStatement();
    sql::ResultSet* res1 = stmt->executeQuery("select year(saleDate) as year from sale group by year order by year asc");
    vector<int> year;
    while (res1->next())
        year.push_back(res1->getInt("year"));

    sql::PreparedStatement* pstmt1 = con->prepareStatement(
        "select sum(sd.quantity) as quantitySold, p.productName, p.productId from sale_detail sd join product p using (productId) join sale using (saleId) where year(saleDate) = ? group by p.productName, p.productId order by quantitySold desc");
    sql::PreparedStatement* pstmt2 = con->prepareStatement(
        "select count(*) as numSale from sale_detail join sale using (saleId) where productId = ? and year(saleDate) = ?");

    struct HighLowProduct { int quantitySold = 0; string productName; int numOfSale = 0; };
    vector<HighLowProduct> highest(year.size()), lowest(year.size());

    for (size_t i = 0; i < year.size(); i++)
    {
        pstmt1->setInt(1, year[i]);
        sql::ResultSet* res = pstmt1->executeQuery();
        res->next();
        string productId = res->getString("productId");
        pstmt2->setString(1, productId);
        pstmt2->setInt(2, year[i]);
        sql::ResultSet* res2 = pstmt2->executeQuery();
        res2->next();
        highest[i].productName = res->getString("productName");
        highest[i].quantitySold = res->getInt("quantitySold");
        highest[i].numOfSale = res2->getInt("numSale");

        res->afterLast();
        res->previous();
        productId = res->getString("productId");
        pstmt2->setString(1, productId);
        pstmt2->setInt(2, year[i]);
        res2 = pstmt2->executeQuery();
        res2->next();
        lowest[i].productName = res->getString("productName");
        lowest[i].quantitySold = res->getInt("quantitySold");
        lowest[i].numOfSale = res2->getInt("numSale");
    }

    cout << left << setw(60) << "\tHighest popularity product in each year: \n" << endl;
    for (size_t j = 0; j < year.size(); j++)
        cout << right << setw(5) << year[j] << ": " << left << setw(46) << highest[j].productName << " - " << highest[j].quantitySold << " sold throughout " << highest[j].numOfSale << " sales\t\t" << endl;
    cout << "\n";
    cout << left << setw(60) << "\tLowest popularity product in each year: \n" << endl;
    for (size_t j = 0; j < year.size(); j++)
        cout << right << setw(5) << year[j] << ": " << left << setw(46) << lowest[j].productName << " - " << lowest[j].quantitySold << " sold throughout " << lowest[j].numOfSale << " sales\t\t" << endl;
    cout << endl;
    cout << bocyan << u8"══════════════════════════════════════════════════════════════════════════════════════\n" << reset;
    system("pause");
}