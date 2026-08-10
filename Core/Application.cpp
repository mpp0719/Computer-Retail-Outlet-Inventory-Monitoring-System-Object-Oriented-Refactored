#include "Core/Application.h"
#include "Core/Database.h"
#include "Core/Utils.h"
#include "Staff/Staff.h"
#include "External/ansi.h"
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <iostream>
#include <iomanip>

using namespace std;

bool Application::startup()
{
    cout << fixed << setprecision(2);
    Utils::initConsole();
    cout << endl;

    // Same credentials as the original global consts — TODO: move to config/env.
    if (!Database::instance().connect("tcp://127.0.0.1:3306", "root", "1234"))
    {
        exit(1);
    }

    staffService.login(); // populates Session::instance()
    return true;
}

void Application::run()
{
    int mchoice;
    Session& session = Session::instance();
    do
    {
        system("cls");
        inventoryService.notifyLowStock(); // was notifystocklow(con)

        cout << boyellow << u8"┌─────────────────────────────────────────────────────────────────┐\n" << reset;
        cout << boyellow << u8"│  WELCOME TO COMPUTER RETAIL OUTLET INVENTORY MONITORING SYSTEM  │\n" << reset;
        cout << boyellow << u8"└─────────────────────────────────────────────────────────────────┘\n\n" << reset;
        cout << bocyan << u8"                           ╔═══════════╗                           \n" << reset;
        cout << bocyan << u8"═══════════════════════════╣ MAIN MENU ╠═══════════════════════════\n" << reset;
        cout << bocyan << u8"                           ╚═══════════╝                           \n" << reset;
        cout << yellow << "Menu: \n" << reset;
        if (session.isManager)
            cout << "1. Handle Product\n2. Record Sales\n3. Stock ordering\n" << pink << "4. Reporting\n5. Manage Staff\n6. Manage Supplier\n0. Exit Program" << reset << "\n99. Logout\n\n";
        else
            cout << "1. Handle Product\n2. Record Sales\n3. Stock ordering\n0. Exit Program\n99. Logout\n\n";

        cout << bocyan << u8"═══════════════════════════════════════════════════════════════════\n" << reset;
        cout << "Enter your choice: ";
        cin >> mchoice;
        mchoice = Utils::integerCheck(mchoice);
        cin.ignore();

        if (mchoice == 1)
            handleProductMenu();
        else if (mchoice == 2)
            manageSaleMenu();
        else if (mchoice == 3)
            manageStockOrderMenu();
        else if (mchoice == 4 && session.isManager)
            reportingMenu();
        else if (mchoice == 5 && session.isManager)
            manageStaffMenu();
        else if (mchoice == 6 && session.isManager)
            manageSupplierMenu();
        else if (mchoice == 99)
            staffService.login();
        else if (mchoice == 0)
            break;
    } while (true);
}

void Application::handleProductMenu()
{
    int choice;
    do
    {
        system("cls");
        cout << bocyan << u8"                                 ╔════════════════╗                           \n" << reset;
        cout << bocyan << u8"═════════════════════════════════╣ HANDLE PRODUCT ╠═════════════════════════════════\n" << reset;
        cout << bocyan << u8"                                 ╚════════════════╝                           \n" << reset;
        cout << "1. Register Product\n2. Update Product\n3. Remove Product\n4. Search/Sort Product\n5. View Product\n0. Back to Main Menu\n\n";
        cout << bocyan << u8"════════════════════════════════════════════════════════════════════════════════════\n" << reset;
        cout << "Enter your choice: ";
        do
        {
            cin >> choice;
            choice = Utils::integerCheck(choice);
            cin.ignore();
            if (choice == 1)
                productService.registerProduct();
            else if (choice == 2)
            {
                // Ported from main()'s menu directly (original prompted for
                // and validated the productId here, not inside updateproduct).
                string productid;
                productService.listAllForSelection();
                cin.ignore();
                cout << "Enter the Product ID that you want to update(@ to cancel): ";
                getline(cin, productid);
                sql::PreparedStatement* pstmt = Database::instance().getConnection()->prepareStatement(
                    "select * from product where productId = ?");
                pstmt->setString(1, productid);
                sql::ResultSet* res = pstmt->executeQuery();
                while (!res->next() && productid != "@")
                {
                    cerr << "Product not found. Please try again: ";
                    cin >> productid;
                    pstmt->setString(1, productid);
                    res = pstmt->executeQuery();
                }
                if (productid == "@")
                    continue;
                productService.updateProduct(productid, false, 0, 0);
            }
            else if (choice == 3)
                productService.removeProduct();
            else if (choice == 4)
                productService.searchSortProduct();
            else if (choice == 5)
                productService.viewAll();
            else if (choice == 0)
                continue;
            else
                cerr << "Invalid choice. Try again: ";
        } while (choice < 0 || choice > 5);
    } while (choice != 0);
}

void Application::manageSaleMenu()
{
    int choice;
    do
    {
        system("cls");
        cout << bocyan << u8"                                 ╔═════════════╗                           \n" << reset;
        cout << bocyan << u8"═════════════════════════════════╣ MANAGE SALE ╠═════════════════════════════════\n" << reset;
        cout << bocyan << u8"                                 ╚═════════════╝                           \n" << reset;
        cout << "1. Register Sale\n2. Refund Sales\n3. View Sale\n0. Back to Main Menu\n\n";
        cout << bocyan << u8"═════════════════════════════════════════════════════════════════════════════════\n" << reset;
        cout << "Enter your choice: ";
        do
        {
            cin >> choice;
            choice = Utils::integerCheck(choice);
            cin.ignore();
            if (choice == 1)
                saleService.createSale();
            else if (choice == 2)
                saleService.refundSale();
            else if (choice == 3)
                saleService.viewSales(1);
            else if (choice == 0)
                continue;
            else
                cerr << "Invalid choice. Try again: ";
        } while (choice < 0 || choice > 3);
    } while (choice != 0);
}

void Application::manageStockOrderMenu()
{
    int choice;
    do
    {
        system("cls");
        cout << bocyan << u8"                                 ╔════════════════════╗                           \n" << reset;
        cout << bocyan << u8"═════════════════════════════════╣ MANAGE STOCK ORDER ╠═════════════════════════════════\n" << reset;
        cout << bocyan << u8"                                 ╚════════════════════╝                           \n" << reset;
        cout << "1. Make Stock Order\n2. View Stock Order\n3. Search Stock Order\n4. Stock Receive\n5. View Inventory\n6. Refund Stock Order Item\n0. Back to Main Menu\n\n";
        cout << bocyan << u8"════════════════════════════════════════════════════════════════════════════════════════\n" << reset;
        cout << "Enter your choice: ";
        do
        {
            cin >> choice;
            choice = Utils::integerCheck(choice);
            cin.ignore();
            if (choice == 1)
                stockOrderService.createOrder();
            else if (choice == 2)
                stockOrderService.viewOrders(1);
            else if (choice == 3)
                stockOrderService.searchOrder();
            else if (choice == 4)
                inventoryService.receiveStock();
            else if (choice == 5)
                inventoryService.viewInventory();
            else if (choice == 6)
                stockOrderService.refundOrder();
            else if (choice == 0)
                continue;
            else
                cerr << "Invalid choice. Try again: ";
        } while (choice < 0 || choice > 6);
    } while (choice != 0);
}

void Application::reportingMenu()
{
    int choice;
    do
    {
        system("cls");
        cout << pink << u8"                                 ╔══════════════════╗                           \n" << reset;
        cout << pink << u8"═════════════════════════════════╣ REPORT AND FILES ╠═════════════════════════════════\n" << reset;
        cout << pink << u8"                                 ╚══════════════════╝                           \n" << reset;
        cout << "1. View Receipts\n2. View Invoices\n3. Display sale\n4. Cost in stock ordering\n5. Highest popularity product\n0. Back to main menu\n\n";
        cout << pink << u8"══════════════════════════════════════════════════════════════════════════════════════\n" << reset;
        cout << "Enter your choice: ";
        do
        {
            cin >> choice;
            choice = Utils::integerCheck(choice);
            cin.ignore();
            if (choice == 1)
                reportService.viewReceipts();
            else if (choice == 2)
                reportService.viewInvoices();
            else if (choice == 3)
                reportService.monthlySalesReport();
            else if (choice == 4)
                reportService.monthlyOrderReport();
            else if (choice == 5)
                reportService.productPopularityReport();
            else if (choice == 0)
                continue;
            else
                cerr << "Invalid choice. Try again: ";
        } while (choice < 0 || choice > 5);
    } while (choice != 0);
}

void Application::manageStaffMenu()
{
    int choice;
    do
    {
        system("cls");
        cout << pink << u8"                                 ╔══════════════╗                           \n" << reset;
        cout << pink << u8"═════════════════════════════════╣ MANAGE STAFF ╠═════════════════════════════════\n" << reset;
        cout << pink << u8"                                 ╚══════════════╝                           \n" << reset;
        cout << "1. Register Staff\n2. Update Staff\n3. Deactivate Staff\n4. Search Staff\n5. View All Staff\n0. Back to Main menu\n\n";
        cout << pink << u8"══════════════════════════════════════════════════════════════════════════════\n" << reset;
        cout << "Enter your choice: ";
        do
        {
            cin >> choice;
            choice = Utils::integerCheck(choice);
            cin.ignore();
            if (choice == 1)
                staffService.registerStaff();
            else if (choice == 2)
                staffService.update();
            else if (choice == 3)
                staffService.deactivate();
            else if (choice == 4)
                staffService.search();
            else if (choice == 5)
                staffService.viewAll();
            else if (choice == 0)
                continue;
            else
                cerr << "Invalid choice. Try again: ";
        } while (choice < 0 || choice > 5);
    } while (choice != 0);
}

void Application::manageSupplierMenu()
{
    int choice;
    do
    {
        system("cls");
        cout << pink << u8"                                 ╔═════════════════╗                           \n" << reset;
        cout << pink << u8"═════════════════════════════════╣ MANAGE SUPPLIER ╠═════════════════════════════════\n" << reset;
        cout << pink << u8"                                 ╚═════════════════╝                           \n" << reset;
        cout << "1. Register Supplier\n2. Update Supplier\n3. Deactivate Supplier\n4. Search Supplier\n5. View Supplier\n0. Back to Main Menu\n\n";
        cout << pink << u8"═════════════════════════════════════════════════════════════════════════════════════\n" << reset;
        cout << "Enter your choice: ";
        do
        {
            cin >> choice;
            choice = Utils::integerCheck(choice);
            cin.ignore();
            if (choice == 1)
                supplierService.registerSupplier();
            else if (choice == 2)
                supplierService.update();
            else if (choice == 3)
                supplierService.deactivate();
            else if (choice == 4)
                supplierService.search();
            else if (choice == 5)
                supplierService.viewAll();
            else if (choice == 0)
                continue;
            else
                cerr << "Invalid choice. Try again: ";
        } while (choice < 0 || choice > 5);
    } while (choice != 0);
}