#pragma once
#include <string>
#include "Sale/Cart.h"
#include "Product/Product.h"
#include "Supplier/Supplier.h"

namespace sql { class ResultSet; }

struct StockOrder
{
    std::string orderId;
    std::string orderDate;
    std::string staffId;
    std::string supplierId;
};

struct OrderDetail
{
    std::string orderDetailId;
    std::string productId;
    std::string orderId;
    int orderQuantity = 0;
    int remainingQuantity = 0;
    bool status = true;
    double itemTotal = 0.0;
};

// DFD 5.0 Supervise Stock Order. generateinvoice/createinvoice (5.5) live in
// InventoryService since INVOICE feeds INVENTORY in the ERD.
class StockOrderService
{
public:
    void createOrder();                   // 5.1
    sql::ResultSet* viewOrders(int type);  // 5.2 — type 1: interactive (offers filter/show-detail), type 2: quiet listing
    void searchOrder();                   // 5.3
    void refundOrder();                   // 5.4

private:
    void createOrderDetail(const std::string& productId, const std::string& orderId, int quantity, double indiTotal);
    void showOrderDetail(sql::ResultSet* res);
    sql::ResultSet* filterOrder(sql::ResultSet* res); // reached from viewOrders' submenu

    void printCart(const Cart& cart);              // stock order cart during createOrder
    void updateCartInteractively(Cart& cart);
    void printRefundCart(const Cart& cart);         // refund selection cart during refundOrder

    ProductService productService;
    SupplierService supplierService;
};