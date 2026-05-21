#ifndef RUNNER_H
#define RUNNER_H

#include <string>
#include <vector>

struct TestCase {
    std::string input;
    std::string output;
};

struct TestResult {
    int index = 0;
    bool passed = false;
    std::string your_output;
    std::string expected_output;
    std::string status;
};

struct JudgeResult {
    std::string status;
    std::string message;
    std::vector<TestResult> test_results;
};

class Runner {
public:
    static JudgeResult judge(const std::string& code,
                              const std::vector<TestCase>& test_cases,
                              int time_limit_ms,
                              int memory_limit_mb);

private:
    static bool compile(const std::string& code,
                        const std::string& source_path,
                        const std::string& exe_path,
                        std::string& error_output);
    static TestResult runSingle(const std::string& exe_path,
                                 const TestCase& tc,
                                 int index,
                                 int time_limit_ms,
                                 int memory_limit_mb);
    static std::string trim(const std::string& s);
    static std::string createTempDir();
    static void cleanup(const std::string& dir);
};

#endif
