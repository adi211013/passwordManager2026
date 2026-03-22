//
// Created by adi on 3/22/26.
//

#pragma once

#include "crow/app.h"
#include "database/DbManager.h"


namespace AuthRoutes
{
    void setup(crow::SimpleApp& app, DbManager& db);
};
