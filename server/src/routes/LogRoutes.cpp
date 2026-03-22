//
// Created by adi on 3/22/26.
//

#include "LogRoutes.h"

#include "authhelper/AuthHelper.h"

namespace LogRoutes
{
    void setup(crow::SimpleApp& app, DbManager& db)
    {
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
    }
};
