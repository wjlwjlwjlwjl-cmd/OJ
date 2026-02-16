#pragma once
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include <string>
#include <iostream>
#include <random>
#include <sstream>
#include <iomanip>
#include <atomic>
#include <fstream>

#include "Logger.hpp"

namespace oj_utils
{
    class file_utils
    {
    public:
        //if the executable file exists, the compile is successful;
        static bool cp_success(const std::string& filename)
        {
            struct stat file_stat;
            return stat(filename.c_str(), &file_stat) == 0;
        }

        static std::string unique_file_name()
        {
            std::stringstream ss;

            std::random_device rd;
            std::mt19937 mt(rd());
            std::uniform_int_distribution<int> distribution(0, 255);

            for(int i = 0; i < 6; i++)
            {
                ss << std::setw(2) << std::setfill('0') << std::hex << distribution(mt);
            }

            std::atomic<int> ato(0);
            int tmp = ato.fetch_add(1);
            ss << "-" << std::setw(4) << std::setfill('0') << ato;
            DEBUG("uniq file name {}", ss.str());
            return ss.str();
        }

        static bool read_file(const std::string& target, std::string& carrier, bool keep)
        {
            std::ifstream in(target);
            if(!in.is_open())
            {
                ERROR("fail open file to read {}", target);
                return false;
            }
            std::string buffer;
            while(getline(in, buffer))
            {
                carrier += buffer;
                carrier += keep ?"\n":"";
                buffer.clear();
            }
            return true;
        }

        static bool write_file(const std::string& target, const std::string content)
        {
            std::ofstream out(target);
            if(!out.is_open())
            {
                ERROR("fail to open file to write {}", target);
                return false;
            }
            out.write(content.c_str(), content.size());
            return true;
        }
    private:
    };

    class name_utils
    {
    public:
        // ********** files generated during the compile process **********
        //add the suuffix to the filename
        static std::string Src(const std::string& filename)
        {
            return "./test_code/" + filename + ".cpp";
        }

        //give the path and exe's name
        static std::string Exe(const std::string& filename)
        {
            std::string pathname = "./tmp/" + filename + ".exe";
            return pathname;
        }

        //give the path and err's name
        static std::string CompileError(const std::string& filename)
        {
            std::string pathname = "./tmp/" + filename + ".compile_error";
            return pathname;
        }

        // ********** files generated during the run process **********
        static std::string Stdin(const std::string& filename)
        {
            std::string pathname = "./tmp/" + filename + ".stdin";
            return pathname;
        }

        static std::string Stdout(const std::string& filename)
        {
            std::string pathname = "./tmp/" + filename + ".stdout";
            return pathname;
        }

        static std::string RunError(const std::string& filename)
        {
            std::string pathname = "./tmp/" + filename + ".run_error";
            return pathname;
        }
    };
}
