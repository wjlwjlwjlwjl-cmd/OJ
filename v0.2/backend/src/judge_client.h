#ifndef JUDGE_CLIENT_H
#define JUDGE_CLIENT_H

#include <string>

class JudgeClient {
public:
    static void init(const std::string& host, int port);
    static void setSyncMode(bool sync);
    static void submitJudge(int submission_id, int problem_id,
                             const std::string& code,
                             int time_limit_ms, int memory_limit_mb,
                             const std::string& test_cases_json);

private:
    static std::string s_host;
    static int s_port;
    static bool s_sync_mode;
    static void doJudge(int submission_id, int problem_id,
                         const std::string& code,
                         int time_limit_ms, int memory_limit_mb,
                         const std::string& test_cases_json);
};

#endif
