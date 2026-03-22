//
// Created by adi on 3/22/26.
//

#include "AuthRoutes.h"

#include "crypto/Crypto.h"

namespace AuthRoutes
{
    void setup(crow::SimpleApp& app, DbManager& db)
    {
        CROW_ROUTE(app, "/register").methods(crow::HTTPMethod::POST)
        ([&db](const crow::request& req)
        {
            auto body = crow::json::load(req.body);
            if (!body || !body.has("login") || !body.has("password"))
            {
                crow::json::wvalue res;
                res["status"] = "Brak loginu lub hasla";
                return crow::response(400, res);
            }
            std::string login = body["login"].s();
            std::string password = body["password"].s();
            std::string hash = Crypto::hashPassword(password);
            if (db.registerUser(login, hash))
            {
                db.addLog(db.getUserId(login), "Konto utworzone");
                crow::json::wvalue res;
                res["status"] = "Konto utworzone";
                return crow::response(201, res);
            }
            crow::json::wvalue res;
            res["status"] = "Login jest juz zajety";
            return crow::response(409, res);
        });
        CROW_ROUTE(app, "/login").methods(crow::HTTPMethod::POST)
        ([&db](const crow::request& req)
        {
            auto body = crow::json::load(req.body);

            if (!body || !body.has("login") || !body.has("password"))
            {
                crow::json::wvalue res;
                res["status"] = "Brak loginu lub hasla";
                return crow::response(400, res);
            }

            std::string login = body["login"].s();
            std::string password = body["password"].s();

            std::string hash = db.getPasswordHashForUser(login);

            if (hash.empty())
            {
                crow::json::wvalue res;
                res["status"] = "Nieprawidlowy login lub haslo";
                return crow::response(401, res);
            }

            if (Crypto::verifyPassword(password, hash))
            {
                std::string token = Crypto::generateToken(login);
                db.addLog(db.getUserId(login), "Zalogowano sie na konto");
                crow::json::wvalue res;
                res["status"] = "Zalogowano pomyslnie";
                res["token"] = token;
                return crow::response(200, res);
            }
            crow::json::wvalue res;
            res["status"] = "Nieprawidlowy login lub haslo";
            return crow::response(401, res);
        });
    }
}
