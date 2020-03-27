
#include "BasicCmds.hpp"
#include "CmdSerializationFactory.h"
#include "IClientCmd.h"
#include "StringStack.h"

namespace Yhaniki {

    using std::string;
    using std::unique_ptr;
    using namespace Yhaniki::ClientCmd;

    template <>
    StringStack CmdSerializationFactory<IClientCmd>::Serialize(
        unique_ptr<IClientCmd> cmd) {

        auto builder = StringStack::Builder::New();

        if (cmd == nullptr) {
            builder.PushBack("NOP");

        } else if (auto clientCmd = dynamic_cast<Nop*>(cmd.get())) {
            builder.PushBack("NOP");

        } else if (auto clientCmd = dynamic_cast<Chat*>(cmd.get())) {
            builder.PushBack("CHAT");
            builder.PushBack(clientCmd->GetMessage());

        } else {
            builder.PushBack("NOP");
        }

        return builder.Compile();
    }

    template <>
    unique_ptr<IClientCmd> CmdSerializationFactory<IClientCmd>::Deserialize(
        StringStack str) {

        if (str[0] == "NOP")
            return std::make_unique<Nop>();

        if (str[0] == "CHAT")
            return std::make_unique<Chat>(str[1]);

        // TODO: Warn "Unknown or invalid command!"
        return std::make_unique<Nop>();
    }
}
