#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <functional>
#include "httplib.h"

class Server {
public:
    using Handler = std::function<void(const httplib::Request&, httplib::Response&)>;

    Server();
    ~Server();

    bool listen(const std::string& host, int port);
    void stop();

    httplib::Server& handle();

    void get(const std::string& pattern, Handler handler);
    void post(const std::string& pattern, Handler handler);
    void put(const std::string& pattern, Handler handler);
    void del(const std::string& pattern, Handler handler);

    void setStaticDir(const std::string& mount, const std::string& dir);

private:
    httplib::Server m_server;
    bool m_running;

    void corsMiddleware(const httplib::Request& req, httplib::Response& res);
};

#endif
