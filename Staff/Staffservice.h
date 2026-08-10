#pragma once
#include <string>

// All staff-related DB operations (DFD 1.0 Staff Authentication, 2.0 Manage
// Staff). Pulls its connection from Database::instance() instead of taking
// a sql::Connection* parameter — that's what let every original free
// function drop its first argument.
class StaffService
{
public:
    void registerStaff();
    bool login();           // returns true on successful login; populates Session::instance()
    void viewAll();
    void update();
    void deactivate();
    void search();

private:
    // Shared by viewAll() and search(): both original functions duplicated
    // an identical paginated-table renderer. baseQuery is the unpaginated
    // "select ... from staff where ..." to page through.
    void displayPaginated(const std::string& baseQuery);
};