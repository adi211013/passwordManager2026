//
// Created by adi on 3/19/26.
//

#include "DbManager.h"

DbManager::DbManager(const std::string& connectionString) : connStr(connectionString)
{
}

bool DbManager::checkConnection()
{
    try
    {
        pqxx::connection conn(connStr);
        if (conn.is_open())
        {
            std::cout << "Polaczono z baza dany" << conn.dbname() << std::endl;
            return true;
        }
        std::cerr << "Blad nie mozna utworzyc polaczenia z baza danych" << std::endl;
        return false;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Wyjatek: " << e.what() << std::endl;
        return false;
    }
}

bool DbManager::registerUser(const std::string& login, const std::string& password)
{
    try
    {
        pqxx::connection conn(connStr);
        pqxx::work txn(conn);
        txn.exec("INSERT INTO users (login,password) VALUES ($1,$2)", pqxx::params{login, password});
        txn.commit();
        return true;
    }
    catch (const std::exception& e)
    {
        std::cerr << "blad przy rejestracji" << e.what() << std::endl;
        return false;
    }
}

std::string DbManager::getPasswordHashForUser(const std::string& login)
{
    try
    {
        pqxx::connection conn(connStr);
        pqxx::work txn(conn);
        pqxx::result res = txn.exec(
            "SELECT password FROM users WHERE login = $1",
            pqxx::params{login}
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
        pqxx::result res = txn.exec("SELECT id FROM users WHERE login =$1", pqxx::params{login});
        if (res.empty())
            return -1;
        return res[0][0].as<int>();
    }
    catch (const std::exception& e)
    {
        std::cerr << "blad pobierania id" << e.what() << std::endl;
        return -1;
    }
}

bool DbManager::addCredential(int userId, const std::string& service, const std::string& username,
                              const std::string& pass)
{
    try
    {
        pqxx::connection conn(connStr);
        pqxx::work txn(conn);
        txn.exec(
            "INSERT INTO credentials (user_id, service_name, login_name, encrypted_password) VALUES ($1, $2, $3, $4)",
            pqxx::params{userId, service, username, pass}
        );
        txn.commit();
        return true;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Blad dodawania hasla" << e.what() << std::endl;
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
        pqxx::result result = txn.exec(
            "SELECT id,service_name,login_name,encrypted_password,updated_at FROM credentials where user_id=$1",
            pqxx::params{userId});

        for (auto const& row : result)
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
    }
    catch (const std::exception& e)
    {
        std::cerr << "Blad przy pobieraniu hasel" << e.what() << std::endl;
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
            "DELETE FROM credentials WHERE id=$1 AND user_id=$2", pqxx::params{credentialId, userId});
        txn.commit();
        return res.affected_rows() == 1;
    }
    catch (const std::exception& e)
    {
        std::cerr << "blad przy usuwaniu hasla" << e.what() << std::endl;
        return false;
    }
}

bool DbManager::updateCredential(int credentialId, int userId, const std::string& service, const std::string& username,
                                 const std::string& encryptedPassword)
{
    try
    {
        pqxx::connection conn(connStr);
        pqxx::work txn(conn);
        pqxx::result res = txn.exec(
            "UPDATE credentials SET service_name = $1, login_name = $2, encrypted_password = $3, updated_at = CURRENT_TIMESTAMP WHERE id = $4 AND user_id = $5",
            pqxx::params{service, username, encryptedPassword, credentialId, userId}
        );
        txn.commit();
        return res.affected_rows() == 1;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Blad przy aktualizacji wpisu" << e.what() << std::endl;
        return false;
    }
}

bool DbManager::updateMasterPassword(int userId, const std::string& newPasswordHash)
{
    try
    {
        pqxx::connection conn(connStr);
        pqxx::work txn(conn);
        pqxx::result res = txn.exec("UPDATE users SET password = $1 WHERE id= $2",
                                    pqxx::params{newPasswordHash, userId});
        txn.commit();
        return res.affected_rows() == 1;
    }
    catch (const std::exception& e)
    {
        std::cerr << "blad przy zmiania hasla" << e.what() << std::endl;
        return false;
    }
}

void DbManager::addLog(int userId, const std::string& description)
{
    try
    {
        pqxx::connection conn(connStr);
        pqxx::work txn(conn);
        txn.exec(
            "INSERT INTO logs (user_id,description) VALUES ($1,$2", pqxx::params{userId, description});
        txn.commit();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Blad przy dodawaniu logu" << e.what() << std::endl;
    }
}

std::vector<LogEntry> DbManager::getLogs(int userId)
{
    std::vector<LogEntry> logs;
    try
    {
        pqxx::connection conn(connStr);
        pqxx::work txn(conn);
        pqxx::result res = txn.exec("SELECT * FROM logs WHERE user_id=$1 ORDER BY created_at DESC",
                                    pqxx::params{userId});
        for (auto const& row : res)
        {
            logs.push_back({
                row["id"].as<int>(),
                row["description"].as<std::string>(),
                row["created_at"].as<std::string>()
            });
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Blad przy pobieraniu logow" << e.what() << std::endl;
    }
    return logs;
}
