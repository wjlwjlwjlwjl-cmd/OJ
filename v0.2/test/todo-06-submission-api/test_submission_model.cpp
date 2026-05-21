#include <gtest/gtest.h>
#include <mysql/mysql.h>

#include "../../backend/src/models/submission.h"
#include "../../backend/src/models/user.h"
#include "../../backend/src/models/problem.h"
#include "../../backend/src/db/connection.h"

static int testUserId = 0;
static int testProblemId = 0;

static const char* TEST_USER = "test_sub_user";
static const char* TEST_PROBLEM_TITLE = "Test Submission Problem";

class SubmissionModelTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        DbConfig cfg;
        cfg.host     = "127.0.0.1";
        cfg.port     = 3306;
        cfg.user     = "oj";
        cfg.password = "oj_password";
        cfg.database = "oj";
        DbConnection::init(cfg);

        DbConnection::execute(
            "CREATE TABLE IF NOT EXISTS users ("
            "id INT AUTO_INCREMENT PRIMARY KEY,"
            "username VARCHAR(64) NOT NULL UNIQUE,"
            "password VARCHAR(256) NOT NULL,"
            "role ENUM(\"admin\",\"user\") NOT NULL DEFAULT \"user\","
            "created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,"
            "updated_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP"
            ") ENGINE=InnoDB");

        DbConnection::execute(
            "CREATE TABLE IF NOT EXISTS problems ("
            "id INT AUTO_INCREMENT PRIMARY KEY,"
            "title VARCHAR(256) NOT NULL,"
            "description TEXT NOT NULL,"
            "difficulty ENUM(\"easy\",\"medium\",\"hard\") NOT NULL DEFAULT \"easy\","
            "time_limit INT NOT NULL DEFAULT 1000,"
            "memory_limit INT NOT NULL DEFAULT 256,"
            "test_cases LONGBLOB,"
            "created_by INT NOT NULL,"
            "created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,"
            "updated_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,"
            "FOREIGN KEY (created_by) REFERENCES users(id)"
            ") ENGINE=InnoDB");

        DbConnection::execute(
            "CREATE TABLE IF NOT EXISTS submissions ("
            "id INT AUTO_INCREMENT PRIMARY KEY,"
            "user_id INT NOT NULL,"
            "problem_id INT NOT NULL,"
            "code TEXT NOT NULL,"
            "language VARCHAR(32) NOT NULL DEFAULT \"cpp\","
            "status ENUM(\"pending\",\"judging\",\"accepted\",\"wrong_answer\","
            "  \"compile_error\",\"time_limit\",\"memory_limit\",\"runtime_error\")"
            "  NOT NULL DEFAULT \"pending\","
            "score INT DEFAULT 0,"
            "judge_result JSON,"
            "submitted_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,"
            "judged_at DATETIME,"
            "FOREIGN KEY (user_id) REFERENCES users(id),"
            "FOREIGN KEY (problem_id) REFERENCES problems(id)"
            ") ENGINE=InnoDB");

        DbConnection::execute("DELETE FROM submissions");
        DbConnection::execute("DELETE FROM problems WHERE title LIKE 'Test%'");
        DbConnection::execute("DELETE FROM users WHERE username LIKE 'test_%'");

        auto user = UserModel::create(TEST_USER, "testpass123");
        ASSERT_TRUE(user.has_value());
        testUserId = user->id;

        auto problem = ProblemModel::create(TEST_PROBLEM_TITLE,
                                             "Test description", "easy",
                                             1000, 256, "[]", testUserId);
        ASSERT_TRUE(problem.has_value());
        testProblemId = problem->id;
    }

    static void TearDownTestSuite() {
        DbConnection::execute("DELETE FROM submissions");
        DbConnection::execute("DELETE FROM problems WHERE title LIKE 'Test%'");
        DbConnection::execute("DELETE FROM users WHERE username LIKE 'test_%'");
        DbConnection::close();
    }

    void TearDown() override {
        DbConnection::execute("DELETE FROM submissions");
    }
};

TEST_F(SubmissionModelTest, CreateSubmission_ReturnsPending) {
    auto sub = SubmissionModel::create(testUserId, testProblemId,
                                        "#include <iostream>\nint main(){return 0;}",
                                        "cpp");
    ASSERT_TRUE(sub.has_value());
    EXPECT_EQ(sub->user_id, testUserId);
    EXPECT_EQ(sub->problem_id, testProblemId);
    EXPECT_EQ(sub->language, "cpp");
    EXPECT_EQ(sub->status, "pending");
    EXPECT_EQ(sub->score, 0);
    EXPECT_GT(sub->id, 0);
}

TEST_F(SubmissionModelTest, CreateSubmission_EmptyCode) {
    auto sub = SubmissionModel::create(testUserId, testProblemId, "", "cpp");
    ASSERT_TRUE(sub.has_value());
    EXPECT_EQ(sub->status, "pending");
}

TEST_F(SubmissionModelTest, UpdateSubmissionStatus) {
    auto sub = SubmissionModel::create(testUserId, testProblemId,
                                        "int main(){return 0;}", "cpp");
    ASSERT_TRUE(sub.has_value());

    std::string judgeResult = R"({"status":"accepted","test_results":[{"passed":true}]})";
    bool updated = SubmissionModel::updateStatus(sub->id, "accepted", 100, judgeResult);
    EXPECT_TRUE(updated);

    auto fetched = SubmissionModel::findById(sub->id);
    ASSERT_TRUE(fetched.has_value());
    EXPECT_EQ(fetched->status, "accepted");
    EXPECT_EQ(fetched->score, 100);
    EXPECT_FALSE(fetched->judge_result.empty());
    EXPECT_FALSE(fetched->judged_at.empty());
}

TEST_F(SubmissionModelTest, UpdateJudgingStatus) {
    auto sub = SubmissionModel::create(testUserId, testProblemId,
                                        "int main(){return 0;}", "cpp");
    ASSERT_TRUE(sub.has_value());

    bool updated = SubmissionModel::updateJudging(sub->id);
    EXPECT_TRUE(updated);

    auto fetched = SubmissionModel::findById(sub->id);
    ASSERT_TRUE(fetched.has_value());
    EXPECT_EQ(fetched->status, "judging");
}

TEST_F(SubmissionModelTest, FindById_Existing) {
    auto sub = SubmissionModel::create(testUserId, testProblemId,
                                        "int main(){return 42;}", "cpp");
    ASSERT_TRUE(sub.has_value());

    auto fetched = SubmissionModel::findById(sub->id);
    ASSERT_TRUE(fetched.has_value());
    EXPECT_EQ(fetched->id, sub->id);
    EXPECT_EQ(fetched->code, "int main(){return 42;}");
}

TEST_F(SubmissionModelTest, FindById_NonExistent) {
    auto fetched = SubmissionModel::findById(9999999);
    EXPECT_FALSE(fetched.has_value());
}

TEST_F(SubmissionModelTest, FindByUserId_ReturnsAll) {
    SubmissionModel::create(testUserId, testProblemId, "code1", "cpp");
    SubmissionModel::create(testUserId, testProblemId, "code2", "cpp");
    SubmissionModel::create(testUserId, testProblemId, "code3", "cpp");

    auto submissions = SubmissionModel::findByUserId(testUserId, 10, 0);
    EXPECT_GE((int)submissions.size(), 3);

    for (const auto& s : submissions) {
        EXPECT_EQ(s.user_id, testUserId);
    }
}

TEST_F(SubmissionModelTest, FindByUserId_Pagination) {
    SubmissionModel::create(testUserId, testProblemId, "code_a", "cpp");
    SubmissionModel::create(testUserId, testProblemId, "code_b", "cpp");
    SubmissionModel::create(testUserId, testProblemId, "code_c", "cpp");

    auto page1 = SubmissionModel::findByUserId(testUserId, 2, 0);
    EXPECT_EQ((int)page1.size(), 2);

    auto page2 = SubmissionModel::findByUserId(testUserId, 2, 2);
    EXPECT_GE((int)page2.size(), 1);
}

TEST_F(SubmissionModelTest, FindByProblemId_ReturnsAll) {
    SubmissionModel::create(testUserId, testProblemId, "code_p1", "cpp");
    SubmissionModel::create(testUserId, testProblemId, "code_p2", "cpp");

    auto submissions = SubmissionModel::findByProblemId(testProblemId, 10, 0);
    EXPECT_GE((int)submissions.size(), 2);

    for (const auto& s : submissions) {
        EXPECT_EQ(s.problem_id, testProblemId);
    }
}

TEST_F(SubmissionModelTest, CountByUserId) {
    int before = SubmissionModel::countByUserId(testUserId);
    SubmissionModel::create(testUserId, testProblemId, "count_test", "cpp");
    int after = SubmissionModel::countByUserId(testUserId);
    EXPECT_EQ(after, before + 1);
}

TEST_F(SubmissionModelTest, CountByProblemId) {
    int before = SubmissionModel::countByProblemId(testProblemId);
    SubmissionModel::create(testUserId, testProblemId, "count_test2", "cpp");
    int after = SubmissionModel::countByProblemId(testProblemId);
    EXPECT_EQ(after, before + 1);
}
