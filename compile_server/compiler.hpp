#pragma once
#include "../comm/utils.hpp"
#include "../comm/Logger.hpp"

#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/time.h>

class Compiler
{
public:
    Compiler()
    {}

    ~Compiler()
    {}

    static bool compile(std::string filename)
    {
        std::string src_file = oj_utils::name_utils::Src(filename);
        std::string exe_file = oj_utils::name_utils::Exe(filename);
        std::string err_file = oj_utils::name_utils::CompileError(filename);
        int errfd = open(err_file.c_str(), O_CREAT | O_WRONLY, 0644);

        pid_t pid;
        if((pid = fork()) < 0)
        {
            ERROR("copiler's sub process fail");
            return false;
        }
        else if(pid == 0)
        {
            umask(0); // avoid the influence of the platform
            if(errfd < 0)
            {
                ERROR("errfile generate fail {}", filename);
                return false;
            }
            dup2(errfd, 2);
            execlp("g++", "g++", src_file.c_str(), "-o", exe_file.c_str(), "-std=c++11", nullptr);
            close(errfd);
            exit(1);
        }
        else
        {
            int status;
            waitpid(pid, &status, 0);
            if(oj_utils::file_utils::cp_success(oj_utils::name_utils::Exe(filename)))
            {
                return true;
            }
            close(errfd);
        }
        return false;
    }
};
