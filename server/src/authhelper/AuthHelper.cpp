//
// Created by adi on 3/22/26.
//

#include "AuthHelper.h"
#include "../crypto/Crypto.h"

AuthResult AuthHelper::verify(const crow::request& req, DbManager& db)
{
    auto authHeader = req.get_header_value("Authorization");
    if (authHeader.empty() || authHeader.substr(0, 7) != "Bearer ")
        return {-1, 401, "Brak autoryzacji"};

    std::string token = authHeader.substr(7);
    std::string login = Crypto::verifyToken(token);
    if (login.empty())
        return {-1, 401, "Nieprawidlowy lub wygasly token"};

    int userId = db.getUserId(login);
    if (userId == -1)
        return {-1, 404, "Nie znaleziono uzytkownika"};

    return {userId, 200, login, ""};
}
