
#include "LanServerV2.Client.h"

#include "ClientCmds.hpp"
#include "ServerCmds.hpp"

namespace Yhaniki {

    using namespace ClientCmd;
    using namespace ServerCmd;


    LanServerV2::Client::Client(
        unique_ptr<CmdPortal<IClientCmd, IServerCmd>> portal) :
        portal_(move(portal)) {}

    bool LanServerV2::Client::HasSomethingToDo() {
        for (auto&& newCmd : portal_->ReceiveCmds())
            cmds_.push(move(newCmd));

        return !cmds_.empty();
    }

    bool LanServerV2::Client::Nothing() {
        if (cmds_.empty()) return true;
        return PopAs<ServerNop>() != nullptr;
    }

    unique_ptr<string> LanServerV2::Client::Chatted() {
        const auto cmd = PopAs<Chat>();
        if (cmd == nullptr) return nullptr;
        return make_unique<string>(cmd->GetMsg());
    }

    void LanServerV2::Client::SomeoneChat(const string& message) const {
        portal_->SendCmd(make_unique<SomeoneChatted>(message));
    }
}
