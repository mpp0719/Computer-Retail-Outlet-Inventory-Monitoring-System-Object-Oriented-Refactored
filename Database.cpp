#include "Database.h"
#include <iostream>

Database& Database::instance()
{
    static Database db;
    return db;
}

bool Database::connect(const std::string& server,
    const std::string& username,
    const std::string& password)
{
    try
    {
        sql::Driver* driver = get_driver_instance();
        con = driver->connect(server, username, password);
        con->setSchema("project1");
        return true;
    }
    catch (sql::SQLException& e)
    {
        std::cerr << "Database connection failed: " << e.what() << std::endl;
        return false;
    }
}

sql::Connection* Database::getConnection() const
{
    return con;
}

Database::~Database()
{
    delete con;
}