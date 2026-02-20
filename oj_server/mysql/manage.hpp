#pragma once
#include "problem.hxx"
#include "problem-odb.hxx"
#include "../../comm/Logger.hpp"

#include <odb/mysql/database.hxx>
#include <odb/database.hxx>
#include <memory>
#include <odb/mysql/connection-factory.hxx>

using namespace odb;
class ProblemTable
{
public:
    ProblemTable(const std::shared_ptr<odb::core::database> db)
        : _db(db)
    {}

    bool insert(Problem& prob)
    {
        try
        {
            odb::transaction trans(_db->begin());
            _db->persist(prob);
            trans.commit();
        }
        catch(const std::exception& e)
        {
            DEBUG("insert fail {}", e.what());
            return false;
        }
        return true;
    }

    std::shared_ptr<Problem> selectOne(const std::string& title)
    {
        std::shared_ptr<Problem> ret;
        try
        {
            odb::transaction trans(_db->begin());
            typedef odb::query<Problem> query;
            typedef odb::result<Problem> result;
            ret.reset(_db->query_one<Problem>(query::title == title));
            trans.commit();
        }
        catch(const std::exception& e)
        {
            DEBUG("{} select one fail {}", title, e.what());
            return nullptr;
        }
        return ret;
    }

    bool SelectAll(std::vector<Problem>& probs)
    {
        try
        {
            odb::transaction trans(_db->begin());
            typedef odb::query<Problem> problem;
            typedef odb::result<Problem> result;
            result ret(_db->query<Problem>());
            for(auto& e: ret)
            {
                Problem prob(e.number(), e.title(), e.star(), e.desc(), e.header(), e.tail(), e.cpu_limit(), e.mem_limit());
                probs.push_back(prob);
            }
            trans.commit();
        }
        catch(const std::exception& e)
        {
            DEBUG("select all of the problem fail {}", e.what());
            return false;
        }
        return true;
    }

    bool erase(int number)
    {
        try
        {
            odb::transaction trans(_db->begin());
            typedef odb::query<Problem> query;
            typedef odb::query<Problem> result;
            _db->erase_query<Problem>(number == query::number);
            trans.commit();
        }
        catch(const std::exception& e)
        {
            DEBUG("erase fail {}", e.what());
            return false;
        }
        return true;
    }
private:
    std::shared_ptr<odb::core::database> _db;
};