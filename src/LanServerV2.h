#pragma once

#include <chrono>

#include "ILanServer.h"
#include "CmdPortal.hpp"
#include "IClientCmd.h"
#include "ezsockets.h"
#include "IServerCmd.h"

namespace Yhaniki {

    using ClientCmd::IClientCmd;
    using ServerCmd::IServerCmd;

    class LanServerV2 final : public ILanServer {
        enum class State { On, Off };
    public:
        class Client;
        static unique_ptr<LanServerV2> Default();

        bool ServerStart() override;
        void ServerStop() override;
        void ServerUpdate() override;

        LanServerV2(
            int portNo,
            unique_ptr<EzSockets> listenSocket,
            std::vector<Client> clients,
            State state,
            std::chrono::milliseconds lastInformTime,
            std::chrono::milliseconds informTimeSpan
            );

    private:
        void ProcessClientRequest(Client& client);

        // Socket
        const int portNo_;
        const unique_ptr<EzSockets> listenSocket_;

        // Client 
        std::vector<Client> clients_{};

        // On / Off state
        State state_;

        // Periodic inform setup
        const std::chrono::milliseconds informTimeSpan_{};
        std::chrono::milliseconds lastInformTime_{};
    };
}

#include "LanServerV2.Client.h"
