//
// Created by adi on 3/19/26.
//

#pragma once
#include <pqxx/pqxx>
#include <string>
#include <iostream>

class DbManager
{
public:
    DbManager(const std::string & connectionString);
    bool checkConnection();
private:
    std::string connStr;
};


