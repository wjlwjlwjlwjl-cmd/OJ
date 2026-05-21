#ifndef PROBLEM_MODEL_H
#define PROBLEM_MODEL_H

#include <string>
#include <optional>
#include <vector>

struct Problem {
    int id;
    std::string title;
    std::string description;
    std::string difficulty;
    int time_limit;
    int memory_limit;
    std::string test_cases;
    int created_by;
    std::string created_at;
    std::string updated_at;
};

class ProblemModel {
public:
    static std::vector<Problem> findAll();
    static std::optional<Problem> findById(int id);
    static std::optional<Problem> create(const std::string& title,
                                          const std::string& description,
                                          const std::string& difficulty,
                                          int time_limit,
                                          int memory_limit,
                                          const std::string& test_cases,
                                          int created_by);
    static bool update(int id, const std::string& title,
                       const std::string& description,
                       const std::string& difficulty,
                       int time_limit,
                       int memory_limit,
                       const std::string& test_cases);
    static bool remove(int id);
};

#endif
