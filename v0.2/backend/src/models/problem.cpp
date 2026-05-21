#include "problem.h"
#include "../db/connection.h"
#include "../utils/logger.h"

std::vector<Problem> ProblemModel::findAll() {
    std::vector<Problem> problems;
    std::string sql = "SELECT id, title, description, difficulty, time_limit, "
                       "memory_limit, created_by, created_at, updated_at "
                       "FROM problems ORDER BY id";
    MYSQL_RES* res = DbConnection::query(sql);
    if (!res) return problems;

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res))) {
        Problem p;
        p.id = std::stoi(row[0]);
        p.title = row[1] ? row[1] : "";
        p.description = row[2] ? row[2] : "";
        p.difficulty = row[3] ? row[3] : "easy";
        p.time_limit = row[4] ? std::stoi(row[4]) : 1000;
        p.memory_limit = row[5] ? std::stoi(row[5]) : 256;
        p.created_by = row[6] ? std::stoi(row[6]) : 0;
        p.created_at = row[7] ? row[7] : "";
        p.updated_at = row[8] ? row[8] : "";
        problems.push_back(p);
    }
    mysql_free_result(res);
    return problems;
}

std::optional<Problem> ProblemModel::findById(int id) {
    std::string sql = "SELECT id, title, description, difficulty, time_limit, "
                       "memory_limit, test_cases, created_by, created_at, updated_at "
                       "FROM problems WHERE id = " + std::to_string(id);
    MYSQL_RES* res = DbConnection::query(sql);
    if (!res) return std::nullopt;

    MYSQL_ROW row = mysql_fetch_row(res);
    if (!row) {
        mysql_free_result(res);
        return std::nullopt;
    }

    unsigned long* lengths = mysql_fetch_lengths(res);

    Problem p;
    p.id = std::stoi(row[0]);
    p.title = row[1] ? row[1] : "";
    p.description = row[2] ? row[2] : "";
    p.difficulty = row[3] ? row[3] : "easy";
    p.time_limit = row[4] ? std::stoi(row[4]) : 1000;
    p.memory_limit = row[5] ? std::stoi(row[5]) : 256;
    if (row[6] && lengths[6] > 0) {
        p.test_cases = std::string(row[6], lengths[6]);
    }
    p.created_by = row[7] ? std::stoi(row[7]) : 0;
    p.created_at = row[8] ? row[8] : "";
    p.updated_at = row[9] ? row[9] : "";
    mysql_free_result(res);
    return p;
}

std::optional<Problem> ProblemModel::create(const std::string& title,
                                              const std::string& description,
                                              const std::string& difficulty,
                                              int time_limit,
                                              int memory_limit,
                                              const std::string& test_cases,
                                              int created_by) {
    std::string escapedTitle = DbConnection::escape(title);
    std::string escapedDesc = DbConnection::escape(description);
    std::string escapedDiff = DbConnection::escape(difficulty);
    std::string escapedTC = DbConnection::escape(test_cases);

    std::string sql = "INSERT INTO problems (title, description, difficulty, "
                       "time_limit, memory_limit, test_cases, created_by) VALUES ('"
                       + escapedTitle + "', '" + escapedDesc + "', '" + escapedDiff
                       + "', " + std::to_string(time_limit) + ", "
                       + std::to_string(memory_limit) + ", '" + escapedTC
                       + "', " + std::to_string(created_by) + ")";

    if (!DbConnection::execute(sql)) {
        return std::nullopt;
    }

    int newId = mysql_insert_id(DbConnection::handle());
    return findById(newId);
}

bool ProblemModel::update(int id, const std::string& title,
                           const std::string& description,
                           const std::string& difficulty,
                           int time_limit,
                           int memory_limit,
                           const std::string& test_cases) {
    std::string escapedTitle = DbConnection::escape(title);
    std::string escapedDesc = DbConnection::escape(description);
    std::string escapedDiff = DbConnection::escape(difficulty);
    std::string escapedTC = DbConnection::escape(test_cases);

    std::string sql = "UPDATE problems SET title='" + escapedTitle
                       + "', description='" + escapedDesc
                       + "', difficulty='" + escapedDiff
                       + "', time_limit=" + std::to_string(time_limit)
                       + ", memory_limit=" + std::to_string(memory_limit)
                       + ", test_cases='" + escapedTC
                       + "' WHERE id=" + std::to_string(id);

    return DbConnection::execute(sql);
}

bool ProblemModel::remove(int id) {
    std::string sql = "DELETE FROM problems WHERE id=" + std::to_string(id);
    return DbConnection::execute(sql);
}
