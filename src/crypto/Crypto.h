//
// Created by adi on 3/19/26.
//

#pragma once
#include <string>


class Crypto
{
public:
    static void init();
    static std::string hashPassword(const std::string& password);
    static std::string generateToken(const std::string& login);
    static bool verifyPassword(const std::string& password, const std::string& hash);
    static std::string verifyToken(const std::string& token);
    static std::string encrypt(const std::string& pass);
    static std::string decrypt(const std::string& pass);
};


