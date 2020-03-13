
#include "CmdPortal.h"

#include "NetworkSyncServer.h"

namespace Yhaniki {

    CmdPortal::CmdPortal(
        unique_ptr<EzSockets>&& clientSocket
        ) :
        socket_(move(clientSocket)) {}

    unique_ptr<CmdPortal> CmdPortal::AcceptBy(const unique_ptr<EzSockets>& serverSocket) {
        auto clientSocket = make_unique<EzSockets>();
        clientSocket->blocking = false;
        if (serverSocket->accept(*clientSocket))
            return make_unique<CmdPortal>(move(clientSocket));
        else 
            return nullptr;
    }

    void* CmdPortal::Receive() const {
        if (!socket_->CanRead())
            return nullptr;

        char str[NETMAXBUFFERSIZE];
        socket_->ReadPack(str, NETMAXBUFFERSIZE);
        if (strcmp(str, "") == 0)
            int a = 1;
        return nullptr;
    }

    bool CmdPortal::IsError() const {
        return socket_->IsError();
    }
}
