#ifndef DB_CONNECTION_H
#define DB_CONNECTION_H

#include <string>
#include <mysql/mysql.h>

struct DbConfig {
    std::string host;
    int port;
    std::string user;
    std::string password;
    std::string database;
};

class DbConnection {
public:
    static bool init(const DbConfig& config);
    static void close();
    static bool isConnected();

    static MYSQL_RES* query(const std::string& sql);
    static bool execute(const std::string& sql);
    static MYSQL* handle();

    static std::string escape(const std::string& str);

private:
    static MYSQL* s_mysql;
};

#endif
