
#include "CmdPortal.h"

namespace Reimplementation {

    CmdPortal::CmdPortal(
        EzSockets* clientSocket) {
        socket_ = clientSocket;
    }

    CmdPortal* CmdPortal::AcceptBy(EzSockets* const serverSocket) {
        const auto clientSocket = new EzSockets;
        clientSocket->blocking = false;
        if (serverSocket->accept(*clientSocket))
            return new CmdPortal(clientSocket);
        else return nullptr;
    }

    CmdPortal::~CmdPortal() {
        delete socket_;
    }

    template <class TCmd>
    TCmd* CmdPortal::Receive() {
        if (!socket_->CanRead())
            return nullptr;

        auto str = socket_->ReadStr();
    }

    bool CmdPortal::IsError() const {
        return socket_->IsError();
    }
}
