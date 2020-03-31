#pragma once
#include "IClientCmd.h"
#include "IServerCmd.h"

namespace Yhaniki {

    class ILanClient {
    public:

        virtual bool connected() const = 0;
        virtual bool connect(const CString& addy, unsigned short port) = 0;
        virtual void close() = 0;
        virtual bool IsError() = 0;

        virtual vector<unique_ptr<ServerCmd::IServerCmd>> ReceiveCmds() = 0;
        virtual bool SendCmd(unique_ptr<ClientCmd::IClientCmd> cmd) = 0;
    };
}
