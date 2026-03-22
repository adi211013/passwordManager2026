//
// Created by adi on 3/20/26.
//

#include "Credentials.h"

Credentials::Credentials(int iid, int iuserId, std::string sserviceName, std::string sloginName, std::string spassword,
                         std::string sDate)
    : id(iid)
      , userId(iuserId),
      serviceName(std::move(sserviceName)),
      loginName(std::move(sloginName)),
      password(std::move(spassword)),
      date(std::move(sDate))
{
}

int Credentials::getId() const { return id; }
int Credentials::getUserId() const { return userId; }
std::string Credentials::getServiceName() const { return serviceName; }
std::string Credentials::getLoginName() const { return loginName; }
std::string Credentials::getPassword() const { return password; }
std::string Credentials::getDate() const { return date; }
