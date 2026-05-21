#include "manage.hpp"

#include <iostream>
#include <vector>

std::shared_ptr<odb::core::database> db(new odb::mysql::database("oj_client", 
"123456", "oj", "172.17.0.1", 0, 0, "utf8"));
ProblemTable ptb(db);

int main()
{
    init_logger();
    //auto ret = ptb.selectOne("jigajreigrje");
    //std::cout << ret->title() << std::endl;
    //std::cout << ret->desc() << std::endl;
    //std::cout << ret->star() << std::endl;

    std::vector<Problem> ret;
    ptb.SelectAll(ret);
    for(auto& e: ret)
    {
        std::cout << e.number() << std::endl;
        std::cout << e.title() << std::endl;
    }
    return 0;
}