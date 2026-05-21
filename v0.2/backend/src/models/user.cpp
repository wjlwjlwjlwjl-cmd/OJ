#include "user.h"
#include "../db/connection.h"
#include "../utils/logger.h"

#include <cstring>
#include <sstream>
#include <iomanip>
#include <random>
#include <openssl/evp.h>

static std::string hexEncode(const unsigned char* data, unsigned int len) {
    std::ostringstream oss;
    for (unsigned int i = 0; i < len; ++i) {
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)data[i];
    }
    return oss.str();
}

static std::string generateSalt() {
    unsigned char salt[16];
    std::random_device rd;
    for (int i = 0; i < 16; ++i) {
        salt[i] = static_cast<unsigned char>(rd());
    }
    return hexEncode(salt, 16);
}

static std::string sha256(const std::string& data) {
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hashLen = 0;

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr);
    EVP_DigestUpdate(ctx, data.data(), data.size());
    EVP_DigestFinal_ex(ctx, hash, &hashLen);
    EVP_MD_CTX_free(ctx);

    return hexEncode(hash, hashLen);
}

static std::string hashPassword(const std::string& salt, const std::string& password) {
    return sha256(salt + password);
}

std::optional<User> UserModel::findById(int id) {
    std::string sql = "SELECT id, username, password, role, created_at, updated_at "
                       "FROM users WHERE id = " + std::to_string(id);
    MYSQL_RES* res = DbConnection::query(sql);
    if (!res) return std::nullopt;

    MYSQL_ROW row = mysql_fetch_row(res);
    if (!row) {
        mysql_free_result(res);
        return std::nullopt;
    }

    User user;
    user.id         = std::stoi(row[0]);
    user.username   = row[1] ? row[1] : "";
    user.password   = row[2] ? row[2] : "";
    user.role       = row[3] ? row[3] : "user";
    user.created_at = row[4] ? row[4] : "";
    user.updated_at = row[5] ? row[5] : "";
    mysql_free_result(res);
    return user;
}

std::optional<User> UserModel::findByUsername(const std::string& username) {
    std::string escaped = DbConnection::escape(username);
    std::string sql = "SELECT id, username, password, role, created_at, updated_at "
                       "FROM users WHERE username = '" + escaped + "'";
    MYSQL_RES* res = DbConnection::query(sql);
    if (!res) return std::nullopt;

    MYSQL_ROW row = mysql_fetch_row(res);
    if (!row) {
        mysql_free_result(res);
        return std::nullopt;
    }

    User user;
    user.id         = std::stoi(row[0]);
    user.username   = row[1] ? row[1] : "";
    user.password   = row[2] ? row[2] : "";
    user.role       = row[3] ? row[3] : "user";
    user.created_at = row[4] ? row[4] : "";
    user.updated_at = row[5] ? row[5] : "";
    mysql_free_result(res);
    return user;
}

std::optional<User> UserModel::create(const std::string& username,
                                       const std::string& password,
                                       const std::string& role) {
    std::string escapedUser = DbConnection::escape(username);
    std::string escapedRole = DbConnection::escape(role);

    std::string salt = generateSalt();
    std::string hash = hashPassword(salt, password);
    std::string stored = salt + "$" + hash;

    std::string sql = "INSERT INTO users (username, password, role) VALUES ('"
                      + escapedUser + "', '" + stored + "', '" + escapedRole + "')";

    if (!DbConnection::execute(sql)) {
        return std::nullopt;
    }

    int newId = mysql_insert_id(DbConnection::handle());
    return findById(newId);
}

bool UserModel::verifyPassword(const std::string& password, const std::string& stored) {
    size_t sep = stored.find('$');
    if (sep == std::string::npos) {
        return sha256(password) == stored;
    }

    std::string salt = stored.substr(0, sep);
    std::string hash = hashPassword(salt, password);
    return (salt + "$" + hash) == stored;
}
