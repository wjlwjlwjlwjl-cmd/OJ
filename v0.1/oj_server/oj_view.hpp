#pragma once
#include "oj_model.hpp"

#include <ctemplate/template.h>

class ProblemView
{
public:
    static void ExpandAllQuestion(std::string& html, std::vector<Problem>& probs)
    {
        std::string in_html = "./template_html/all_questions.html";
        ctemplate::TemplateDictionary root("all_problems");
        for(int i = 0; i < probs.size(); i++)
        {
            ctemplate::TemplateDictionary* sub = root.AddSectionDictionary("question_list");
            sub->SetValue("number", std::to_string(probs[i].number()));
            sub->SetValue("title", probs[i].title());
            sub->SetValue("star", probs[i].star());
        }
        ctemplate::Template *tpl = ctemplate::Template::GetTemplate(in_html, ctemplate::DO_NOT_STRIP);
        tpl->Expand(&html, &root);
    }

    static void ExpandOneQuesiton(int number, std::string& html, std::shared_ptr<Problem>& prob)
    {
        std::string in_html = "./template_html/one_question.html";
        ctemplate::TemplateDictionary root("one_question");

        root.SetValue("number", std::to_string(prob->number()));
        root.SetValue("title", prob->title());
        root.SetValue("star", prob->star());
        root.SetValue("desc", prob->desc());
        root.SetValue("header", prob->header());
        DEBUG("**{}**", prob->title());

        ctemplate::Template *tpl = ctemplate::Template::GetTemplate(in_html, ctemplate::DO_NOT_STRIP);
        tpl->Expand(&html, &root);
    }
private:
};