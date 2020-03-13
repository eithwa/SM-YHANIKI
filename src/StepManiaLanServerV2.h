#pragma once

#include <chrono>

#include "IStepManiaLanServer.h"
#include "ezsockets.h"

namespace Reimplementation {

    class CmdPortal;

    class StepManiaLanServerV2 final : public IStepManiaLanServer {
        enum class State { On, Off };
    public:
        static StepManiaLanServerV2* Default();
        ~StepManiaLanServerV2();
        //StepManiaLanServerV2(const StepManiaLanServerV2&) = delete;

        bool ServerStart() override;
        void ServerStop() override;
        void ServerUpdate() override;

    private:
        StepManiaLanServerV2(
            const int portNo,
            EzSockets* listenSocket,
            std::vector<CmdPortal>&& clients,
            const State state,
            const std::chrono::milliseconds lastInformTime,
            const std::chrono::milliseconds informTimeSpan
            );

        // Socket
        const int portNo_;
        EzSockets* listenSocket_;

        // Client 
        std::vector<CmdPortal> clients_{};

        // On / Off state
        State state_;

        // Periodic inform
        const std::chrono::milliseconds informTimeSpan_{};
        std::chrono::milliseconds lastInformTime_{};
    };
}
