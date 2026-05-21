#include "submission.h"
#include "../db/connection.h"
#include "../utils/logger.h"

std::optional<Submission> SubmissionModel::create(int user_id, int problem_id,
                                                    const std::string& code,
                                                    const std::string& language) {
    std::string escapedCode = DbConnection::escape(code);
    std::string escapedLang = DbConnection::escape(language);

    std::string sql = "INSERT INTO submissions (user_id, problem_id, code, language, status) "
                       "VALUES (" + std::to_string(user_id) + ", "
                       + std::to_string(problem_id) + ", '"
                       + escapedCode + "', '" + escapedLang + "', 'pending')";

    if (!DbConnection::execute(sql)) {
        Logger::error("Failed to create submission");
        return std::nullopt;
    }

    int newId = mysql_insert_id(DbConnection::handle());
    return findById(newId);
}

std::optional<Submission> SubmissionModel::findById(int id) {
    std::string sql = "SELECT id, user_id, problem_id, code, language, status, "
                       "score, judge_result, submitted_at, judged_at "
                       "FROM submissions WHERE id = " + std::to_string(id);
    MYSQL_RES* res = DbConnection::query(sql);
    if (!res) return std::nullopt;

    MYSQL_ROW row = mysql_fetch_row(res);
    if (!row) {
        mysql_free_result(res);
        return std::nullopt;
    }

    unsigned long* lengths = mysql_fetch_lengths(res);

    Submission s;
    s.id = std::stoi(row[0]);
    s.user_id = row[1] ? std::stoi(row[1]) : 0;
    s.problem_id = row[2] ? std::stoi(row[2]) : 0;
    s.code = row[3] ? row[3] : "";
    s.language = row[4] ? row[4] : "cpp";
    s.status = row[5] ? row[5] : "pending";
    s.score = row[6] ? std::stoi(row[6]) : 0;
    if (row[7] && lengths[7] > 0) {
        s.judge_result = std::string(row[7], lengths[7]);
    }
    s.submitted_at = row[8] ? row[8] : "";
    s.judged_at = row[9] ? row[9] : "";
    mysql_free_result(res);
    return s;
}

std::vector<Submission> SubmissionModel::findByUserId(int user_id, int limit, int offset) {
    std::vector<Submission> submissions;
    std::string sql = "SELECT id, user_id, problem_id, code, language, status, "
                       "score, judge_result, submitted_at, judged_at "
                       "FROM submissions WHERE user_id = " + std::to_string(user_id)
                       + " ORDER BY submitted_at DESC LIMIT " + std::to_string(limit)
                       + " OFFSET " + std::to_string(offset);
    MYSQL_RES* res = DbConnection::query(sql);
    if (!res) return submissions;

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res))) {
        Submission s;
        s.id = std::stoi(row[0]);
        s.user_id = std::stoi(row[1]);
        s.problem_id = std::stoi(row[2]);
        s.code = row[3] ? row[3] : "";
        s.language = row[4] ? row[4] : "cpp";
        s.status = row[5] ? row[5] : "pending";
        s.score = row[6] ? std::stoi(row[6]) : 0;
        s.submitted_at = row[8] ? row[8] : "";
        s.judged_at = row[9] ? row[9] : "";
        submissions.push_back(s);
    }
    mysql_free_result(res);
    return submissions;
}

std::vector<Submission> SubmissionModel::findByProblemId(int problem_id, int limit, int offset) {
    std::vector<Submission> submissions;
    std::string sql = "SELECT id, user_id, problem_id, code, language, status, "
                       "score, judge_result, submitted_at, judged_at "
                       "FROM submissions WHERE problem_id = " + std::to_string(problem_id)
                       + " ORDER BY submitted_at DESC LIMIT " + std::to_string(limit)
                       + " OFFSET " + std::to_string(offset);
    MYSQL_RES* res = DbConnection::query(sql);
    if (!res) return submissions;

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res))) {
        Submission s;
        s.id = std::stoi(row[0]);
        s.user_id = std::stoi(row[1]);
        s.problem_id = std::stoi(row[2]);
        s.code = row[3] ? row[3] : "";
        s.language = row[4] ? row[4] : "cpp";
        s.status = row[5] ? row[5] : "pending";
        s.score = row[6] ? std::stoi(row[6]) : 0;
        s.submitted_at = row[8] ? row[8] : "";
        s.judged_at = row[9] ? row[9] : "";
        submissions.push_back(s);
    }
    mysql_free_result(res);
    return submissions;
}

int SubmissionModel::countByUserId(int user_id) {
    std::string sql = "SELECT COUNT(*) FROM submissions WHERE user_id = "
                       + std::to_string(user_id);
    MYSQL_RES* res = DbConnection::query(sql);
    if (!res) return 0;
    MYSQL_ROW row = mysql_fetch_row(res);
    int count = row ? std::stoi(row[0]) : 0;
    mysql_free_result(res);
    return count;
}

int SubmissionModel::countByProblemId(int problem_id) {
    std::string sql = "SELECT COUNT(*) FROM submissions WHERE problem_id = "
                       + std::to_string(problem_id);
    MYSQL_RES* res = DbConnection::query(sql);
    if (!res) return 0;
    MYSQL_ROW row = mysql_fetch_row(res);
    int count = row ? std::stoi(row[0]) : 0;
    mysql_free_result(res);
    return count;
}

bool SubmissionModel::updateStatus(int id, const std::string& status, int score,
                                    const std::string& judge_result) {
    std::string escapedStatus = DbConnection::escape(status);
    std::string escapedResult = DbConnection::escape(judge_result);

    std::string sql = "UPDATE submissions SET status='" + escapedStatus
                       + "', score=" + std::to_string(score)
                       + ", judge_result='" + escapedResult
                       + "', judged_at=NOW() WHERE id=" + std::to_string(id);
    return DbConnection::execute(sql);
}

bool SubmissionModel::updateJudging(int id) {
    std::string sql = "UPDATE submissions SET status='judging' WHERE id="
                       + std::to_string(id);
    return DbConnection::execute(sql);
}
