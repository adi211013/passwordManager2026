#include "VaultRoutes.h"
#include "authhelper/AuthHelper.h"
#include "crypto/Crypto.h"

namespace VaultRoutes
{
    void setup(crow::SimpleApp& app, DbManager& db)
    {
        CROW_ROUTE(app, "/vault").methods(crow::HTTPMethod::POST)([&db](const crow::request& req)
        {
            auto auth = AuthHelper::verify(req, db);
            if (auth.userId == -1)
            {
                crow::json::wvalue res;
                res["status"] = auth.errorMessage;
                return crow::response(auth.errorCode, res);
            }
            int userId = auth.userId;
            auto body = crow::json::load(req.body);
            if (!body || !body.has("service") || !body.has("username") || !body.has("password"))
            {
                crow::json::wvalue res;
                res["status"] = "Brak danych serwisu, uzytkownika lub hasla";
                return crow::response(400, res);
            }

            std::string username;
            std::string service;
            std::string password;
            try
            {
                service = body["service"].s();
                username = body["username"].s();
                password = body["password"].s();
            }
            catch (std::exception& e)
            {
                crow::json::wvalue res;
                res["status"] = "Blad podczas pobierania loginu lub hasla";
                return crow::response(400, res);
            }
            if (service.empty() || service.length() > 96 || username.empty() || username.length() > 64 || password.
                length() < 8 || password.length() > 64)
            {
                crow::json::wvalue res;

                res["status"] = "Nazwa serwisu lub haslo lub login nie spelniaja wymagan";
                return crow::response(400, res);
            }
            std::string encryptedPassword = Crypto::encrypt(password);
            if (db.addCredential(userId, service, username, encryptedPassword))
            {
                db.addLog(userId, "Dodano haslo w serwisie " + service);
                crow::json::wvalue res;
                res["status"] = "Haslo zapisane";
                return crow::response(201, res);
            }
            crow::json::wvalue res;
            res["status"] = "Blad bazy danych przy zapisaniu";
            return crow::response(500, res);
        });
        CROW_ROUTE(app, "/vault").methods(crow::HTTPMethod::GET)([&db](const crow::request& req)
        {
            auto auth = AuthHelper::verify(req, db);
            if (auth.userId == -1)
            {
                crow::json::wvalue res;
                res["status"] = auth.errorMessage;
                return crow::response(auth.errorCode, res);
            }
            int userId = auth.userId;


            auto creds = db.getCredentials(userId);

            crow::json::wvalue responseJson;
            std::vector<crow::json::wvalue> credsList;

            for (const auto& c : creds)
            {
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
        CROW_ROUTE(app, "/vault/<int>").methods(crow::HTTPMethod::DELETE)(
            [&db](const crow::request& req, int credentialId)
            {
                auto auth = AuthHelper::verify(req, db);
                if (auth.userId == -1)
                {
                    crow::json::wvalue res;
                    res["status"] = auth.errorMessage;
                    return crow::response(auth.errorCode, res);
                }
                int userId = auth.userId;
                if (db.deleteCredential(credentialId, userId))
                {
                    db.addLog(userId, "Haslo usuniete ");
                    crow::json::wvalue res;
                    res["status"] = "Haslo usuniete";
                    return crow::response(200, res);
                }
                crow::json::wvalue res;
                res["status"] = "Nie znaleziono hasla lub brak uprawnien";
                return crow::response(404, res);
            });
        CROW_ROUTE(app, "/vault/<int>").methods(crow::HTTPMethod::PUT)([&db](const crow::request& req, int credentialId)
        {
            auto auth = AuthHelper::verify(req, db);
            if (auth.userId == -1)
            {
                crow::json::wvalue res;
                res["status"] = auth.errorMessage;
                return crow::response(auth.errorCode, res);
            }
            int userId = auth.userId;
            auto body = crow::json::load(req.body);
            if (!body || !body.has("service") || !body.has("username") || !body.has("password"))
            {
                crow::json::wvalue res;
                res["status"] = "Nieprawidlowy JSON";
                return crow::response(400, res);
            }

            std::string username;
            std::string service;
            std::string password;
            try
            {
                service = body["service"].s();
                username = body["username"].s();
                password = body["password"].s();
            }
            catch (std::exception& e)
            {
                crow::json::wvalue res;
                res["status"] = "Blad podczas pobierania loginu lub hasla";
                return crow::response(400, res);
            }
            if (service.empty() || service.length() > 96 || username.empty() || username.length() > 64 || password.
                length() < 8 || password.length() > 64)
            {
                crow::json::wvalue res;

                res["status"] = "Nazwa serwisu lub haslo lub login nie spelniaja wymagan";
                return crow::response(400, res);
            }
            std::string encryptedPassword = Crypto::encrypt(password);

            if (db.updateCredential(credentialId, userId, service, username, encryptedPassword))
            {
                db.addLog(userId, "Zmieniono dane w serwisie " + service);
                crow::json::wvalue res;
                res["status"] = "Wpis zaktualizowany";
                return crow::response(200, res);
            }
            crow::json::wvalue res;
            res["status"] = "Nie znaleziono wpisu lub brak uprawnien";
            return crow::response(404, res);
        });

        CROW_ROUTE(app, "/user/password").methods(crow::HTTPMethod::PUT)([&db](const crow::request& req)
        {
            auto auth = AuthHelper::verify(req, db);
            if (auth.userId == -1)
            {
                crow::json::wvalue res;
                res["status"] = auth.errorMessage;
                return crow::response(auth.errorCode, res);
            }
            int userId = auth.userId;
            std::string login = auth.login;
            auto body = crow::json::load(req.body);
            if (!body || !body.has("old_password") || !body.has("new_password"))
            {
                crow::json::wvalue res;
                res["status"] = "Nieprawidlowy JSON";
                return crow::response(400, res);
            }
            std::string oldPassword;
            std::string newPassword;

            try
            {
                oldPassword = body["old_password"].s();
                newPassword = body["new_password"].s();
            }
            catch (std::exception& e)
            {
                crow::json::wvalue res;
                res["status"] = "Blad podczas pobierania loginu lub hasla";
                return crow::response(400, res);
            }
            if (newPassword.length() < 8 || newPassword.length() > 96)
            {
                crow::json::wvalue res;
                res["status"] = "Haslo nie spelniaja wymagan";
                return crow::response(400, res);
            }
            std::string currentHash = db.getPasswordHashForUser(login);
            if (currentHash.empty() || !Crypto::verifyPassword(oldPassword, currentHash))
            {
                crow::json::wvalue res;
                res["status"] = "Stare haslo jest nieprawidlowe";
                return crow::response(401, res);
            }

            std::string newHash = Crypto::hashPassword(newPassword);
            if (db.updateMasterPassword(userId, newHash))
            {
                db.addLog(userId, "Zmieniono haslo");
                crow::json::wvalue res;
                res["status"] = "Haslo glowne zostalo zmienione";
                return crow::response(200, res);
            }
            crow::json::wvalue res;
            res["status"] = "Blad serwera podczas zmiany hasla";
            return crow::response(500, res);
        });
    }
}
