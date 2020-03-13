#pragma once
#include <StdString.h>

class IStepManiaLanServer {
public:
    virtual bool ServerStart() = 0;
    virtual void ServerStop() = 0;
    virtual void ServerUpdate() = 0;
    virtual ~IStepManiaLanServer() = default;

    CString GetServerName() const { return serverName_; }
    void SetServerName(const CString& newName) { serverName_ = newName; }

    CString lastError;
    int lastErrorCode;

protected:
    CString serverName_;
};

