//
// Created by adi on 3/22/26.
//

#pragma once
struct AppConfig
{
    std::string dbConnStr;
    int port;
    std::string jwtSecret;
    std::string masterKey;
};