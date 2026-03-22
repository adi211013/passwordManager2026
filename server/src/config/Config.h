//
// Created by adi on 3/22/26.
//

#pragma once
#include <string>

#include "AppConfig.h"


namespace  Config
{
     AppConfig load(const std::string& envPath = "../../.env");
};
