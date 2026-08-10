#include "Sale/Cart.h"

int Cart::find(const std::string& productId) const
{
    for (size_t i = 0; i < items_.size(); i++)
        if (items_[i].productId == productId)
            return (int)i;
    return -1;
}

void Cart::removeAt(size_t index)
{
    if (index < items_.size())
        items_.erase(items_.begin() + index);
}

double Cart::total() const
{
    double sum = 0;
    for (const auto& item : items_)
        sum += item.indiTotal;
    return sum;
}