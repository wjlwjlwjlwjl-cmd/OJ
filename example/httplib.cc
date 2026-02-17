#include "../comm/httplib.h"

using namespace httplib;

int main()
{
    Server svr;
    svr.Get("/", [](const Request& request, Response& response){
        response.set_content("Hello httplib", "content-type: text/plain;");
    });
    svr.set_base_dir("./wwwroot");
    svr.listen("0.0.0.0", 8080);
    return 0;
}