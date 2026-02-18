/***************
 * odb -d mysql --generate-query --generate-schema --profile boost/date-time person.hxx
 ****************/
#pragma once
#include <string>
#include <odb/database.hxx>
#include <odb/core.hxx>

#pragma db object
#pragma db table("problem")
class Problem
{
public:
    Problem(int number, const std::string& title, const std::string& star, const std::string& desc, 
        const std::string& header, const std::string& tail,
        int cpu_limit = 1, int mem_limit = 5000)
        : _number(number)
        , _title(title) 
        , _star(star)
        , _desc(desc)
        , _header(header)
        , _tail(tail)
        , _cpu_limit(cpu_limit)
        , _mem_limit(mem_limit)
    {}
    int number()
    {
        return _number;
    }

    void number(int number)
    {
        _number = number;
    }

    std::string title()
    {
        return _title;
    }

    void title(const std::string& title)
    {
        _title = title;
    }

    std::string star()
    {
        return _star;
    }

    void star(const std::string& star)
    {
        _star = star;
    }

    std::string desc()
    {
        return _desc;
    }

    void desc(const std::string& desc)
    {
        _desc = desc;
    }

    std::string header()
    {
        return _header;
    }

    void header(const std::string& header)
    {
        _header = header;
    }

    std::string tail()
    {
        return _tail;
    }

    void tail(const std::string& tail)
    {
        _tail = tail;
    }

    void cpu_limit(int limit)
    {
        _cpu_limit = limit;
    }

    int cpu_limit()
    {
        return _cpu_limit;
    }

    int mem_limit()
    {
        return _mem_limit;
    }

    void mem_limit(int limit)
    {
        _mem_limit = limit;
    }
private:
    friend class odb::access;
    #pragma db id auto
    int _id; //number of the problem
    #pragma db unique
    int _number;
    #pragma db type("varchar(128)")
    std::string _title; //the title of the problem
    #pragma db type("varchar(8)")
    std::string _star; //the difficulty of the problem
    std::string _desc; // description of the problem
    std::string _header; // the prebuilded code given to the user
    std::string _tail; //the code of the given example
    #pragma db default(1)
    int _cpu_limit; // the limit of the cpu and mem
    #pragma db default(50000)
    int _mem_limit;
};