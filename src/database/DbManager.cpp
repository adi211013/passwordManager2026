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
        std::cerr<<"Blad nie mozna utworzyc polaczenia z baza danych"<<std::endl;
        return false;
    }catch (const std::exception& e)
    {
        std::cerr<<"Wyjatek: "<<e.what()<<std::endl;
        return false;
    }
}

bool DbManager::registerUser(const std::string& login, const std::string& password)
{
    try
    {
        pqxx::connection conn(connStr);
        pqxx::work txn(conn);
        txn.exec_params("INSERT INTO users (login,password) VALUES ($1,$2)",login,password);
        txn.commit();
        return true;
    }catch (const std::exception&e)
    {
        std::cerr<<"blad przy rejestracji"<<e.what()<<std::endl;
        return false;
    }
}
std::string DbManager::getPasswordHashForUser(const std::string& login)
{
    try
    {
        pqxx::connection conn(connStr);
        pqxx::work txn(conn);
        pqxx::result res = txn.exec_params(
            "SELECT password FROM users WHERE login = $1",
            login
        );
        if (res.empty())
            return "";
        return res[0][0].as<std::string>();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Blad przy pobieraniu hasla: " << e.what() << std::endl;
        return "";
    }
}