#include "router.h"
#include "controllers/auth_controller.h"
#include "controllers/problem_controller.h"
#include "controllers/submission_controller.h"
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
    m_server.get("/api/problems",           ProblemController::handleListProblems);
    m_server.get("/api/problems/:id",       ProblemController::handleGetProblem);
    m_server.post("/api/problems",          ProblemController::handleCreateProblem);
    m_server.put("/api/problems/:id",       ProblemController::handleUpdateProblem);
    m_server.del("/api/problems/:id",       ProblemController::handleDeleteProblem);
    Logger::info("  Problem routes registered");
}

void Router::setupSubmissionRoutes() {
    m_server.post("/api/submit",             SubmissionController::handleSubmit);
    m_server.get("/api/submission/:id",      SubmissionController::handleGetSubmission);
    m_server.get("/api/submissions",         SubmissionController::handleListSubmissions);
    Logger::info("  Submission routes registered");
}

void Router::setupJudgeRoutes() {
    Logger::info("  Judge routes (internal, via JudgeClient)");
}
