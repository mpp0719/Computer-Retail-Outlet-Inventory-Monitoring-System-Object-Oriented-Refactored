#pragma once
#include <string>

// DFD 7.0 Generate Report.
// TODO: port bodies from original main.cpp:
//   readfile (types 1/2) -> line 4872  (7.1 View Receipt / 7.2 View Invoice)
//   displaymonthlysales  -> line 5249  (7.3 Generate Sales Report)
//   displaymonthlyorder  -> line 5554  (7.4 Generate Stock Order Report)
//   productanalysis      -> line 5859  (7.5 Show Item Popularity)
// checkmonth is now Utils::checkmonth.
class ReportService
{
public:
    void createReceiptFile(const std::string& saleId, double pay, const std::string& cardNum, int type); // 7.5, called by SaleService::createSale at checkout
    void showReceiptFile(const std::string& saleId); // readfile(con, 3, saleId) — displays one receipt right after checkout
    void showInvoiceFile(const std::string& invoiceId); // readfile(con, 4, invoiceId) — displays one invoice right after stock receiving
    void viewReceipts();
    void viewInvoices();
    void monthlySalesReport();
    void monthlyOrderReport();
    void productPopularityReport();
};