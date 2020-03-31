#pragma once

#include <queue>

#include "As.hpp"
#include "LanServerV2.h"

namespace Yhaniki {

    class LanServerV2::Client {

    public:
        bool IsError() const { return portal_->IsError(); }

        bool HasSomethingToDo();

        // Check Cmds
        bool Nothing();
        unique_ptr<string> Chatted();

        // Send Cmds
        void SomeoneChat(const string& message) const;

        Client(unique_ptr<CmdPortal<IClientCmd, IServerCmd>> portal);
    private:

        template<class TCmd>
        unique_ptr<TCmd> PopAs() {
            if (cmds_.empty())
                return nullptr;

            auto cmdTry = As<TCmd>(cmds_.front());
            if (cmdTry == nullptr) 
                return nullptr;

            auto cmd = ReleaseAs<TCmd>(cmds_.front());
            cmds_.pop();

            return move(cmd);
        }

        unique_ptr<CmdPortal<IClientCmd, IServerCmd>> portal_;
        queue<unique_ptr<IClientCmd>> cmds_;
    };
}

