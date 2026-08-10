#pragma once
#include "Staff/StaffService.h"
#include "Product/Product.h"
#include "Sale/Sale.h"
#include "Supplier/Supplier.h"
#include "Stockorder/StockOrder.h"
#include "Inventory/Inventory.h"
#include "Report/ReportService.h"

class Application
{
public:
    bool startup();   // connects DB, calls Utils::initConsole(), initial login
    void run();        // the main menu loop (was main()'s do/while)

private:
    void handleProductMenu();   // mchoice == 1
    void manageSaleMenu();      // mchoice == 2
    void manageStockOrderMenu();// mchoice == 3
    void reportingMenu();       // mchoice == 4 (manager only)
    void manageStaffMenu();     // mchoice == 5 (manager only)
    void manageSupplierMenu();  // mchoice == 6 (manager only)

    StaffService staffService;
    ProductService productService;
    SaleService saleService;
    SupplierService supplierService;
    StockOrderService stockOrderService;
    InventoryService inventoryService;
    ReportService reportService;
};