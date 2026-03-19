//
// Created by adi on 3/19/26.
//

#include "DbManager.h"
DbManager::DbManager(const std::string& connectionString) : connStr(connectionString) {}
bool DbManager::checkConnection()
{
    try
    {
        pqxx::connection conn(connStr);
        if (conn.is_open())
        {
            std::cout<<"Polaczono z baza dany"<<conn.dbname()<<std::endl;
            return true;
        }
        else
        {
            std::cerr<<"Blad nie mozna utworzyc polaczenia z baza danych"<<std::endl;
            return false;
        }
    }catch (const std::exception& e)
    {
        std::cerr<<"Wyjatek: "<<e.what()<<std::endl;
        return false;
    }
}
