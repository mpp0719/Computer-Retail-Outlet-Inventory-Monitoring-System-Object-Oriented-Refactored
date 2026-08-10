#pragma once
#include <string>
#include <vector>

struct CartItem
{
    std::string productId;
    int quantity = 0;
    double indiTotal = 0.0;
};

// Replaces the global fixed-size arrays cart[100]/stockorder[100]/dsi[100]/
// refundstock[100]. Rendering/updating a cart still needs DB lookups (price,
// stock, etc.) that differ per use case, so that logic stays in the owning
// service (SaleService for now) — this class just holds the items.
class Cart
{
public:
    int find(const std::string& productId) const; // index, or -1 if not present
    void add(const CartItem& item) { items_.push_back(item); }
    void removeAt(size_t index);
    CartItem& at(size_t index) { return items_[index]; }
    const CartItem& at(size_t index) const { return items_[index]; }
    size_t size() const { return items_.size(); }
    bool empty() const { return items_.empty(); }
    double total() const;

private:
    std::vector<CartItem> items_;
};