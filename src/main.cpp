#include "crow.h"
#include <string>

#include "database/DbManager.h"

int main()
{
    std::string connStr="dbname=passmanager user=admin password=admin host=127.0.0.1 port=5432";
    DbManager db(connStr);
    if (!db.checkConnection())
    {
        std::cerr<<"Brak polaczenia z baza danych"<<std::endl;
        return 1;
    }
    crow::SimpleApp app;
    CROW_ROUTE(app,"/")([]()
    {
        return "dziala hujstwo";
    });
    CROW_ROUTE(app,"/ping")([]()
    {
       return crow::response(200,"{\"status\": \"ok\"}");
    });
    app.port((18080)).multithreaded().run();

    return 0;
}
