


#include <string>
#include <utility>

#include "IClientCmd.h"

namespace Yhaniki {

    class Nop : public IClientCmd {
    public:
        Type GetType() const override { return Type::Nop; }
    };

    class Chat : public IClientCmd {
    public:
        Type GetType() const override { return Type::Chat; }

        Chat(std::string message) : message_(std::move(message)) {}
    private:
        std::string message_;
    };
}
