#pragma once
#include "StaffService.h"
#include "Product.h"
#include "Sale.h"
#include "Supplier.h"
#include "StockOrder.h"
#include "Inventory.h"
#include "ReportService.h"

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