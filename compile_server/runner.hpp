#include "../comm/utils.hpp"
#include "../comm/Logger.hpp"

#include <sys/wait.h>

class Runner
{
public:
    Runner()
    {}

    ~Runner()
    {}

    int run(const std::string& filename)
    {
        pid_t pid;
        if((pid = fork()) < 0)
        {
            ERROR("the subprocess of the run module fail");
            return -1;
        }
        else if(pid == 0)
        {
            std::string exe_file = oj_utils::name_utils::Exe(filename);
            std::string stdin_file = oj_utils::name_utils::Stdin(filename);
            std::string stdout_file = oj_utils::name_utils::Stdout(filename);
            std::string stderr_file = oj_utils::name_utils::RunError(filename);

            int stdinfd = open(stdin_file.c_str(), O_CREAT | O_WRONLY, 0644);
            int stdoutfd = open(stdout_file.c_str(), O_CREAT | O_WRONLY, 0644);
            int stderrfd = open(stderr_file.c_str(), O_CREAT | O_WRONLY, 0644);

            dup2(stdinfd, 0);
            dup2(stdoutfd, 1);
            dup2(stderrfd, 2);

            if(stdinfd < 0 || stdoutfd < 0 || stderrfd < 0)
            {
                ERROR("failed to open file of one of the 0 1 2");
                return -1;
            }

            execl(exe_file.c_str(), exe_file.c_str(), nullptr);
            close(stdinfd);
            close(stdoutfd);
            close(stderrfd);
            return -1;
        }
        else
        {
            int status;
            waitpid(pid, &status, 0);
            return status & 0x7f;
        }
    }
private:
};