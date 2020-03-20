#pragma once

#include "CmdSerializationFactory.h"
#include "ezsockets.h"
#include "NetworkSyncServer.h"

namespace Yhaniki {

    class CmdPortal {

    public:
        static unique_ptr<CmdPortal> AcceptBy(const unique_ptr<EzSockets>& serverSocket) {

            auto clientSocket = make_unique<EzSockets>();
            clientSocket->blocking = false;

            if (serverSocket->accept(*clientSocket))
                return make_unique<CmdPortal>(move(clientSocket));
            else return nullptr;
        }

        template<class TICmd>
        unique_ptr<TICmd> Receive() const {

            if (!socket_->CanRead())
                return nullptr;

            char cStr[NETMAXBUFFERSIZE];
            socket_->ReadPack(cStr, NETMAXBUFFERSIZE);

            return CmdSerializationFactory<TICmd>::Deserialize(
                StringStack::FromString(string(cStr)));
        }

        template<class TICmd>
        bool Send(unique_ptr<TICmd> cmd) const {

            if (!socket_->CanWrite())
                return false;

            auto strStack = CmdSerializationFactory<TICmd>::Serialize(cmd);
            socket_->SendPack(strStack.ToString().c_str());

            return true;
        }

        bool IsError() const {
            // TODO: Other errors?
            return socket_->IsError();
        }

        CmdPortal(unique_ptr<EzSockets> clientSocket) :
            socket_(move(clientSocket)) {}

    private:
        const unique_ptr<EzSockets> socket_;
    };
}
