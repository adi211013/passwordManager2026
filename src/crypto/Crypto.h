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
};


