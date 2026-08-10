#pragma once
#include <string>
#include "Product.h"
#include "Cart.h"
#include "ReportService.h"

namespace sql { class ResultSet; }

struct Sale
{
    std::string saleId;
    std::string paymentMethod;
    std::string saleDate;
    double subtotal = 0.0;
    double refundTotal = 0.0;
    std::string staffId;
};

struct SaleDetail
{
    std::string productId;
    std::string saleId;
    int quantity = 0;
    double total = 0.0;
    int refundQtt = 0;
    bool refundStatus = false;
};

// DFD 4.0 Record Sales.
class SaleService
{
public:
    void createSale();                  // 4.1
    sql::ResultSet* viewSales(int type); // 4.3 — type 1: interactive (offers filter/show-detail), type 2: quiet listing
    void refundSale();                  // 4.4

private:
    void createSaleDetail(const std::string& productId, const std::string& saleId, int quantity, double total);
    void showSaleDetail(sql::ResultSet* res);
    sql::ResultSet* filterSale(sql::ResultSet* res); // 4.2, reached from viewSales' submenu
    void applyRefundToDetail(const std::string& productId, const std::string& saleId, int quantity, int max);

    void printCart(const Cart& cart);
    void updateCartInteractively(Cart& cart);
    void printRefundCart(const Cart& cart);

    ProductService productService;
    ReportService reportService;
};