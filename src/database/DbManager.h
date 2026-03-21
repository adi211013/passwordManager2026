//
// Created by adi on 3/19/26.
//

#pragma once
#include <pqxx/pqxx>
#include <string>
#include <iostream>
#include "../credentials/Credentials.h"
#include <vector>

class DbManager
{
public:
    DbManager(const std::string & connectionString);
    bool checkConnection();
    bool registerUser(const std::string& login, const std::string& password);
    std::string getPasswordHashForUser(const std::string& login);
    int getUserId(const std::string& login);
    bool addCredential(int userId,const std::string& service, const std::string& username,const std::string& pass);
    std::vector<Credentials> getCredentials(int userId);
    bool deleteCredential(int credentialId,int userId);
private:
    std::string connStr;
};


