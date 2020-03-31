#include "LanServerV2.h"

#include <utility>

#include "LanServerV2.Client.h"

using namespace std::chrono;

namespace Yhaniki {

    LanServerV2::LanServerV2(
        const int portNo,
        unique_ptr<EzSockets> listenSocket,
        vector<Client> clients,
        const State state,
        const milliseconds lastInformTime,
        const milliseconds informTimeSpan
        ) :
        portNo_(portNo),
        listenSocket_(move(listenSocket)),
        clients_(move(clients)),
        state_(state),
        informTimeSpan_(informTimeSpan),
        lastInformTime_(lastInformTime) {}

    unique_ptr<LanServerV2> LanServerV2::Default() {
        return make_unique<LanServerV2>(
            8765,
            make_unique<EzSockets>(),
            vector<Client>{},
            State::Off,
            duration_cast<milliseconds>(system_clock::now().time_since_epoch()),
            milliseconds(100)
            );
    }

    bool LanServerV2::ServerStart() {

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

    void LanServerV2::ServerStop() {

        state_ = State::Off;

        clients_.clear();
        listenSocket_->close();
    }

    void LanServerV2::ServerUpdate() {

        // Return if server is off
        {
            if (state_ == State::Off) return;
        }

        // Remove error clients
        {
            clients_.erase(
                remove_if(
                    clients_.begin(), clients_.end(),
                    [](auto&& client) { return client.IsError(); }
            ), clients_.end());
        }

        // Look for new clients
        {
            auto portal = CmdPortal<IClientCmd, IServerCmd>::AcceptBy(listenSocket_);
            if (portal != nullptr)
                clients_.emplace_back(move(portal));
        }

        // Process client requests and then respond to them
        {
            for (auto&& client : clients_) {
                if (client.HasSomethingToDo())
                    ProcessClientRequest(client);
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

    void LanServerV2::ProcessClientRequest(Client& client) {

        if (client.Nothing()) 
            return;

        if (const auto message = client.Chatted()) {
            for (const auto& other : clients_)
                other.SomeoneChat("123 " + *message);
            return;
        }
    }
}