#pragma once
#include <memory>

#include "StringStack.h"

namespace Yhaniki {

    template<class TICmd>
    class CmdSerializationFactory {
    public:
        static StringStack Serialize(std::unique_ptr<TICmd> cmd);
        static std::unique_ptr<TICmd> Deserialize(StringStack str);
    };
}