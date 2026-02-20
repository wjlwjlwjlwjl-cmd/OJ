#include "manage.hpp"
#include "../../comm/utils.hpp"

#include <iostream>

std::shared_ptr<odb::core::database> db(new odb::mysql::database("oj_client", 
"123456", "oj", "172.17.0.1", 0, 0, "utf8"));
ProblemTable ptb(db);

Problem getNew()
{
    int number;
    std::cout << "please select the number of the problem## ";
    std::cin >> number; 
    std::cin.ignore();

    std::string title;
    std::cout << "please input the title## ";
    getline(std::cin, title);

    std::string star;
    std::cout << "please input the star## ";
    getline(std::cin, star);

    std::string filename_desc;
    std::cout << "please give the file name where desc is stored## ";
    getline(std::cin, filename_desc);

    std::string filename_header;
    std::cout << "please give the file name where header is stored## ";
    getline(std::cin, filename_header);

    std::string filename_tail;
    std::cout << "please give the file name where tail is stored## ";
    getline(std::cin, filename_tail);

    int cpu_limit, mem_limit;
    std::cout << "please input the cpu limit## ";
    std::cin >> cpu_limit;
    std::cout << "please input the mem limit## ";
    std::cin >> mem_limit;

    std::string desc;
    oj_utils::file_utils::read_file("./tmp/" + filename_desc, desc, true);
    std::string header;
    oj_utils::file_utils::read_file("./tmp/" + filename_header, header, true);
    std::string tail;
    oj_utils::file_utils::read_file("./tmp/" + filename_tail, tail, true);
    
    Problem prob(number, title, star, desc, header, tail, cpu_limit, mem_limit);
    return prob;
}

void insert()
{
    Problem prob = getNew();
    if(ptb.insert(prob))
        std::cout << "insert success!!!" << std::endl;
}

void erase()
{
    std::cout << "please input the number of the problem to be erased## ";
    int number;
    std::cin >> number;
    std::cin.ignore();
    if(ptb.erase(number))
        std::cout << "erase success!!!" << std::endl;
}

int main()
{
    init_logger();
    std::cout << "***********************************************\n";
    std::cout << "* welcome to the problem adding service!\n";
    std::cout << "* services are listed as follows:\n";
    std::cout << "* 1. insert\n";
    std::cout << "* 2. erase by problem number\n";
    std::cout << "* 3. exit\n";
    std::cout << "***********************************************\n";
    std::cout << "please select one ##";
    int number;
    std::cin >> number;
    std::cin.ignore();
    switch(number)
    {
    case 1:
        insert();
        break;
    case 2:
        erase();
        break;
    case 3:
        return 0;
    default:
        std::cout << "unsupported request!!!";
    }
    return 0;
}