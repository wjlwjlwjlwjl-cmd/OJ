#ifndef PROBLEM_CONTROLLER_H
#define PROBLEM_CONTROLLER_H

#include "httplib.h"

class ProblemController {
public:
    static void handleListProblems(const httplib::Request& req, httplib::Response& res);
    static void handleGetProblem(const httplib::Request& req, httplib::Response& res);
    static void handleCreateProblem(const httplib::Request& req, httplib::Response& res);
    static void handleUpdateProblem(const httplib::Request& req, httplib::Response& res);
    static void handleDeleteProblem(const httplib::Request& req, httplib::Response& res);
};

#endif
