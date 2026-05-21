#include "problem_controller.h"
#include "../models/problem.h"
#include "../models/user.h"
#include "../middleware/session.h"
#include "../utils/logger.h"
#include <json.hpp>

using json = nlohmann::json;

static json problemToJson(const Problem& p) {
    json j;
    j["id"] = p.id;
    j["title"] = p.title;
    j["description"] = p.description;
    j["difficulty"] = p.difficulty;
    j["time_limit"] = p.time_limit;
    j["memory_limit"] = p.memory_limit;
    j["created_by"] = p.created_by;
    j["created_at"] = p.created_at;
    j["updated_at"] = p.updated_at;
    return j;
}

static json problemToJsonFull(const Problem& p) {
    json j = problemToJson(p);
    j["test_cases"] = p.test_cases;
    return j;
}

static bool requireAuth(const httplib::Request& req, httplib::Response& res,
                         Session& session) {
    std::string cookie = req.get_header_value("Cookie");
    auto s = SessionMiddleware::authenticate(cookie);
    if (!s.has_value()) {
        res.status = 401;
        res.set_content(R"({"error":"Not authenticated"})", "application/json");
        return false;
    }
    session = s.value();
    return true;
}

static bool requireAdmin(const httplib::Request& req, httplib::Response& res,
                          Session& session) {
    if (!requireAuth(req, res, session)) return false;
    if (session.role != "admin") {
        res.status = 403;
        res.set_content(R"({"error":"Admin access required"})", "application/json");
        return false;
    }
    return true;
}

void ProblemController::handleListProblems(const httplib::Request& req,
                                            httplib::Response& res) {
    Session session;
    if (!requireAuth(req, res, session)) return;

    auto problems = ProblemModel::findAll();
    json j = json::array();
    for (const auto& p : problems) {
        j.push_back(problemToJson(p));
    }
    res.set_content(j.dump(), "application/json");
}

void ProblemController::handleGetProblem(const httplib::Request& req,
                                          httplib::Response& res) {
    Session session;
    if (!requireAuth(req, res, session)) return;

    auto it = req.path_params.find("id");
    if (it == req.path_params.end()) {
        res.status = 400;
        res.set_content(R"({"error":"Missing problem id"})", "application/json");
        return;
    }

    int id;
    try {
        id = std::stoi(it->second);
    } catch (...) {
        res.status = 400;
        res.set_content(R"({"error":"Invalid problem id"})", "application/json");
        return;
    }

    auto problem = ProblemModel::findById(id);
    if (!problem.has_value()) {
        res.status = 404;
        res.set_content(R"({"error":"Problem not found"})", "application/json");
        return;
    }

    json j = problemToJsonFull(problem.value());
    res.set_content(j.dump(), "application/json");
}

void ProblemController::handleCreateProblem(const httplib::Request& req,
                                             httplib::Response& res) {
    Session session;
    if (!requireAdmin(req, res, session)) return;

    json body;
    try {
        body = json::parse(req.body);
    } catch (...) {
        res.status = 400;
        res.set_content(R"({"error":"Invalid JSON"})", "application/json");
        return;
    }

    std::string title = body.value("title", "");
    std::string description = body.value("description", "");
    std::string difficulty = body.value("difficulty", "easy");
    int time_limit = body.value("time_limit", 1000);
    int memory_limit = body.value("memory_limit", 256);
    std::string test_cases = body.value("test_cases", "[]");

    if (title.empty()) {
        res.status = 400;
        res.set_content(R"({"error":"Title is required"})", "application/json");
        return;
    }

    if (difficulty != "easy" && difficulty != "medium" && difficulty != "hard") {
        difficulty = "easy";
    }

    auto problem = ProblemModel::create(title, description, difficulty,
                                         time_limit, memory_limit, test_cases,
                                         session.user_id);
    if (!problem.has_value()) {
        res.status = 500;
        res.set_content(R"({"error":"Failed to create problem"})", "application/json");
        return;
    }

    res.status = 201;
    json j = problemToJsonFull(problem.value());
    res.set_content(j.dump(), "application/json");

    Logger::info("Problem created: " + title + " by user " + std::to_string(session.user_id));
}

void ProblemController::handleUpdateProblem(const httplib::Request& req,
                                             httplib::Response& res) {
    Session session;
    if (!requireAdmin(req, res, session)) return;

    auto it = req.path_params.find("id");
    if (it == req.path_params.end()) {
        res.status = 400;
        res.set_content(R"({"error":"Missing problem id"})", "application/json");
        return;
    }

    int id;
    try {
        id = std::stoi(it->second);
    } catch (...) {
        res.status = 400;
        res.set_content(R"({"error":"Invalid problem id"})", "application/json");
        return;
    }

    json body;
    try {
        body = json::parse(req.body);
    } catch (...) {
        res.status = 400;
        res.set_content(R"({"error":"Invalid JSON"})", "application/json");
        return;
    }

    auto existing = ProblemModel::findById(id);
    if (!existing.has_value()) {
        res.status = 404;
        res.set_content(R"({"error":"Problem not found"})", "application/json");
        return;
    }

    std::string title = body.value("title", existing->title);
    std::string description = body.value("description", existing->description);
    std::string difficulty = body.value("difficulty", existing->difficulty);
    int time_limit = body.value("time_limit", existing->time_limit);
    int memory_limit = body.value("memory_limit", existing->memory_limit);
    std::string test_cases = body.value("test_cases", existing->test_cases);

    if (difficulty != "easy" && difficulty != "medium" && difficulty != "hard") {
        difficulty = existing->difficulty;
    }

    if (!ProblemModel::update(id, title, description, difficulty,
                               time_limit, memory_limit, test_cases)) {
        res.status = 500;
        res.set_content(R"({"error":"Failed to update problem"})", "application/json");
        return;
    }

    auto updated = ProblemModel::findById(id);
    json j = problemToJsonFull(updated.value());
    res.set_content(j.dump(), "application/json");

    Logger::info("Problem updated: " + title);
}

void ProblemController::handleDeleteProblem(const httplib::Request& req,
                                             httplib::Response& res) {
    Session session;
    if (!requireAdmin(req, res, session)) return;

    auto it = req.path_params.find("id");
    if (it == req.path_params.end()) {
        res.status = 400;
        res.set_content(R"({"error":"Missing problem id"})", "application/json");
        return;
    }

    int id;
    try {
        id = std::stoi(it->second);
    } catch (...) {
        res.status = 400;
        res.set_content(R"({"error":"Invalid problem id"})", "application/json");
        return;
    }

    auto existing = ProblemModel::findById(id);
    if (!existing.has_value()) {
        res.status = 404;
        res.set_content(R"({"error":"Problem not found"})", "application/json");
        return;
    }

    if (!ProblemModel::remove(id)) {
        res.status = 500;
        res.set_content(R"({"error":"Failed to delete problem"})", "application/json");
        return;
    }

    json j;
    j["message"] = "Problem deleted";
    res.set_content(j.dump(), "application/json");

    Logger::info("Problem deleted: " + existing->title);
}
