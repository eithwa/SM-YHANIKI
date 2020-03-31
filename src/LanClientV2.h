#pragma once
#include <memory>

#include "CmdPortal.hpp"
#include "ILanClient.h"
#include "IServerCmd.h"

namespace Yhaniki {

    using ClientCmd::IClientCmd;
    using ServerCmd::IServerCmd;

    class LanClientV2 final : public ILanClient {
    public:
        static unique_ptr<LanClientV2> New();

        bool connected() const override;
        bool connect(const CString& addy, unsigned short port) override;
        void close() override;
        bool IsError() override;

        vector<unique_ptr<IServerCmd>> ReceiveCmds() override;
        bool SendCmd(unique_ptr<IClientCmd> cmd) override;

        LanClientV2(unique_ptr<CmdPortal<IServerCmd, IClientCmd>> portal);

    private:
        unique_ptr<CmdPortal<IServerCmd, IClientCmd>> portal_;
    };
}
