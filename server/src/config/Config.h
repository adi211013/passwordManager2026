//
// Created by adi on 3/22/26.
//

#pragma once
#include <string>

struct AppConfig
{
    std::string dbConnStr;
    int port;
    std::string jwtSecret;
    std::string masterKey;
};

class Config
{
public:
    static AppConfig load(const std::string& envPath = "../../.env");
};
