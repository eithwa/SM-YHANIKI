#pragma once
#include "IClientCmd.h"

namespace Yhaniki {

    class ILanClient {
    public:
        virtual void close() = 0;
        virtual bool connect(const CString& addy, unsigned short port) = 0;
        virtual vector<unique_ptr<ClientCmd::IClientCmd>> ReceiveCmds() = 0;

    protected:
    };
}
