#ifndef ROUTER_H
#define ROUTER_H

#include "server.h"

class Router {
public:
    explicit Router(Server& server);

    void setupRoutes();

private:
    Server& m_server;

    void setupAuthRoutes();
    void setupProblemRoutes();
    void setupSubmissionRoutes();
    void setupJudgeRoutes();
};

#endif
