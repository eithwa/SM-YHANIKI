#pragma once

#include "CmdSerializationFactory.h"
#include "ezsockets.h"
#include "NetworkSyncServer.h"

namespace Yhaniki {

    template<class TIRecvCmd, class TISendCmd>
    class CmdPortal {

    public:
        static unique_ptr<CmdPortal> AcceptBy(const unique_ptr<EzSockets>& serverSocket) {

            auto clientSocket = make_unique<EzSockets>();
            clientSocket->blocking = false;

            if (serverSocket->accept(*clientSocket))
                return make_unique<CmdPortal>(move(clientSocket));
            else return nullptr;
        }

        vector<unique_ptr<TIRecvCmd>> ReceiveCmds() const {

            if (!socket_->CanRead())
                return {};

            vector<unique_ptr<TIRecvCmd>> cmds = {};
            char cStr[NETMAXBUFFERSIZE];

            while (socket_->ReadPack(cStr, NETMAXBUFFERSIZE) > 0) {
                cmds.push_back(
                    CmdSerializationFactory<TIRecvCmd>::Deserialize(
                        StringStack::FromString(string(cStr))));
            }

            return cmds;
        }

        bool SendCmd(unique_ptr<TISendCmd> cmd) const {

            if (!socket_->CanWrite())
                return false;

            auto strStack = CmdSerializationFactory<TISendCmd>::Serialize(cmd);
            socket_->SendPack(strStack.ToString().c_str());

            return true;
        }

        bool IsError() const {
            // TODO: Other errors?
            return socket_->IsError();
        }

        CmdPortal(
            unique_ptr<EzSockets> clientSocket) :
            socket_(move(clientSocket)) {}

    private:
        const unique_ptr<EzSockets> socket_;
    };
}
