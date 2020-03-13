#include <chrono>

#include "StepManiaLanServerV2.h"
#include "CmdPortal.h"

using namespace std::chrono;

namespace Reimplementation {

    StepManiaLanServerV2::StepManiaLanServerV2(
        const int portNo,
        EzSockets* listenSocket,
        std::vector<CmdPortal>&& clients,
        const State state,
        const std::chrono::milliseconds lastInformTime,
        const std::chrono::milliseconds informTimeSpan
        ) :
        portNo_(portNo),
        listenSocket_(listenSocket),
        clients_(clients),
        state_(state),
        informTimeSpan_(informTimeSpan),
        lastInformTime_(lastInformTime) {
    }

    StepManiaLanServerV2* StepManiaLanServerV2::Default() {
        return new StepManiaLanServerV2(
            8765,
            new EzSockets(),
            std::vector<CmdPortal>{},
            State::Off,
            duration_cast<milliseconds>(system_clock::now().time_since_epoch()),
            milliseconds(100)
            );
    }
    StepManiaLanServerV2::~StepManiaLanServerV2() {
        ServerStop();
        delete listenSocket_;
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
                std::remove_if(
                    clients_.begin(), clients_.end(),
                    [](auto&& client) { return client.IsError(); }
            ), clients_.end());
        }

        // Look for new clients
        {
            const auto newClient = CmdPortal::AcceptBy(listenSocket_);
            if (newClient != nullptr)
                clients_.push_back(move(*newClient));
        }

        // Process client requests and then respond to them
        {
            for (auto&& client : clients_) {
                // TODO: ...
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