#ifndef JUDGE_SERVER_H
#define JUDGE_SERVER_H

#include <string>
#include "httplib.h"

class JudgeServer {
public:
    JudgeServer();
    bool listen(const std::string& host, int port);
    void stop();

private:
    httplib::Server m_server;
    bool m_running;

    void setupRoutes();
    void handleJudge(const httplib::Request& req, httplib::Response& res);
};

#endif
