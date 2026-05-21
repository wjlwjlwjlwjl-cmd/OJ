#ifndef SUBMISSION_CONTROLLER_H
#define SUBMISSION_CONTROLLER_H

#include "httplib.h"

class SubmissionController {
public:
    static void handleSubmit(const httplib::Request& req, httplib::Response& res);
    static void handleGetSubmission(const httplib::Request& req, httplib::Response& res);
    static void handleListSubmissions(const httplib::Request& req, httplib::Response& res);
};

#endif
