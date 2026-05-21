#ifndef USER_MODEL_H
#define USER_MODEL_H

#include <string>
#include <optional>

struct User {
    int id;
    std::string username;
    std::string password;
    std::string role;
    std::string created_at;
    std::string updated_at;
};

class UserModel {
public:
    static std::optional<User> findById(int id);
    static std::optional<User> findByUsername(const std::string& username);
    static std::optional<User> create(const std::string& username,
                                       const std::string& password,
                                       const std::string& role = "user");
    static bool verifyPassword(const std::string& password, const std::string& hash);
};

#endif
