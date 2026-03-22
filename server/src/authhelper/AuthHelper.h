//
// Created by adi on 3/22/26.
//

#pragma once
#include "crow.h"
#include "../database/DbManager.h"

struct AuthResult
{
    int userId;
    int errorCode;
    std::string login;
    std::string errorMessage;
};

class AuthHelper
{
public:
    static AuthResult verify(const crow::request& req, DbManager& db);
};
