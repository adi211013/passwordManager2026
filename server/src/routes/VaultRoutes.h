//
// Created by adi on 3/22/26.
//

#pragma once
#include "crow/app.h"
#include "database/DbManager.h"


namespace VaultRoutes
{
    void setup(crow::SimpleApp& app, DbManager& db);
};
