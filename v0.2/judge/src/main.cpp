#include <iostream>
#include <csignal>
#include <cstdlib>
#include "judge_server.h"
#include <json.hpp>

using json = nlohmann::json;

static JudgeServer g_server;

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
        if (!cfg.is_null()) return cfg;
    }

    std::vector<std::string> candidates = {
        "../config/config.json",
        "../../config/config.json",
        "./config/config.json",
        "/home/wang/oj/v0.2/config/config.json",
    };

    for (const auto& path : candidates) {
        auto cfg = loadConfig(path);
        if (!cfg.is_null()) return cfg;
    }

    return nullptr;
}

static void signalHandler(int sig) {
    std::cout << "[Judge] Received signal " << sig << ", shutting down..." << std::endl;
    g_server.stop();
    exit(0);
}

int main() {
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    auto config = findConfig();
    std::string host = "127.0.0.1";
    int port = 9090;

    if (!config.is_null() && config.contains("judge")) {
        host = config["judge"].value("host", host);
        port = config["judge"].value("port", port);
    }

    if (!g_server.listen(host, port)) {
        std::cerr << "[Judge] Failed to start server on " << host << ":" << port << std::endl;
        return 1;
    }

    return 0;
}
