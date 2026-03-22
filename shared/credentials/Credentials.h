//
// Created by adi on 3/20/26.
//

#pragma once
#include <string>


class Credentials
{
private:
    int id;
    int userId;
    std::string serviceName;
    std::string loginName;
    std::string password;
    std::string date;

public:
    Credentials(int iid, int iuserId, std::string sserviceName, std::string sloginName, std::string spassword,
                std::string sDate);
    int getId() const;
    int getUserId() const;
    std::string getServiceName() const;
    std::string getLoginName() const;
    std::string getPassword() const;
    std::string getDate() const;
};
