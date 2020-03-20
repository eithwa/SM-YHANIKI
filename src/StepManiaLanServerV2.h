#pragma once

#include <chrono>

#include "IStepManiaLanServer.h"
#include "CmdPortal.hpp"
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
            int portNo,
            unique_ptr<EzSockets> listenSocket,
            std::vector<unique_ptr<CmdPortal>> clients,
            State state,
            std::chrono::milliseconds lastInformTime,
            std::chrono::milliseconds informTimeSpan
            );
    private:

        // Socket
        const int portNo_;
        const unique_ptr<EzSockets> listenSocket_;

        // Client 
        std::vector<unique_ptr<CmdPortal>> clients_{};

        // On / Off state
        State state_;

        // Periodic inform setup
        const std::chrono::milliseconds informTimeSpan_{};
        std::chrono::milliseconds lastInformTime_{};
    };
}
