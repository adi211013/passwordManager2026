#include "crow.h"
#include <string>

#include "crypto/Crypto.h"
#include "database/DbManager.h"

int main()
{
    try {
        Crypto::init();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
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
                return crow::response(400,"{\"error\": \"Brakk loginu lub hasla\"}");
            std::string login=body["login"].s();
            std::string passowrd=body["password"].s();
            std::string hash=Crypto::hashPassword(passowrd);
            if (db.registerUser(login,hash))
                return crow::response(201,"{\"status\": \"Konto utworzone\"}");
            return crow::response(409,"{\"status\": \"Login jest juz zajety lub wystapil blad bazy danych\"}");
    });
    CROW_ROUTE(app, "/login").methods(crow::HTTPMethod::POST)
    ([&db](const crow::request& req) {
        auto body = crow::json::load(req.body);

        if (!body || !body.has("login") || !body.has("password"))
            return crow::response(400, "{\"error\": \"Brak loginu lub hasla\"}");

        std::string login = body["login"].s();
        std::string password = body["password"].s();

        std::string hash = db.getPasswordHashForUser(login);

        if (hash.empty())
            return crow::response(401, "{\"error\": \"Nieprawidlowy login lub haslo\"}");

        if (Crypto::verifyPassword(password, hash)) {
            std::string token = Crypto::generateToken(login);
            return crow::response(200, "{\"status\": \"Zalogowano pomyslnie!\", \"token\": \"" + token + "\"}");
        }
        return crow::response(401, "{\"error\": \"Nieprawidlowy login lub haslo\"}");
    });
    CROW_ROUTE(app,"/ping")([]()
    {
       return crow::response(200,"{\"status\": \"ok\"}");
    });


    CROW_ROUTE(app, "/verify_session").methods(crow::HTTPMethod::GET)
    ([](const crow::request& req) {
        auto authHeader = req.get_header_value("Authorization");
        if (authHeader.empty() || authHeader.substr(0, 7) != "Bearer ")
            return crow::response(401, "{\"error\": \"Brak autoryzacji\"}");

        std::string token = authHeader.substr(7);
        std::string login = Crypto::verifyToken(token);
        if (login.empty())
            return crow::response(401, "{\"error\": \"Nieprawidlowy lub wygasly token\"}");

        return crow::response(200, "{\"status\": \"zalogowano, " + login + "\"}");
    });

    CROW_ROUTE(app,"/vault").methods(crow::HTTPMethod::POST)([&db](const crow::request& req)
    {
        auto auth=req.get_header_value("Authorization");
        if (auth.empty()||auth.substr(0,7)!="Bearer ")
            return crow::response(401,"{\"error\": \"Brak autoryzacji\"}");
        std::string token=auth.substr(7);
        std::string login=Crypto::verifyToken(token);
        if (login.empty())
            return crow::response(401,"{\"error\": \"Nieprawidlowy login\"}");
        int userId=db.getUserId(login);
        if (userId==-1)
            return crow::response(404,"{\"error\": \"Nieznaleziono uzytkownika\"}");
        auto body=crow::json::load(req.body);
        if (!body || !body.has("service")|| !body.has("username")|| !body.has("password"))
            return crow::response(400, "{\"error\": \"Brak danych serwisu, uzytkownika lub hasla\"}");

        std::string service=body["service"].s();
        std::string username =body["username"].s();
        std::string password=body["password"].s();
        std::string encryptedPassword=Crypto::encrypt(password);
        if (db.addCredential(userId,service,username,encryptedPassword))
            return crow::response(201,"{\"status\": Haslo zapisane\"}");
        return crow::response(500,"{\"status\": Blad bazy danych przy zapisaniu\"}");

    });
    CROW_ROUTE(app,"/vault").methods(crow::HTTPMethod::GET)([&db](const crow::request& req)
    {
        auto authHeader = req.get_header_value("Authorization");
                if (authHeader.empty() || authHeader.substr(0, 7) != "Bearer ")
                    return crow::response(401, "{\"error\": \"Brak autoryzacji\"}");


                std::string token = authHeader.substr(7);
                std::string login = Crypto::verifyToken(token);

                if (login.empty())
                    return crow::response(401, "{\"error\": \"Nieprawidlowy lub wygasly token\"}");


                int userId = db.getUserId(login);
                if (userId == -1)
                    return crow::response(404, "{\"error\": \"Nie znaleziono uzytkownika\"}");


                auto creds = db.getCredentials(userId);

                crow::json::wvalue responseJson;
                std::vector<crow::json::wvalue> credsList;

                for (const auto& c : creds) {
                    crow::json::wvalue temp;
                    temp["id"] = c.getId();
                    temp["userId"] = c.getUserId();
                    temp["serviceName"] = c.getServiceName();
                    temp["loginName"] = c.getLoginName();
                    temp["password"] = Crypto::decrypt(c.getPassword());
                    temp["date"] = c.getDate();

                    credsList.push_back(std::move(temp));
                }

                responseJson["credentials"] = std::move(credsList);
                return crow::response(200, responseJson);
    });
    CROW_ROUTE(app,"/vault/<int>").methods(crow::HTTPMethod::DELETE)([&db](const crow::request &req,int credentialId)
    {
       auto authHeader=req.get_header_value("Authorization");
        if (authHeader.empty()||authHeader.substr(0,7)!= "Bearer ")
            return crow::response(401, "{\"error\": \"Nieprawidlowy lub wygasly token\"}");
        std::string token=authHeader.substr(7);
        std::string login=Crypto::verifyToken(token);
        if (login.empty())
                    return crow::response(401, "{\"error\": \"Nieprawidlowy lub wygasly token\"}");
        int userId = db.getUserId(login);
                if (userId == -1)
                    return crow::response(404, "{\"error\": \"Nie znaleziono uzytkownika\"}");
        if (db.deleteCredential(credentialId,userId))
            return crow::response(200, "{\"status\": \"Haslo usuniete\"}");
        return crow::response(404, "{\"error\": \"Nie znaleziono hasla lub brak uprawnien\"}");
    });
    //TODO
    //2 Zmiana hasla admina
    //3 Zmiana hasel w serwisie
    //4. LOGI
    //5 PRZENIESIENIE MAINA
    //6 Podmianka metod i uporzadkowanie kodu
    //7 Ogarniecia struktury plikow i przygotowanie do ogarniania klienta
    app.port((18080)).multithreaded().run();

    return 0;
}
