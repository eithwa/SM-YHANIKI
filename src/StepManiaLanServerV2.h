#pragma once

#include <chrono>

#include "IStepManiaLanServer.h"
#include "CmdPortal.h"
#include "ezsockets.h"

namespace Yhaniki {

    class StepManiaLanServerV2 final : public IStepManiaLanServer {
        enum class State { On, Off };
    public:
        static unique_ptr<StepManiaLanServerV2> Default();

        bool ServerStart() override;
        void ServerStop() override;
        void ServerUpdate() override;

        StepManiaLanServerV2(
            const int portNo,
            unique_ptr<EzSockets>&& listenSocket,
            std::vector<unique_ptr<CmdPortal>>&& clients,
            const State state,
            const std::chrono::milliseconds lastInformTime,
            const std::chrono::milliseconds informTimeSpan
            );
    private:

        // Socket
        const int portNo_;
        const unique_ptr<EzSockets> listenSocket_;

        // Client 
        std::vector<unique_ptr<CmdPortal>> clients_{};

        // On / Off state
        State state_;

        // Periodic inform
        const std::chrono::milliseconds informTimeSpan_{};
        std::chrono::milliseconds lastInformTime_{};
    };
}
