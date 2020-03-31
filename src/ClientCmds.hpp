


#include <string>
#include <utility>

#include "IClientCmd.h"

namespace Yhaniki::ClientCmd {

    using std::string;
    using std::move;

    class Nop : public IClientCmd {};

    class Chat : public IClientCmd {
    public:
        Chat(string message) : message_(move(message)) {}
        [[nodiscard]] string GetMsg() const { return message_; }
    private:
        string message_;
    };

    class ReportState : public IClientCmd {
    public:
        enum State {
            IDLE,
            SelectMusic,
        };
        ReportState(bool on) : on_(on) {}
        [[nodiscard]] bool IsOn() const { return on_; }
    private:
        bool on_;
    };
}
