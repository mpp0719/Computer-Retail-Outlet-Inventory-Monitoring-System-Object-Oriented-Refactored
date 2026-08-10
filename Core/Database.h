#pragma once
#include <mysql_connection.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/statement.h>
#include <string>

class Database
{
public:
    static Database& instance();

    bool connect(const std::string& server,
        const std::string& username,
        const std::string& password);

    sql::Connection* getConnection() const;

    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;
    ~Database();

private:
    Database() = default;
    sql::Connection* con = nullptr;
};