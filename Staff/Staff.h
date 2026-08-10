#pragma once
#include <string>

// Plain data matching the STAFF table in the ERD.
struct Staff
{
    std::string staffId;
    std::string staffName;
    std::string staffPos;
    std::string staffAddress;
    std::string staffEmail;
    std::string staffPhone;
    bool status = true;
};

// Replaces the global `staff LoggedOnstaff` object.
class Session
{
public:
    static Session& instance();

    std::string staffId;
    bool isManager = false;

    Session(const Session&) = delete;
    Session& operator=(const Session&) = delete;

private:
    Session() = default;
};