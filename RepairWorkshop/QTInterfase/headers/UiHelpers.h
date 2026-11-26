#ifndef UIHELPERS_H
#define UIHELPERS_H

#include <memory>
#include <utility>

template <typename T, typename... Args>
T* makeOwned(Args&&... args) {
    return std::make_unique<T>(std::forward<Args>(args)...).release();
}

#endif
