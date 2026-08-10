#pragma once
#include <string>
#include "Product/Product.h"
#include "Report/ReportService.h"

struct Invoice
{
    std::string invoiceId;
    std::string invoiceDate;
    double subtotal = 0.0;
    std::string supplierId;
};

struct InventoryReceipt
{
    std::string receiveId;
    std::string orderDetailId;
    std::string invoiceId;
    int receiveQuantity = 0;
};

// Matches the original's idtemp/oid[100] — items selected during a single
// stockreceive() session before they're committed to the DB. Local to
// receiveStock() as a vector, same as Cart is local to createSale().
struct ReceivingItem
{
    std::string orderDetailId;
    std::string productId;
    int remainingQuantity = 0;
    int receivedQty = 0;
};

// DFD 6.0 Organize Inventory (+ 5.5 Generate Invoice, which the ERD ties
// directly into INVENTORY).
class InventoryService
{
public:
    std::string createInvoiceRecord(double subtotal, const std::string& supplierId); // was createinvoice
    void receiveStock();                                                            // 6.3
    void generateInvoiceFile(const std::string& orderId, const std::string& invoiceId); // 5.5, was generateinvoice
    void viewInventory();                                                            // 6.4
    void notifyLowStock();                                                           // 6.2

private:
    void createInventoryRecord(const std::string& invoiceId, const std::string& orderDetailId, int receiveQty);

    ProductService productService;
    ReportService reportService;
};