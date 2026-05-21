#ifndef SUBMISSION_MODEL_H
#define SUBMISSION_MODEL_H

#include <string>
#include <optional>
#include <vector>

struct Submission {
    int id;
    int user_id;
    int problem_id;
    std::string code;
    std::string language;
    std::string status;
    int score;
    std::string judge_result;
    std::string submitted_at;
    std::string judged_at;
};

class SubmissionModel {
public:
    static std::optional<Submission> create(int user_id, int problem_id,
                                              const std::string& code,
                                              const std::string& language);
    static std::optional<Submission> findById(int id);
    static std::vector<Submission> findByUserId(int user_id, int limit = 20, int offset = 0);
    static std::vector<Submission> findByProblemId(int problem_id, int limit = 20, int offset = 0);
    static int countByUserId(int user_id);
    static int countByProblemId(int problem_id);
    static bool updateStatus(int id, const std::string& status, int score,
                              const std::string& judge_result);
    static bool updateJudging(int id);
};

#endif
