
#include "IServerCmd.h"
#include "ServerCmds.hpp"
#include "SerializationFactory.h"
#include "StringStack.h"

namespace Yhaniki {

    using std::string;
    using std::unique_ptr;
    using std::make_unique;
    using namespace ServerCmd;

    const string s_NOP = "NOP";
    const string s_CHAT = "SOMEONECHATTED";

    template <>
    StringStack SerializationFactory<IServerCmd>::Serialize(
        unique_ptr<IServerCmd> cmd) {

        auto builder = StringStack::Builder::New();

        if (cmd == nullptr) {
            builder.PushBack(s_NOP);

        } else if (auto _ = dynamic_cast<Nop*>(cmd.get())) {
            builder.PushBack(s_NOP);

        } else if (const auto chatCmd = dynamic_cast<SomeoneChatted*>(cmd.get())) {
            builder.PushBack(s_CHAT);
            builder.PushBack(chatCmd->GetMsg());

        } else {
            builder.PushBack(s_NOP);
        }

        return builder.Compile();
    }

    template <>
    unique_ptr<IServerCmd> SerializationFactory<IServerCmd>::Deserialize(
        StringStack str) {

        if (str[0] == s_NOP)
            return make_unique<Nop>();

        if (str[0] == s_CHAT)
            return make_unique<SomeoneChatted>(str[1]);

        // TODO: Warn "Unknown or invalid command!"
        return make_unique<Nop>();
    }
}
