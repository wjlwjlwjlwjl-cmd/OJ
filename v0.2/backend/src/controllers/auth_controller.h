#ifndef AUTH_CONTROLLER_H
#define AUTH_CONTROLLER_H

#include "httplib.h"

class AuthController {
public:
    static void handleRegister(const httplib::Request& req, httplib::Response& res);
    static void handleLogin(const httplib::Request& req, httplib::Response& res);
    static void handleLogout(const httplib::Request& req, httplib::Response& res);
    static void handleMe(const httplib::Request& req, httplib::Response& res);
};

#endif
