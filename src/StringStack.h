#pragma once
#include <string>
#include <vector>


namespace Yhaniki {

    class StringStack {
    public:

        class Builder {
        public:
            static Builder New();
            void PushBack(std::string str);
            StringStack Compile() const;

        private:
            std::vector<std::string> stack_;
        };

        StringStack(std::vector<std::string> stack);
        static StringStack FromString(std::string encodedStr);

        std::string ToString() const;

        std::string operator[](std::size_t idx) const;

    private:
        const std::vector<std::string> stack_;
    };
}
