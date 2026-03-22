//
// Created by adi on 3/22/26.
//

#include "Config.h"

#include <fstream>
#include <iosfwd>
#include <iostream>
#include <unordered_map>
#include <cstdlib>


namespace Config
{
    AppConfig load(const std::string& envPath)
    {
        AppConfig config;
        std::unordered_map<std::string, std::string> envs;

        std::ifstream file(envPath);
        if (!file.is_open())
        {
            std::cout << "[INFO] Nie znaleziono pliku " << envPath << ". Polegam na zmiennych OS/Docker." << std::endl;
        }
        else
        {
            std::string line;
            while (std::getline(file, line))
            {
                if (line.empty() || line[0] == '#') continue;

                auto delimiterPos = line.find('=');
                if (delimiterPos != std::string::npos)
                {
                    std::string key = line.substr(0, delimiterPos);
                    std::string value = line.substr(delimiterPos + 1);
                    envs[key] = value;
                }
            }
        }

        auto getEnvValue = [&](const std::string& key) -> std::string
        {
            if (const char* sysEnv = std::getenv(key.c_str()))
            {
                return std::string(sysEnv);
            }
            if (envs.count(key))
            {
                return envs[key];
            }
            return "";
        };
        config.dbConnStr = getEnvValue("DB_CONN_STR");

        std::string portStr = getEnvValue("PORT");
        if (!portStr.empty())
        {
            config.port = std::stoi(portStr);
        }
        else
        {
            config.port = 18080;
        }

        config.jwtSecret = getEnvValue("JWT_SECRET");
        config.masterKey = getEnvValue("MASTER_KEY");

        return config;
    }
}
