
#include "BasicCmds.hpp"
#include "CmdSerializationFactory.h"
#include "IClientCmd.h"
#include "StringStack.h"

namespace Yhaniki {

    using std::string;
    using std::unique_ptr;

    template <>
    StringStack CmdSerializationFactory<IClientCmd>::Serialize(
        unique_ptr<IClientCmd> cmd) {

        auto builder = StringStack::Builder::New();

        //builder.PushBack(cmd->GetType());

        return builder.Compile();
    }

    template <>
    unique_ptr<IClientCmd> CmdSerializationFactory<IClientCmd>::Deserialize(
        StringStack str) {

        return unique_ptr<IClientCmd>{ new Nop() };
    }
}
