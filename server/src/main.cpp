#include "crow.h"
#include <string>

#include "crypto/Crypto.h"
#include "database/DbManager.h"
#include "config/Config.h"
#include "routes/AuthRoutes.h"
#include "routes/LogRoutes.h"
#include "routes/VaultRoutes.h"
#include <thread>
#include <chrono>

int main()
{
    AppConfig config = Config::load();
    try
    {
        Crypto::init(config.jwtSecret, config.masterKey);
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    if (config.dbConnStr.empty())
    {
        std::cerr << "Brak connection stringa do bazy danych" << std::endl;
        return 1;
    }
    DbManager db(config.dbConnStr);

    int maxRetries = 5;
    while (!db.checkConnection() && maxRetries > 0)
    {
        std::cerr << "Brak polaczenia z baza. Czekam 2 sekundy... (pozostalo prob: " << maxRetries << ")" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));
        maxRetries--;
    }

    if (!db.checkConnection())
    {
        std::cerr << "OSTATECZNY BLAD: Nie udalo sie polaczyc z baza danych po kilku probach. Zamykam serwer." <<
            std::endl;
        return 1;
    }

    std::cout << "Polaczono z baza danych pomyslnie!" << std::endl;
    crow::SimpleApp app;
    AuthRoutes::setup(app, db);
    VaultRoutes::setup(app, db);
    LogRoutes::setup(app, db);
    app.port(config.port).multithreaded().run();

    return 0;
}
