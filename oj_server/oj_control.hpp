#pragma once
#include "oj_model.hpp"
#include "oj_view.hpp"

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
private:
    std::shared_ptr<ProblemTable> _pt;
};