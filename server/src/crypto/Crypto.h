//
// Created by adi on 3/19/26.
//

#pragma once
#include <string>


namespace Crypto
{
    void init(const std::string& jwtSecret, const std::string& masterKey);
    std::string hashPassword(const std::string& password);
    std::string generateToken(const std::string& login);
    bool verifyPassword(const std::string& password, const std::string& hash);
    std::string verifyToken(const std::string& token);
    std::string encrypt(const std::string& pass);
    std::string decrypt(const std::string& pass);
};
