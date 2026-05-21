#include "server.h"
#include "utils/logger.h"

Server::Server() : m_running(false) {
    m_server.set_pre_routing_handler(
        [this](const httplib::Request& req, httplib::Response& res) {
            corsMiddleware(req, res);
            if (req.method == "OPTIONS") {
                return httplib::Server::HandlerResponse::Handled;
            }
            return httplib::Server::HandlerResponse::Unhandled;
        });

    m_server.set_exception_handler(
        [](const httplib::Request&, httplib::Response& res, std::exception_ptr ep) {
            try {
                std::rethrow_exception(ep);
            } catch (const std::exception& e) {
                Logger::error("Unhandled exception: " + std::string(e.what()));
            }
            res.status = 500;
            res.set_content(R"({"error":"Internal Server Error"})", "application/json");
        });
}

Server::~Server() {
    stop();
}

bool Server::listen(const std::string& host, int port) {
    Logger::info("Server starting on " + host + ":" + std::to_string(port));
    m_running = true;
    return m_server.listen(host.c_str(), port);
}

void Server::stop() {
    if (m_running) {
        m_server.stop();
        m_running = false;
        Logger::info("Server stopped");
    }
}

httplib::Server& Server::handle() {
    return m_server;
}

void Server::get(const std::string& pattern, Handler handler) {
    m_server.Get(pattern.c_str(), handler);
}

void Server::post(const std::string& pattern, Handler handler) {
    m_server.Post(pattern.c_str(), handler);
}

void Server::put(const std::string& pattern, Handler handler) {
    m_server.Put(pattern.c_str(), handler);
}

void Server::del(const std::string& pattern, Handler handler) {
    m_server.Delete(pattern.c_str(), handler);
}

void Server::setStaticDir(const std::string& mount, const std::string& dir) {
    m_server.set_mount_point(mount.c_str(), dir.c_str());
    Logger::info("Static dir mounted: " + mount + " -> " + dir);
}

void Server::corsMiddleware(const httplib::Request&, httplib::Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization, Cookie");
    res.set_header("Access-Control-Allow-Credentials", "true");
}
