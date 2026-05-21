#include "submission_controller.h"
#include "../models/submission.h"
#include "../models/problem.h"
#include "../middleware/session.h"
#include "../judge_client.h"
#include "../utils/logger.h"
#include <json.hpp>

using json = nlohmann::json;

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

static json submissionToJson(const Submission& s) {
    json j;
    j["id"] = s.id;
    j["user_id"] = s.user_id;
    j["problem_id"] = s.problem_id;
    j["language"] = s.language;
    j["status"] = s.status;
    j["score"] = s.score;
    j["submitted_at"] = s.submitted_at;
    j["judged_at"] = s.judged_at;
    if (!s.judge_result.empty()) {
        try {
            j["judge_result"] = json::parse(s.judge_result);
        } catch (...) {
            j["judge_result"] = s.judge_result;
        }
    }
    return j;
}

void SubmissionController::handleSubmit(const httplib::Request& req,
                                         httplib::Response& res) {
    Session session;
    if (!requireAuth(req, res, session)) return;

    json body;
    try {
        body = json::parse(req.body);
    } catch (...) {
        res.status = 400;
        res.set_content(R"({"error":"Invalid JSON"})", "application/json");
        return;
    }

    int problem_id = body.value("problem_id", 0);
    std::string code = body.value("code", "");
    std::string language = body.value("language", "cpp");

    if (problem_id <= 0) {
        res.status = 400;
        res.set_content(R"({"error":"Invalid problem_id"})", "application/json");
        return;
    }
    if (code.empty()) {
        res.status = 400;
        res.set_content(R"({"error":"Code is required"})", "application/json");
        return;
    }

    auto problem = ProblemModel::findById(problem_id);
    if (!problem.has_value()) {
        res.status = 404;
        res.set_content(R"({"error":"Problem not found"})", "application/json");
        return;
    }

    auto submission = SubmissionModel::create(session.user_id, problem_id,
                                               code, language);
    if (!submission.has_value()) {
        res.status = 500;
        res.set_content(R"({"error":"Failed to create submission"})",
                        "application/json");
        return;
    }

    JudgeClient::submitJudge(submission->id, problem_id, code,
                              problem->time_limit, problem->memory_limit,
                              problem->test_cases);

    res.status = 202;
    json j;
    j["id"] = submission->id;
    j["status"] = "pending";
    res.set_content(j.dump(), "application/json");
}

void SubmissionController::handleGetSubmission(const httplib::Request& req,
                                                httplib::Response& res) {
    Session session;
    if (!requireAuth(req, res, session)) return;

    auto it = req.path_params.find("id");
    if (it == req.path_params.end()) {
        res.status = 400;
        res.set_content(R"({"error":"Missing submission id"})", "application/json");
        return;
    }

    int id;
    try {
        id = std::stoi(it->second);
    } catch (...) {
        res.status = 400;
        res.set_content(R"({"error":"Invalid submission id"})", "application/json");
        return;
    }

    auto submission = SubmissionModel::findById(id);
    if (!submission.has_value()) {
        res.status = 404;
        res.set_content(R"({"error":"Submission not found"})", "application/json");
        return;
    }

    if (submission->user_id != session.user_id && session.role != "admin") {
        res.status = 403;
        res.set_content(R"({"error":"Access denied"})", "application/json");
        return;
    }

    json j = submissionToJson(submission.value());
    res.set_content(j.dump(), "application/json");
}

void SubmissionController::handleListSubmissions(const httplib::Request& req,
                                                  httplib::Response& res) {
    Session session;
    if (!requireAuth(req, res, session)) return;

    int limit = 20;
    int offset = 0;
    if (req.has_param("limit")) {
        try { limit = std::stoi(req.get_param_value("limit")); } catch (...) {}
    }
    if (req.has_param("offset")) {
        try { offset = std::stoi(req.get_param_value("offset")); } catch (...) {}
    }
    if (limit < 1) limit = 20;
    if (limit > 100) limit = 100;
    if (offset < 0) offset = 0;

    std::vector<Submission> submissions;
    int total = 0;

    if (req.has_param("problem_id")) {
        int problem_id;
        try {
            problem_id = std::stoi(req.get_param_value("problem_id"));
        } catch (...) {
            res.status = 400;
            res.set_content(R"({"error":"Invalid problem_id"})", "application/json");
            return;
        }
        submissions = SubmissionModel::findByProblemId(problem_id, limit, offset);
        total = SubmissionModel::countByProblemId(problem_id);
    } else {
        submissions = SubmissionModel::findByUserId(session.user_id, limit, offset);
        total = SubmissionModel::countByUserId(session.user_id);
    }

    json j;
    j["submissions"] = json::array();
    for (const auto& s : submissions) {
        j["submissions"].push_back(submissionToJson(s));
    }
    j["total"] = total;
    j["limit"] = limit;
    j["offset"] = offset;
    res.set_content(j.dump(), "application/json");
}
