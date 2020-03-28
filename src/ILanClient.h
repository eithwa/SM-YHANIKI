#pragma once
#include "IClientCmd.h"
#include "IServerCmd.h"

namespace Yhaniki {

    class ILanClient {
    public:
        virtual void close() = 0;
        virtual bool connect(const CString& addy, unsigned short port) = 0;

        virtual vector<unique_ptr<ServerCmd::IServerCmd>> ReceiveCmds() = 0;
        virtual bool SendCmd(unique_ptr<ClientCmd::IClientCmd> cmd) = 0;
    };
}
