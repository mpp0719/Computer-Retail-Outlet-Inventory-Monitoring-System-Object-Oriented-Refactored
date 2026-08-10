#pragma once
#include <string>

// Matches SUPPLIER table.
struct Supplier
{
    std::string supplierId;
    std::string supplierName;
    std::string supplierPhone;
    std::string supplierAddress;
    std::string supplierEmail;
    bool status = true;
};

// Mirrors StaffService's shape (register/update/view/deactivate/search),
// fully ported from original main.cpp lines 3114-3584.
class SupplierService
{
public:
    void registerSupplier();
    void update();
    void viewAll();
    void deactivate();
    void search();

private:
    void displayPaginated(const std::string& baseQuery);
};