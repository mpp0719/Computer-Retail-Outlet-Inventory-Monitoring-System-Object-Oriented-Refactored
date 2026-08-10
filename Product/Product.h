#pragma once
#include <string>

namespace sql { class ResultSet; } // forward declaration, avoids pulling the MySQL headers into every file that includes Product.h

// Matches the PRODUCT table in the ERD.
struct Product
{
    std::string productId;
    std::string productName;
    std::string productType;
    double basePrice = 0.0;
    int quantity = 0;
    double sellingPrice = 0.0;
    bool status = true;
};

// DFD 3.0 Handle Product. Fully ported from original main.cpp:
//   registerproduct   -> line 764
//   searchproduct     -> line 839  (split into searchSortProduct + searchForSelection below)
//   removeproduct     -> line 1160
//   updateproduct     -> line 1238
//   viewallproduct    -> line 1452 (split into viewAll + listAllForSelection below)
class ProductService
{
public:
    // 3.1 Register Product
    void registerProduct();

    // 3.2 Search/Sort Product — standalone menu item (was searchproduct(con, 0, ...))
    void searchSortProduct();

    // 3.3 Remove Product
    void removeProduct();

    // 3.4 Update Product.
    // autoUpdate == false: interactive field-by-field update (menu-driven).
    // autoUpdate == true: silent quantity adjustment used internally by
    //   SaleService/StockOrderService — updateType 1 subtracts `quantity`
    //   (a sale, or a stock-order refund), updateType 2 adds it back
    //   (a sale refund, or stock received).
    void updateProduct(const std::string& productId, bool autoUpdate, int updateType, int quantity);

    // 3.5 View Product — standalone menu item, offers filter/sort/view-all
    // (was viewallproduct(con, 0))
    void viewAll();

    // Simple unfiltered paginated listing with no submenu, used by other
    // services (e.g. shown before RemoveProduct/UpdateProduct prompt for an
    // ID). Was viewallproduct(con, 1).
    void listAllForSelection();

    // Search used internally by SaleService/StockOrderService when picking
    // a product to add to a cart. Was searchproduct(con, 1, res); returns
    // the raw result set so the caller can read the chosen row.
    sql::ResultSet* searchForSelection();

private:
    // Shared by every product listing above — they all render the exact
    // same 8-column table, just with a different WHERE/ORDER BY clause.
    // Returns false (and prints "No product found.") if the query matched
    // zero rows, so callers that chain more prompts after the listing
    // (like searchSortProduct's "Sort? y/n") know whether to bother asking.
    bool displayPaginated(const std::string& baseQuery);
};