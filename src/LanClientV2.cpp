

#include "LanClientV2.h"

#include <memory>

namespace Yhaniki {

    LanClientV2::LanClientV2(
        unique_ptr<CmdPortal<IServerCmd, IClientCmd>> portal) :
        portal_(move(portal))
    {}

    std::unique_ptr<LanClientV2> LanClientV2::New() {
        return make_unique<LanClientV2>(nullptr);
    }

    bool LanClientV2::connected() const {
        if (portal_ == nullptr)
            return false;
        if (portal_->IsError())
            return false;
        return true;
    }

    bool LanClientV2::connect(const CString& addy, unsigned short port) {

        if (connected()) return false;

        portal_ = CmdPortal<IServerCmd, IClientCmd>::ConnectTo(addy, port);

        if (!connected()) return false;

        return true;
    }

    void LanClientV2::close() {}
    vector<unique_ptr<IClientCmd>> LanClientV2::ReceiveCmds() { return {}; }
}
