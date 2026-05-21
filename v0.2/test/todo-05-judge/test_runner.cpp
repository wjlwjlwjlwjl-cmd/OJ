#include <gtest/gtest.h>
#include "../../judge/src/runner.h"

static const int DEFAULT_TIME_LIMIT_MS = 2000;
static const int DEFAULT_MEMORY_LIMIT_MB = 256;

static std::string helloCode() {
    return R"(#include <iostream>
int main() { std::cout << "Hello"; return 0; })";
}

static std::string worldCode() {
    return R"(#include <iostream>
int main() { std::cout << "World"; return 0; })";
}

static std::string compileErrorCode() {
    return R"(#include <iostream>
int main() { std::cout << "Hello" << std::endl; return 0; )";
}

static std::string sumCode() {
    return R"(#include <iostream>
int main() { int a,b; std::cin >> a >> b; std::cout << a+b; return 0; })";
}

static std::string infiniteLoopCode() {
    return "int main() { while(true) {} return 0; }";
}

static std::string sleepCode() {
    return R"(#include <unistd.h>
int main() { usleep(200000); return 0; })";
}

static std::string largeAllocCode() {
    return R"(int main() {
    int n = 100000000;
    volatile int* arr = new int[n];
    arr[0] = 42;
    return 0;
})";
}

static std::string normalAllocCode() {
    return R"(#include <cstdlib>
int main() {
    int n = 100000;
    int* arr = new int[n];
    for (int i = 0; i < n; i++) arr[i] = i;
    int sum = 0;
    for (int i = 0; i < n; i++) sum += arr[i];
    return 0;
})";
}

static std::string divByZeroCode() {
    return R"(#include <iostream>
int main() { int x = 1, y = 0; std::cout << (x / y); return 0; })";
}

static std::string segfaultCode() {
    return R"(int main() { int* p = nullptr; *p = 42; return 0; })";
}

static std::string nonZeroExitCode() {
    return "int main() { return 1; }";
}

static std::string readFileCode() {
    return R"(#include <iostream>
#include <fstream>
#include <string>
int main() {
    std::ifstream f("/etc/passwd");
    std::string s;
    if (f) { f >> s; std::cout << s; }
    return 0;
})";
}

static std::string systemCallCode() {
    return R"(#include <cstdlib>
int main() { system("echo hi"); return 0; })";
}

static std::string forkCode() {
    return R"(#include <unistd.h>
#include <sys/wait.h>
int main() {
    pid_t pid = fork();
    if (pid == 0) _exit(0);
    if (pid > 0) wait(nullptr);
    return 0;
})";
}

TEST(RunnerTest, AcceptedExactOutputMatch) {
    std::vector<TestCase> cases = {{"", "Hello"}};
    auto result = Runner::judge(helloCode(), cases,
                                DEFAULT_TIME_LIMIT_MS, DEFAULT_MEMORY_LIMIT_MB);
    EXPECT_EQ(result.status, "accepted");
    ASSERT_EQ(result.test_results.size(), 1);
    EXPECT_TRUE(result.test_results[0].passed);
}

TEST(RunnerTest, WrongAnswer) {
    std::vector<TestCase> cases = {{"", "Hello"}};
    auto result = Runner::judge(worldCode(), cases,
                                DEFAULT_TIME_LIMIT_MS, DEFAULT_MEMORY_LIMIT_MB);
    EXPECT_EQ(result.status, "wrong_answer");
    ASSERT_EQ(result.test_results.size(), 1);
    EXPECT_FALSE(result.test_results[0].passed);
}

TEST(RunnerTest, CompileError) {
    std::vector<TestCase> cases = {{"", ""}};
    auto result = Runner::judge(compileErrorCode(), cases,
                                DEFAULT_TIME_LIMIT_MS, DEFAULT_MEMORY_LIMIT_MB);
    EXPECT_EQ(result.status, "compile_error");
    EXPECT_FALSE(result.message.empty());
    EXPECT_TRUE(result.test_results.empty());
}

TEST(RunnerTest, MultipleTestCasesAllPass) {
    std::vector<TestCase> cases = {
        {"1 2\n", "3"},
        {"10 20\n", "30"},
        {"0 0\n", "0"}
    };
    auto result = Runner::judge(sumCode(), cases,
                                DEFAULT_TIME_LIMIT_MS, DEFAULT_MEMORY_LIMIT_MB);
    EXPECT_EQ(result.status, "accepted");
    ASSERT_EQ(result.test_results.size(), 3);
    EXPECT_TRUE(result.test_results[0].passed);
    EXPECT_TRUE(result.test_results[1].passed);
    EXPECT_TRUE(result.test_results[2].passed);
}

TEST(RunnerTest, MultipleTestCasesPartialPass) {
    std::vector<TestCase> cases = {
        {"1 2\n", "3"},
        {"10 20\n", "999"},
        {"0 0\n", "0"}
    };
    auto result = Runner::judge(sumCode(), cases,
                                DEFAULT_TIME_LIMIT_MS, DEFAULT_MEMORY_LIMIT_MB);
    EXPECT_EQ(result.status, "wrong_answer");
    ASSERT_EQ(result.test_results.size(), 3);
    EXPECT_TRUE(result.test_results[0].passed);
    EXPECT_FALSE(result.test_results[1].passed);
    EXPECT_TRUE(result.test_results[2].passed);
}

TEST(RunnerTest, InfiniteLoopTimeLimit) {
    std::vector<TestCase> cases = {{"", ""}};
    auto result = Runner::judge(infiniteLoopCode(), cases,
                                500, DEFAULT_MEMORY_LIMIT_MB);
    EXPECT_EQ(result.status, "time_limit");
}

TEST(RunnerTest, TimeLimitBoundary) {
    std::vector<TestCase> cases = {{"", ""}};
    auto result = Runner::judge(sleepCode(), cases,
                                1000, DEFAULT_MEMORY_LIMIT_MB);
    EXPECT_EQ(result.status, "accepted");
    ASSERT_EQ(result.test_results.size(), 1);
    EXPECT_TRUE(result.test_results[0].passed);
}

TEST(RunnerTest, ZeroTimeLimit) {
    std::vector<TestCase> cases = {{"", ""}};
    auto result = Runner::judge(infiniteLoopCode(), cases,
                                0, DEFAULT_MEMORY_LIMIT_MB);
    EXPECT_EQ(result.status, "time_limit");
}

TEST(RunnerTest, MemoryLimitExceeded) {
    std::vector<TestCase> cases = {{"", ""}};
    auto result = Runner::judge(largeAllocCode(), cases,
                                DEFAULT_TIME_LIMIT_MS, 50);
    EXPECT_EQ(result.status, "runtime_error");
}

TEST(RunnerTest, MemoryLimitBoundary) {
    std::vector<TestCase> cases = {{"", ""}};
    auto result = Runner::judge(normalAllocCode(), cases,
                                DEFAULT_TIME_LIMIT_MS, 50);
    EXPECT_EQ(result.status, "accepted");
    ASSERT_EQ(result.test_results.size(), 1);
}

TEST(RunnerTest, DivisionByZero) {
    std::vector<TestCase> cases = {{"", ""}};
    auto result = Runner::judge(divByZeroCode(), cases,
                                DEFAULT_TIME_LIMIT_MS, DEFAULT_MEMORY_LIMIT_MB);
    EXPECT_EQ(result.status, "runtime_error");
}

TEST(RunnerTest, NullPointerDereference) {
    std::vector<TestCase> cases = {{"", ""}};
    auto result = Runner::judge(segfaultCode(), cases,
                                DEFAULT_TIME_LIMIT_MS, DEFAULT_MEMORY_LIMIT_MB);
    EXPECT_EQ(result.status, "runtime_error");
}

TEST(RunnerTest, NonZeroReturnValue) {
    std::vector<TestCase> cases = {{"", ""}};
    auto result = Runner::judge(nonZeroExitCode(), cases,
                                DEFAULT_TIME_LIMIT_MS, DEFAULT_MEMORY_LIMIT_MB);
    EXPECT_EQ(result.status, "runtime_error");
}

TEST(RunnerTest, ReadFileAttempt) {
    std::vector<TestCase> cases = {{"", ""}};
    auto result = Runner::judge(readFileCode(), cases,
                                DEFAULT_TIME_LIMIT_MS, DEFAULT_MEMORY_LIMIT_MB);
    ASSERT_EQ(result.test_results.size(), 1);
    ASSERT_FALSE(result.test_results[0].your_output.empty());
    EXPECT_EQ(result.test_results[0].status, "wrong_answer");
}

TEST(RunnerTest, SystemCall) {
    std::vector<TestCase> cases = {{"", ""}};
    auto result = Runner::judge(systemCallCode(), cases,
                                DEFAULT_TIME_LIMIT_MS, DEFAULT_MEMORY_LIMIT_MB);
    ASSERT_EQ(result.test_results.size(), 1);
}

TEST(RunnerTest, ForkAttempt) {
    std::vector<TestCase> cases = {{"", ""}};
    auto result = Runner::judge(forkCode(), cases,
                                DEFAULT_TIME_LIMIT_MS, DEFAULT_MEMORY_LIMIT_MB);
    ASSERT_EQ(result.test_results.size(), 1);
}

TEST(RunnerTest, EmptyCode) {
    std::vector<TestCase> cases = {{"", ""}};
    auto result = Runner::judge("", cases,
                                DEFAULT_TIME_LIMIT_MS, DEFAULT_MEMORY_LIMIT_MB);
    EXPECT_EQ(result.status, "compile_error");
}

TEST(RunnerTest, NoTestCases) {
    std::vector<TestCase> cases;
    auto result = Runner::judge(helloCode(), cases,
                                DEFAULT_TIME_LIMIT_MS, DEFAULT_MEMORY_LIMIT_MB);
    EXPECT_EQ(result.status, "accepted");
    EXPECT_TRUE(result.test_results.empty());
}

TEST(RunnerTest, OutputWhitespaceInsensitivity) {
    std::vector<TestCase> cases = {{"", "Hello"}};
    auto code = R"(#include <iostream>
int main() { std::cout << "  Hello  " << std::endl; return 0; })";
    auto result = Runner::judge(code, cases,
                                DEFAULT_TIME_LIMIT_MS, DEFAULT_MEMORY_LIMIT_MB);
    EXPECT_EQ(result.status, "accepted");
    ASSERT_EQ(result.test_results.size(), 1);
    EXPECT_TRUE(result.test_results[0].passed);
}
