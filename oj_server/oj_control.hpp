#pragma once
#include "oj_model.hpp"
#include "oj_view.hpp"
#include "../comm/utils.hpp"
#include "../comm/httplib.h"

#include <fstream>
#include <jsoncpp/json/json.h>

/***********
 * the logic control of the oj_server 
 ********** */
class Machine
{
public:
Machine()
    : _ip(0)
    , _port(0)
    , _load(0)
    , _mtx(nullptr)
    , _online(false)
{}

Machine(std::string& ip, int port, std::mutex* mtx)
    : _ip(ip)
    , _port(port)
    , _load(0)
    , _mtx(mtx)
    , _online(true)
{}

bool online()
{
    return _online;
}

uint64_t load()
{
    return _load;
}

void IncLoad()
{
    _mtx->lock();
    _load++;
    _mtx->unlock();
}

void DesLoad()
{
    _mtx->lock();
    if(_load > 0)
    {
        _load--;
    }
    _mtx->unlock();
}

std::string ip()
{
    return _ip;
}

int port()
{
    return _port;
}

private:
    bool _online;
    std::string _ip;
    int _port;
    uint64_t _load;
    std::mutex* _mtx;
};

class LoadBalance
{
private:
    bool Load()
    {
        std::string pathname = "./conf/cs.conf";
        std::ifstream in(pathname);
        if(!in.is_open())
        {
            ERROR("open cs.conf fail");
            return false;
        }
        std::string buff;
        while(getline(in, buff))
        {
            std::vector<std::string> carrier;
            oj_utils::string_utils::StringSplit(carrier, buff, ':');
            if(carrier.size() != 2)
            {
                ERROR("split abnormal");
                return false;
            }
            std::string ip = carrier[0];
            int port = std::stoi(carrier[1]);
            DEBUG("{}  //  {}", ip, port);
            std::mutex m_mtx;
            Machine m(ip, port, &m_mtx);
            _machines.push_back(m);
            buff.clear();
        }
        DEBUG("{} are online in load", _machines.size());
        return true;
    }
public:
    LoadBalance()
    {
        if(!Load())
        {
            ERROR("parse the conf fail");
            exit(1);
        }
    }

    bool SmartChoose(Machine* m)
    {
        mtx.lock();
        if(_machines.size() == 0)
        {
            ERROR("all of the compile server died!!!!!!!!!!!!");
            return false;
        }
        //polling
        int choice = 0;
        for(int i = 0; i < _machines.size(); i++)
        {
            if(_machines[i].online())
            {
                if(_machines[choice].load() > _machines[i].load())
                {
                    choice = i;
                }
            }
        }
        *m = _machines[choice];
        mtx.unlock();
    }

    bool Offline(const std::string& ip, int port)
    {
        std::vector<Machine>::iterator it = _machines.begin();
        for(; it != _machines.end(); it++)
        {
            if(it->ip() == ip && it->port() == port)
            {
                _machines.erase(it);
                return true;
            }
        }
        ERROR("haven't found the host {}:{}", ip, port);
        return false;
    }

    bool Online()
    {

    }

    ~LoadBalance()
    {}
private:
    std::vector<Machine> _machines;
    std::mutex mtx;
};

class ProblemControl
{
public:
    ProblemControl(const std::shared_ptr<odb::core::database> db)
    {
        _pt = std::make_shared<ProblemTable>(db);
    }

    void GetAllQuestion(std::string& html)
    {
        std::vector<Problem> probs;
        if(_pt->SelectAll(probs))
        {
            ProblemView::ExpandAllQuestion(html, probs);
        }
        else
        {
            DEBUG("select all fail");
        }
    }

    void GetOneQuestion(int number, std::string& html)
    {
        auto ret = _pt->SelectOne(number);
        DEBUG("*{}*", ret->title());
        if(ret)
        {
            ProblemView::ExpandOneQuesiton(number, html, ret);
        }
        else
        {
            DEBUG("select single fail");
        }
    }

    void judge(int number, std::string& in_string, std::string& out_string)
    {
        auto prob = _pt->SelectOne(number);
        Json::Reader reader;
        Json::Value value;
        reader.parse(in_string, value);
        std::string code = value["code"].asString(); //user's code
        Json::Value compile;
        compile["input"] = value["input"].asString();
        compile["code"] = code + prob->tail();
        compile["cpu_limit"] = prob->cpu_limit();
        compile["mem_limit"] = prob->mem_limit();
        Json::FastWriter writer;
        std::string to_compile = writer.write(compile);
        
        while(1)
        {
            Machine* m;
            if(!lb.SmartChoose(m))
            {
                break;
            }
            DEBUG("choose cs host {}: {}", m->ip(), m->port());
            m->IncLoad();
            Client client(m->ip(), m->port());
            if(auto ret = client.Post("/compile_run", to_compile, "application/json;charset=utf-8"))
            {
                out_string = ret->body;
                m->DesLoad();
            }
            else
            {
                DEBUG("host {}:{} offline", m->ip(), m->port());
                lb.Offline(m->ip(), m->port());
            }
        }
    }
private:
    std::shared_ptr<ProblemTable> _pt;
    LoadBalance lb;
};