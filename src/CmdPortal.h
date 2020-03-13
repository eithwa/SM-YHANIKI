#pragma once

#include "ezsockets.h"

namespace Reimplementation {

    class CmdPortal {

    public:
        static CmdPortal* AcceptBy(EzSockets* const serverSocket);
        ~CmdPortal();

        template<class TCmd>
        TCmd* Receive();

        bool IsError() const;

    private:
        explicit CmdPortal(EzSockets* clientSocket);
        // Socket
        EzSockets* socket_;
    };
}
