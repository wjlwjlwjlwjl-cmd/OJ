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

    static int run(const std::string& filename, int cpu_limit, int mem_limit)
    {
        std::string exe_file = oj_utils::name_utils::Exe(filename);
        if(!oj_utils::file_utils::cp_success(exe_file))
        {
            return -1;
        }

        std::string stdin_file = oj_utils::name_utils::Stdin(filename);
        std::string stdout_file = oj_utils::name_utils::Stdout(filename);
        std::string stderr_file = oj_utils::name_utils::RunError(filename);

        int stdinfd = open(stdin_file.c_str(), O_CREAT | O_WRONLY, 0644);
        int stdoutfd = open(stdout_file.c_str(), O_CREAT | O_WRONLY, 0644);
        int stderrfd = open(stderr_file.c_str(), O_CREAT | O_WRONLY, 0644);

        
        pid_t pid;
        if((pid = fork()) < 0)
        {
            ERROR("the subprocess of the run module fail");
            return -1;
        }
        else if(pid == 0)
        {
            dup2(stdinfd, 0);
            dup2(stdoutfd, 1);
            dup2(stderrfd, 2);

            if(stdinfd < 0 || stdoutfd < 0 || stderrfd < 0)
            {
                ERROR("failed to open file of one of the 0 1 2");
                return -1;
            }

            setRunLimit(cpu_limit, mem_limit);
            execl(exe_file.c_str(), exe_file.c_str(), nullptr);
            close(stdinfd);
            close(stdoutfd);
            close(stderrfd);
            exit(1);
        }
        else
        {
            int status;
            waitpid(pid, &status, 0);
            close(stdinfd);
            close(stdoutfd);
            close(stderrfd);
            if(status & 0x7f != 0)
            {
                DEBUG("subprocess received signal {}, core dumped", status & 0x7f);
            }
            return status & 0x7f;
        }
    }
private:
    static void setRunLimit(int cpu_limit, int mem_limit)
    {
        //cpu_limit, s; mem_limit, kb
        struct rlimit r1;
        r1.rlim_cur = cpu_limit;
        r1.rlim_max = RLIM_INFINITY;
        setrlimit(RLIMIT_CPU, &r1);

        struct rlimit r2;
        r2.rlim_cur = mem_limit * 1024;
        r2.rlim_max = RLIM_INFINITY;
        setrlimit(RLIMIT_AS, &r2);
    }
};