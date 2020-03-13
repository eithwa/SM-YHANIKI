#include "StepManiaLanServerV2.h"

#include <utility>

using namespace std::chrono;

namespace Yhaniki {

    StepManiaLanServerV2::StepManiaLanServerV2(
        const int portNo,
        unique_ptr<EzSockets>&& listenSocket,
        vector<unique_ptr<CmdPortal>>&& clients,
        const State state,
        const milliseconds lastInformTime,
        const milliseconds informTimeSpan
        ) :
        portNo_(portNo),
        listenSocket_(move(listenSocket)),
        clients_(move(clients)),
        state_(state),
        informTimeSpan_(informTimeSpan),
        lastInformTime_(lastInformTime) {
    }

    unique_ptr<StepManiaLanServerV2> StepManiaLanServerV2::Default() {
        return make_unique<StepManiaLanServerV2>(
            8765,
            make_unique<EzSockets>(),
            vector<unique_ptr<CmdPortal>>{},
            State::Off,
            duration_cast<milliseconds>(system_clock::now().time_since_epoch()),
            milliseconds(100)
        );
    }

    bool StepManiaLanServerV2::ServerStart() {

        listenSocket_->blocking = false;

        if (!listenSocket_->create()) {
            lastError = "Failed to create socket";
            lastErrorCode = listenSocket_->lastCode;
            return false;
        }

        if (!listenSocket_->bind(portNo_)) {
            lastError = "Failed to bind socket";
            lastErrorCode = listenSocket_->lastCode;
            return false;
        }

        if (!listenSocket_->listen()) {
            lastError = "Failed to make socket listen.";
            lastErrorCode = listenSocket_->lastCode;
            return false;
        }

        state_ = State::On;

        return true;
    }

    void StepManiaLanServerV2::ServerStop() {

        state_ = State::Off;

        clients_.clear();
        listenSocket_->close();
    }

    void StepManiaLanServerV2::ServerUpdate() {

        // Return if server is off
        {
            if (state_ == State::Off) return;
        }

        // Remove error clients
        {
            clients_.erase(
                remove_if(
                    clients_.begin(), clients_.end(),
                    [](auto&& client) { return client->IsError(); }
            ), clients_.end());
        }

        // Look for new clients
        {
            auto portal = CmdPortal::AcceptBy(listenSocket_);
            if (portal != nullptr)
                clients_.push_back(move(portal));
        }

        // Process client requests and then respond to them
        {
            for (auto&& client : clients_) {
                if (client->Receive() == nullptr)
                    continue;
            }
        }

        // Send infos
        {
            const auto durationNow =
                duration_cast<milliseconds>(
                    system_clock::now().time_since_epoch());

            if (durationNow > lastInformTime_ + informTimeSpan_) {
                lastInformTime_ = durationNow;
                // TODO: ...
            }
        }
    }
}