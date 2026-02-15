#include "../comm/utils.hpp"
#include "../comm/Logger.hpp"

#include <sys/wait.h>

class Compiler
{
public:
    Compiler()
    {}

    ~Compiler()
    {}

    bool compile(std::string filename)
    {
        pid_t pid;
        if((pid = fork()) < 0)
        {
            ERROR("copiler's sub process fail");
            exit(1);
        }
        else if(pid == 0)
        {
            std::string src_file = oj_utils::name_utils::Src(filename);
            std::string exe_file = oj_utils::name_utils::Exe(filename);
            std::string err_file = oj_utils::name_utils::Error(filename);

            int errfd = open(err_file.c_str(), O_CREAT | O_WRONLY, 0644);
            if(errfd < 0)
            {
                ERROR("errfile generate fail");
                exit(1);
            }
            dup2(errfd, 2);
            execlp("g++", "g++", src_file.c_str(), "-o", exe_file.c_str(), "-std=c++11", nullptr);
        }
        else
        {
            int status;
            waitpid(pid, &status, 0);
            if(oj_utils::file_utils::cp_success(oj_utils::name_utils::Exe(filename)))
            {
                return true;
            }
        }
        return false;
    }
private:
};
