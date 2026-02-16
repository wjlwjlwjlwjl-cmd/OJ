#include <iostream>
#include <jsoncpp/json/json.h>

int main()
{
    Json::Value root;
    root["name"] = "wang";
    root["age"] = 19;
    root["school"] = "Fudan university";
    Json::Value hobby;
    hobby.append("coding");
    hobby.append("fitness");
    root["hobby"] = hobby;

    //Json::FastWriter writer;
    Json::StyledWriter writer;
    std::string content = writer.write(root);
    std::cout << content << std::endl;
    return 0;
}