#include "connection.h"
#include "../utils/logger.h"

MYSQL* DbConnection::s_mysql = nullptr;

bool DbConnection::init(const DbConfig& config) {
    if (s_mysql) {
        close();
    }

    s_mysql = mysql_init(nullptr);
    if (!s_mysql) {
        Logger::error("Failed to initialize MySQL");
        return false;
    }

    unsigned int timeout = 5;
    mysql_options(s_mysql, MYSQL_OPT_CONNECT_TIMEOUT, &timeout);

    if (!mysql_real_connect(
            s_mysql,
            config.host.c_str(),
            config.user.c_str(),
            config.password.c_str(),
            config.database.c_str(),
            config.port,
            nullptr, 0)) {
        Logger::error("MySQL connection failed: " + std::string(mysql_error(s_mysql)));
        mysql_close(s_mysql);
        s_mysql = nullptr;
        return false;
    }

    mysql_set_character_set(s_mysql, "utf8mb4");
    Logger::info("MySQL connected to " + config.host + ":" + std::to_string(config.port));
    return true;
}

void DbConnection::close() {
    if (s_mysql) {
        mysql_close(s_mysql);
        s_mysql = nullptr;
        Logger::info("MySQL connection closed");
    }
}

bool DbConnection::isConnected() {
    if (!s_mysql) return false;
    return mysql_ping(s_mysql) == 0;
}

MYSQL_RES* DbConnection::query(const std::string& sql) {
    if (!s_mysql) {
        Logger::error("query() called without active connection");
        return nullptr;
    }

    if (mysql_query(s_mysql, sql.c_str()) != 0) {
        Logger::error("Query failed: " + std::string(mysql_error(s_mysql)));
        Logger::error("SQL: " + sql);
        return nullptr;
    }

    return mysql_store_result(s_mysql);
}

bool DbConnection::execute(const std::string& sql) {
    if (!s_mysql) {
        Logger::error("execute() called without active connection");
        return false;
    }

    if (mysql_query(s_mysql, sql.c_str()) != 0) {
        Logger::error("Execute failed: " + std::string(mysql_error(s_mysql)));
        Logger::error("SQL: " + sql);
        return false;
    }

    return true;
}

MYSQL* DbConnection::handle() {
    return s_mysql;
}

std::string DbConnection::escape(const std::string& str) {
    if (!s_mysql) return str;

    std::string result(str.size() * 2 + 1, '\0');
    unsigned long len = mysql_real_escape_string(
        s_mysql, result.data(), str.c_str(), str.size());
    result.resize(len);
    return result;
}
