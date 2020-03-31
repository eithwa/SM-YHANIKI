#pragma once
#include <StdString.h>

namespace Yhaniki {

    class ILanServer {
    public:
        virtual bool ServerStart() = 0;
        virtual void ServerStop() = 0;
        virtual void ServerUpdate() = 0;
        virtual ~ILanServer() = default;

        CString* GetServerName() { return &serverName_; }
        void SetServerName(const CString& newName) { serverName_ = newName; }

        CString lastError = "";
        int lastErrorCode = 0;

    protected:
        CString serverName_ = "";
    };
}

