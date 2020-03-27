

#include "StringStack.h"

namespace Yhaniki {
    using std::string;
    using std::vector;

    // TODO: What?
    const string delimiter = "_<^>_";

    StringStack::Builder StringStack::Builder::New() { return {}; }
    void StringStack::Builder::PushBack(string str) {
        // TODO: Check str validity!
        stack_.push_back(str);
    }
    StringStack StringStack::Builder::Compile() const {
        return { stack_ };
    }


    StringStack::StringStack(std::vector<std::string> stack) :
        stack_(move(stack)) {}

    StringStack StringStack::FromString(string str) {
        auto builder = Builder::New();

        size_t pos = 0;
        string token;
        while ((pos = str.find(delimiter)) != std::string::npos) {
            token = str.substr(0, pos);
            builder.PushBack(token);
            str.erase(0, pos + delimiter.length());
        }

        return builder.Compile();
    }

    string StringStack::ToString() const {
        string result = "";

        for (auto && str : stack_) 
            result += str + delimiter;

        return result;
    }

    std::string StringStack::operator[](std::size_t idx) const {
        if (idx >= stack_.size())
            return ""; // TODO: Is it a good sentinel value?
        return stack_[idx];
    }
}
