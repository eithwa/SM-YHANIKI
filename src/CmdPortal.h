#pragma once

#include "ezsockets.h"

namespace Yhaniki {

    class CmdPortal {

    public:
        static unique_ptr<CmdPortal> AcceptBy(const unique_ptr<EzSockets>& serverSocket);

        void* Receive() const;
        bool IsError() const;

        CmdPortal(unique_ptr<EzSockets>&& clientSocket);
    private:

        const unique_ptr<EzSockets> socket_;
    };
}
