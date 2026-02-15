#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include <string>

namespace oj_utils
{
    class file_utils
    {
    public:
        bool exists(const std::string& filename)
        {
            struct stat file_stat;
            return stat(filename.c_str(), &file_stat) == 0;
        }
    private:
    };

    class name_utils
    {
    public:
        static std::string& Src(std::string& filename)
        {
            return filename += ".cpp";
        }

        static std::string Exe(std::string& filename)
        {
            std::string pathname = "./temp" + filename + ".exe";
            return pathname;
        }

        static std::string Error(std::string& filename)
        {
            std::string pathname = "./temp" + filename + ".error";
            return pathname;
        }
    };
}
