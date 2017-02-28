// -------------------------------------------------------------------------
//    @FileName         ��    NFILoginNet_ServerModule.h
//    @Author           ��    Ark Game Tech
//    @Date             ��    2012-12-15
//    @Module           ��    NFILoginNet_ServerModule
//
// -------------------------------------------------------------------------

#ifndef NFI_LOGINLOGIC_MODULE_H
#define NFI_LOGINLOGIC_MODULE_H

#include <iostream>
#include "NFIModule.h"

class NFILoginLogicModule
    : public NFIModule
{
public:
    virtual int OnLoginProcess(const NFGUID& object, const std::string& strAccount, const std::string& strPwd) = 0;

};

#endif