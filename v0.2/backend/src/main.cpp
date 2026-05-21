#include <iostream>
#include <csignal>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <vector>

#include "server.h"
#include "router.h"
#include "middleware/session.h"
#include "utils/logger.h"
#include "db/connection.h"
#include <json.hpp>

using json = nlohmann::json;

static Server g_server;

static json loadConfig(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return nullptr;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return json::parse(buffer.str());
}

static json findConfig() {
    const char* env = std::getenv("CONFIG_PATH");
    if (env) {
        auto cfg = loadConfig(env);
        if (!cfg.is_null()) {
            Logger::info("Loaded config from: " + std::string(env));
            return cfg;
        }
    }

    std::vector<std::string> candidates = {
        "../config/config.json",
        "../../config/config.json",
        "./config/config.json",
        "/home/wang/oj/v0.2/config/config.json",
    };

    for (const auto& path : candidates) {
        auto cfg = loadConfig(path);
        if (!cfg.is_null()) {
            Logger::info("Loaded config from: " + path);
            return cfg;
        }
    }

    Logger::error("Cannot find config.json. Set CONFIG_PATH env or place in candidates.");
    exit(1);
}

static DbConfig parseDbConfig(const json& j) {
    DbConfig cfg;
    cfg.host     = j["host"];
    cfg.port     = j.value("port", 3306);
    cfg.user     = j["user"];
    cfg.password = j["password"];
    cfg.database = j["database"];
    return cfg;
}

static void signalHandler(int sig) {
    Logger::info("Received signal " + std::to_string(sig) + ", shutting down...");
    DbConnection::close();
    g_server.stop();
    exit(0);
}

int main() {
    Logger::init(DEBUG);
    Logger::info("OJ Backend starting...");

    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    auto config = findConfig();
    auto& dbCfg = config["db"];
    auto& srvCfg = config["server"];

    if (!DbConnection::init(parseDbConfig(dbCfg))) {
        Logger::error("Database initialization failed, exiting");
        return 1;
    }

    int sessionExpiry = config["session"].value("expiry_hours", 24);
    SessionMiddleware::init(sessionExpiry);

    Router router(g_server);
    router.setupRoutes();

    std::string host = srvCfg.value("host", "0.0.0.0");
    int port = srvCfg.value("port", 8080);

    if (!g_server.listen(host, port)) {
        Logger::error("Failed to start server on " + host + ":" + std::to_string(port));
        DbConnection::close();
        return 1;
    }

    return 0;
}
