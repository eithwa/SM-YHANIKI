#pragma once
#include <memory>

namespace Yhaniki {

    template<class T, class Tbase>
    T* As(std::unique_ptr<Tbase>& base) {
        return dynamic_cast<T*>(base.get());
    }

    template<class T, class Tbase>
    std::unique_ptr<T> ReleaseAs(std::unique_ptr<Tbase>& base) {
        return std::unique_ptr<T>(dynamic_cast<T*>(base.release()));
    }
}
