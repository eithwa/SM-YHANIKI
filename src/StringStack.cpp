

#include "StringStack.h"

namespace Yhaniki {
    using std::string;
    using std::vector;



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
        return { vector<string>{} };
    }
    string StringStack::ToString() const { return ""; }
}
