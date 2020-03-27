


#include <string>
#include <utility>

#include "IClientCmd.h"

namespace Yhaniki::ClientCmd {

    class Nop : public IClientCmd {};

    class Chat : public IClientCmd {
    public:
        Chat(std::string message) : message_(std::move(message)) {}
        [[nodiscard]] std::string GetMessage() const { return message_; }
    private:
        std::string message_;
    };

}
