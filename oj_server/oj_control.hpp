#pragma once
#include "oj_model.hpp"
#include "oj_view.hpp"
#include "../comm/utils.hpp"
#include "../comm/httplib.h"

#include <fstream>
#include <jsoncpp/json/json.h>

using namespace httplib;

/***********
 * the logic control of the oj_server 
 ********** */
class Machine
{
public:
    bool _online;
    std::string _ip;
    int _port;
    int _load;
    std::mutex* _mtx;
public:
    Machine()
        : _ip("")
        , _port(0)
        , _load(0)
        , _mtx(nullptr)
        , _online(false)
    {}

    void IncLoad()
    {
        if(_mtx)
        _mtx->lock();
        _load++;
        if(_mtx)
        _mtx->unlock();
    }

    void DesLoad()
    {
        if(_mtx)
            _mtx->lock();
        if(_load > 0)
        {
            _load--;
        }
        if(_mtx)
            _mtx->unlock();
    }
};

class LoadBalance
{
private:
    bool Load()
    {
        srand(time(0));
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
            Machine m;
            m._ip = ip;
            m._port = port;
            m._mtx = new std::mutex();
            m._online = true;
            m._load = 0;
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
        DEBUG("parse conf file success");
    }

    bool SmartChoose(Machine** m)
    {
        mtx.lock();
        if(_machines.size() == 0)
        {
            ERROR("all of the compile server died!!!!!!!!!!!!");
            return false;
        }
        //polling
        // int choice = 0;
        // for(int i = 0; i < _machines.size(); i++)
        // {
        //     if(_machines[i]._online)
        //     {
        //         if(_machines[choice]._load > _machines[i]._load)
        //         {
        //             choice = i;
        //         }
        //     }
        // }
        int choice = rand() % _machines.size();
        *m = &_machines[choice];
        std::cout << "**********the choice is " << choice << std::endl;
        mtx.unlock();
        return true;
    }

    bool Offline(const std::string& ip, int port)
    {
        mtx.lock();
        std::vector<Machine>::iterator it = _machines.begin();
        for(; it != _machines.end(); it++)
        {
            if(it->_ip == ip && it->_port == port)
            {
                it->_online = false;
                return true;
            }
        }
        mtx.unlock();
        ERROR("haven't found the host {}:{}", ip, port);
        return false;
    }

    bool Online()
    {
        return true;
    }

    void ShowMachines()
    {
        mtx.lock();
        for(auto& e: _machines)
        {
            std::cout << e._ip << ": " << e._port << "---" << e._online << std::endl;
        }
        mtx.unlock();
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
        if(ret)
        {
            ProblemView::ExpandOneQuesiton(number, html, ret);
        }
        else
        {
            DEBUG("select single fail");
        }
    }

    void judge(int number, const std::string& in_string, std::string& out_string)
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
            if(!lb.SmartChoose(&m))
            {
                break;
            }
            m->IncLoad();
            Client client(m->_ip, m->_port);
            auto ret = client.Post("/compile_run", to_compile, "application/json;charset=utf-8");
            out_string = ret->body;
            DEBUG("choose {}: {}", m->_ip, m->_port);
            if(auto ret = client.Post("/compile_run", to_compile, "application/json;charset=utf-8"))
            {
                if(ret->status == 200)
                {
                    out_string = ret->body; //std::unique_ptr<Response>
                    m->DesLoad();
                    DEBUG("{}: {} cr success", m->_ip, m->_port);
                    break;
                }
                else
                {
                    m->DesLoad();
                    continue;
                }
            }
            else
            {
                DEBUG("host {}:{} offline", m->_ip, m->_port);
                lb.Offline(m->_ip, m->_port);
                lb.ShowMachines();
                break;
            }
        }
    }
private:
    std::shared_ptr<ProblemTable> _pt;
    LoadBalance lb;
};