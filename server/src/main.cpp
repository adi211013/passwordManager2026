#include "crow.h"
#include <string>

#include "crypto/Crypto.h"
#include "database/DbManager.h"
#include "authhelper/AuthHelper.h"
#include "config/Config.h"

int main()
{
    AppConfig config = Config::load();
    try
    {
        Crypto::init(config.jwtSecret, config.masterKey);
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    if (config.dbConnStr.empty())
    {
        std::cerr << "Brak connection stringa do bazy danych" << std::endl;
    }
    DbManager db(config.dbConnStr);
    if (!db.checkConnection())
    {
        std::cerr << "Brak polaczenia z baza danych" << std::endl;
        return 1;
    }
    crow::SimpleApp app;
    CROW_ROUTE(app, "/register").methods(crow::HTTPMethod::POST)
    ([&db](const crow::request& req)
    {
        auto body = crow::json::load(req.body);
        if (!body || !body.has("login") || !body.has("password"))
            return crow::response(400, "{\"error\": \"Brakk loginu lub hasla\"}");
        std::string login = body["login"].s();
        std::string password = body["password"].s();
        std::string hash = Crypto::hashPassword(password);
        if (db.registerUser(login, hash))
        {
            db.addLog(db.getUserId(login), "Konto utworzone");
            return crow::response(201, "{\"status\": \"Konto utworzone\"}");
        }
        return crow::response(409, "{\"status\": \"Login jest juz zajety lub wystapil blad bazy danych\"}");
    });
    CROW_ROUTE(app, "/login").methods(crow::HTTPMethod::POST)
    ([&db](const crow::request& req)
    {
        auto body = crow::json::load(req.body);

        if (!body || !body.has("login") || !body.has("password"))
            return crow::response(400, "{\"error\": \"Brak loginu lub hasla\"}");

        std::string login = body["login"].s();
        std::string password = body["password"].s();

        std::string hash = db.getPasswordHashForUser(login);

        if (hash.empty())
            return crow::response(401, "{\"error\": \"Nieprawidlowy login lub haslo\"}");

        if (Crypto::verifyPassword(password, hash))
        {
            std::string token = Crypto::generateToken(login);
            db.addLog(db.getUserId(login), "Zalogowano sie na konto");
            return crow::response(200, "{\"status\": \"Zalogowano pomyslnie!\", \"token\": \"" + token + "\"}");
        }
        return crow::response(401, "{\"error\": \"Nieprawidlowy login lub haslo\"}");
    });

    CROW_ROUTE(app, "/vault").methods(crow::HTTPMethod::POST)([&db](const crow::request& req)
    {
        auto auth = AuthHelper::verify(req, db);
        if (auth.userId == -1)
            return crow::response(auth.errorCode, auth.errorMessage);
        int userId = auth.userId;
        auto body = crow::json::load(req.body);
        if (!body || !body.has("service") || !body.has("username") || !body.has("password"))
            return crow::response(400, "{\"error\": \"Brak danych serwisu, uzytkownika lub hasla\"}");

        std::string service = body["service"].s();
        std::string username = body["username"].s();
        std::string password = body["password"].s();
        std::string encryptedPassword = Crypto::encrypt(password);
        if (db.addCredential(userId, service, username, encryptedPassword))
        {
            db.addLog(userId, "Dodano haslo w seriwisie " + service);
            return crow::response(201, "{\"status\": Haslo zapisane\"}");
        }
        return crow::response(500, "{\"status\": Blad bazy danych przy zapisaniu\"}");
    });
    CROW_ROUTE(app, "/vault").methods(crow::HTTPMethod::GET)([&db](const crow::request& req)
    {
        auto auth = AuthHelper::verify(req, db);
        if (auth.userId == -1)
            return crow::response(auth.errorCode, auth.errorMessage);
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
    CROW_ROUTE(app, "/vault/<int>").methods(crow::HTTPMethod::DELETE)([&db](const crow::request& req, int credentialId)
    {
        auto auth = AuthHelper::verify(req, db);
        if (auth.userId == -1)
            return crow::response(auth.errorCode, auth.errorMessage);
        int userId = auth.userId;
        if (db.deleteCredential(credentialId, userId))
        {
            db.addLog(userId, "Usunieto dane w serwisie ");
            return crow::response(200, "{\"status\": \"Haslo usuniete\"}");
        }
        return crow::response(404, "{\"error\": \"Nie znaleziono hasla lub brak uprawnien\"}");
    });
    CROW_ROUTE(app, "/vault/<int>").methods(crow::HTTPMethod::PUT)([&db](const crow::request& req, int credentialId)
    {
        auto auth = AuthHelper::verify(req, db);
        if (auth.userId == -1)
            return crow::response(auth.errorCode, auth.errorMessage);
        int userId = auth.userId;
        auto body = crow::json::load(req.body);
        if (!body || !body.has("service") || !body.has("username") || !body.has("password"))
            return crow::response(400, "{\"error\": \"Nieprawidlowy JSON\"}");

        std::string service = body["service"].s();
        std::string username = body["username"].s();
        std::string password = body["password"].s();

        std::string encryptedPassword = Crypto::encrypt(password);

        if (db.updateCredential(credentialId, userId, service, username, encryptedPassword))
        {
            db.addLog(userId, "Zmieniono dane w serwisie " + service);
            return crow::response(200, "{\"status\": \"Wpis zaktualizowany\"}");
        }

        return crow::response(404, "{\"error\": \"Nie znaleziono wpisu lub brak uprawnien\"}");
    });

    CROW_ROUTE(app, "/user/password").methods(crow::HTTPMethod::PUT)([&db](const crow::request& req)
    {
        auto auth = AuthHelper::verify(req, db);
        if (auth.userId == -1)
            return crow::response(auth.errorCode, auth.errorMessage);
        int userId = auth.userId;
        std::string login = auth.login;
        auto body = crow::json::load(req.body);
        if (!body || !body.has("old_password") || !body.has("new_password"))
            return crow::response(400, "{\"error\": \"Nieprawidlowy JSON\"}");

        std::string oldPassword = body["old_password"].s();
        std::string newPassword = body["new_password"].s();

        std::string currentHash = db.getPasswordHashForUser(login);
        if (currentHash.empty() || !Crypto::verifyPassword(oldPassword, currentHash))
            return crow::response(401, "{\"error\": \"Stare haslo jest nieprawidlowe\"}");

        std::string newHash = Crypto::hashPassword(newPassword);
        if (db.updateMasterPassword(userId, newHash))
        {
            db.addLog(userId, "Zmieniono haslo");
            return crow::response(200, "{\"status\": \"Haslo glowne zostalo zmienione\"}");
        }

        return crow::response(500, "{\"error\": \"Blad serwera podczas zmiany hasla\"}");
    });
    CROW_ROUTE(app, "/logs").methods(crow::HTTPMethod::GET)([&db](const crow::request& req)
    {
        auto auth = AuthHelper::verify(req, db);
        if (auth.userId == -1)
            return crow::response(auth.errorCode, auth.errorMessage);
        int userId = auth.userId;
        auto logs = db.getLogs(userId);

        crow::json::wvalue responseJson;
        std::vector<crow::json::wvalue> logsList;

        for (const auto& l : logs)
        {
            crow::json::wvalue temp;
            temp["id"] = l.id;
            temp["description"] = l.description;
            temp["date"] = l.date;
            logsList.push_back(std::move(temp));
        }

        responseJson["logs"] = std::move(logsList);
        return crow::response(200, responseJson);
    });
    //TODO
    //1 PRZENIESIENIE MAINA
    //2 oczyszczenie kodu
    app.port(config.port).multithreaded().run();

    return 0;
}
