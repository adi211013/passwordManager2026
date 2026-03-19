#include "crow.h"
#include <string>

#include "crypto/Crypto.h"
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
    CROW_ROUTE(app, "/register").methods(crow::HTTPMethod::POST)
        ([&db](const crow::request& req){
            auto body=crow::json::load(req.body);
            if (!body||!body.has("login")||!body.has("password"))
            {
                return crow::response(400,"{\"error\": \"Brakk loginu lub hasla\"}");
            }
            std::string login=body["login"].s();
            std::string passowrd=body["password"].s();
            std::string hash=Crypto::hashPassword(passowrd);
            if (db.registerUser(login,hash))
            {
                return crow::response(201,"{\"status\": \"Konto utworzone\"}");
            }
                return crow::response(409,"{\"status\": \"Login jest juz zajety lub wystapil blad bazy danych\"}");
    });
    CROW_ROUTE(app,"/ping")([]()
    {
       return crow::response(200,"{\"status\": \"ok\"}");
    });
    app.port((18080)).multithreaded().run();

    return 0;
}
