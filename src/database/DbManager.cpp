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
int DbManager::getUserId(const std::string& login)
{
    try
    {
        pqxx::connection conn(connStr);
        pqxx::work txn(conn);
        pqxx::result res=txn.exec_params("SELECT id FROM users WHERE login =$1",login);
        if (res.empty())
            return -1;
        return res[0][0].as<int>();
    }catch (const std::exception& e)
    {
        std::cerr<<"blad pobierania id"<<e.what()<<std::endl;
        return -1;
    }
}
bool DbManager::addCredential(int userId,const std::string& service, const std::string& username,const std::string& pass)
{
    try
    {
        pqxx::connection conn(connStr);
        pqxx::work txn(conn);
        txn.exec_params(
                  "INSERT INTO credentials (user_id, service_name, login_name, encrypted_password) VALUES ($1, $2, $3, $4)",
                  userId, service, username, pass
              );
        txn.commit();
        return true;
    }catch (const std::exception& e)
    {
        std::cerr<<"Blad dodawania hasla"<<e.what()<<std::endl;
        return false;
    }
}
std::vector<Credentials> DbManager::getCredentials(int userId)
{
    std::vector<Credentials> credentials;
    try
    {
        pqxx::connection conn(connStr);
        pqxx::work txn(conn);
        pqxx::result result =txn.exec(
            "SELECT id,service_name,login_name,encrypted_password,updated_at FROM credentials where user_id=$1",
            pqxx::params{userId});

        for (auto const &row : result)
            {
                credentials.push_back({
                    row["id"].as<int>(),
                    userId,
                    row["service_name"].as<std::string>(),
                    row["login_name"].as<std::string>(),
                    row["encrypted_password"].as<std::string>(),
                    row["updated_at"].as<std::string>()
                });
            }
    }catch (const std::exception& e)
    {
        std::cerr<<"Blad przy pobieraniu hasel"<<e.what()<<std::endl;
    }
    return credentials;
}

bool DbManager::deleteCredential(int credentialId, int userId)
{
    try
    {
        pqxx::connection conn(connStr);
        pqxx::work txn(conn);
        pqxx::result res = txn.exec(
            "DELETE FROM credentials WHERE id=$1 AND user_id=$2",pqxx::params{credentialId,userId});
        txn.commit();
        return res.affected_rows()==1;
    }catch (const std::exception &e)
    {
        std::cerr<<"blad przy usuwaniu hasla"<<e.what()<<std::endl;
        return false;
    }
}
