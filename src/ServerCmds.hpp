#include <string>
#include <utility>

#include "IServerCmd.h"

namespace Yhaniki::ServerCmd {

    using std::string;
    using std::move;

    class ServerNop : public IServerCmd {};

    class SomeoneChatted : public IServerCmd {
    public:
        SomeoneChatted(std::string message) : message_(std::move(message)) {}
        [[nodiscard]] std::string GetMsg() const { return message_; }
    private:
        std::string message_;
    };

}