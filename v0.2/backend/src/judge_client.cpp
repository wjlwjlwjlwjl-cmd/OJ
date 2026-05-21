#include "judge_client.h"
#include "models/submission.h"
#include "utils/logger.h"
#include "httplib.h"
#include <json.hpp>
#include <thread>

using json = nlohmann::json;

std::string JudgeClient::s_host = "127.0.0.1";
int JudgeClient::s_port = 9090;
bool JudgeClient::s_sync_mode = false;

void JudgeClient::init(const std::string& host, int port) {
    s_host = host;
    s_port = port;
}

void JudgeClient::setSyncMode(bool sync) {
    s_sync_mode = sync;
}

void JudgeClient::doJudge(int submission_id, int problem_id,
                           const std::string& code,
                           int time_limit_ms, int memory_limit_mb,
                           const std::string& test_cases_json) {
    SubmissionModel::updateJudging(submission_id);
    Logger::info("Judging submission " + std::to_string(submission_id));

    httplib::Client client("http://" + s_host + ":" + std::to_string(s_port));

    json body;
    body["code"] = code;
    body["time_limit_ms"] = time_limit_ms;
    body["memory_limit_mb"] = memory_limit_mb;
    body["test_cases_str"] = test_cases_json;

    auto res = client.Post("/judge", body.dump(), "application/json");
    if (!res) {
        Logger::error("Judge service request failed for submission "
                      + std::to_string(submission_id));
        SubmissionModel::updateStatus(submission_id, "runtime_error", 0,
                                       R"({"error":"Judge service unreachable"})");
        return;
    }

    json result;
    try {
        result = json::parse(res->body);
    } catch (...) {
        Logger::error("Invalid judge response for submission "
                      + std::to_string(submission_id));
        SubmissionModel::updateStatus(submission_id, "runtime_error", 0,
                                       R"({"error":"Invalid judge response"})");
        return;
    }

    std::string status = result.value("status", "runtime_error");
    int score = 0;
    if (result.contains("test_results") && result["test_results"].is_array()) {
        auto& results = result["test_results"];
        if (!results.empty()) {
            int passed = 0;
            for (const auto& tr : results) {
                if (tr.value("passed", false)) passed++;
            }
            score = passed * 100 / (int)results.size();
        }
    }

    std::string judgeResultStr = result.dump();
    SubmissionModel::updateStatus(submission_id, status, score, judgeResultStr);
    Logger::info("Submission " + std::to_string(submission_id) + " done: " + status);
}

void JudgeClient::submitJudge(int submission_id, int problem_id,
                               const std::string& code,
                               int time_limit_ms, int memory_limit_mb,
                               const std::string& test_cases_json) {
    if (s_sync_mode) {
        doJudge(submission_id, problem_id, code, time_limit_ms, memory_limit_mb,
                test_cases_json);
    } else {
        std::thread([submission_id, problem_id, code, time_limit_ms, memory_limit_mb, test_cases_json]() {
            doJudge(submission_id, problem_id, code, time_limit_ms, memory_limit_mb,
                    test_cases_json);
        }).detach();
    }
}
