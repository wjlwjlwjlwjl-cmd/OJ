#include "router.h"
#include "controllers/auth_controller.h"
#include "utils/logger.h"
#include <json.hpp>

using json = nlohmann::json;

Router::Router(Server& server) : m_server(server) {}

void Router::setupRoutes() {
    Logger::info("Registering API routes...");

    m_server.get("/api/health", [](const httplib::Request&, httplib::Response& res) {
        json j;
        j["status"] = "ok";
        res.set_content(j.dump(), "application/json");
    });

    setupAuthRoutes();
    setupProblemRoutes();
    setupSubmissionRoutes();
    setupJudgeRoutes();

    Logger::info("All routes registered");
}

void Router::setupAuthRoutes() {
    m_server.post("/api/auth/register", AuthController::handleRegister);
    m_server.post("/api/auth/login",    AuthController::handleLogin);
    m_server.post("/api/auth/logout",   AuthController::handleLogout);
    m_server.get("/api/auth/me",        AuthController::handleMe);
    Logger::info("  Auth routes registered");
}

void Router::setupProblemRoutes() {
    Logger::info("  Problem routes (pending)");
}

void Router::setupSubmissionRoutes() {
    Logger::info("  Submission routes (pending)");
}

void Router::setupJudgeRoutes() {
    Logger::info("  Judge routes (pending)");
}
