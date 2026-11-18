#ifndef REPOSITORY_H
#define REPOSITORY_H

#include "../Exeptions/EntityNotFoundException.h"
#include <vector>
#include <algorithm>
#include <ranges>
#include <iterator>
#include <cstddef>

template <typename T>
class Repository {
private:
    std::vector<T> items;

public:
    Repository() = default;

    void add(const T& item) {
        items.push_back(item);
    }

    void add(T&& item) {
        items.push_back(std::move(item));
    }

    Repository<T>& operator+=(const T& item) {
        add(item);
        return *this;
    }

    bool removeById(unsigned int targetId) {
        auto oldSize = items.size();
        auto removed = std::ranges::remove_if(items,
            [targetId](const T& obj) { return obj.getId() == targetId; });
        items.erase(removed.begin(), removed.end());
        return items.size() < oldSize;
    }

    T* findById(unsigned int targetId) {
        auto it = std::ranges::find_if(items,
            [targetId](const T& obj) { return obj.getId() == targetId; });
        return (it != items.end()) ? &(*it) : nullptr;
    }

    const T* findById(unsigned int targetId) const {
        auto it = std::ranges::find_if(items,
            [targetId](const T& obj) { return obj.getId() == targetId; });
        return (it != items.cend()) ? &(*it) : nullptr;
    }

    template <typename Predicate>
    std::vector<T> findIf(Predicate pred) const {
        std::vector<T> result;
        std::ranges::copy_if(items, std::back_inserter(result), pred);
        return result;
    }

    T& operator[](std::size_t index) {
        if (index >= items.size()) {
            throw EntityNotFoundException("Repository index out of range");
        }
        return items[index];
    }

    const T& operator[](std::size_t index) const {
        if (index >= items.size()) {
            throw EntityNotFoundException("Repository index out of range");
        }
        return items[index];
    }

    std::size_t size() const noexcept {
        return items.size();
    }

    bool empty() const noexcept {
        return items.empty();
    }

    void clear() noexcept {
        items.clear();
    }

    auto begin() noexcept { return items.begin(); }
    auto end() noexcept { return items.end(); }
    auto begin() const noexcept { return items.cbegin(); }
    auto end() const noexcept { return items.cend(); }
    auto cbegin() const noexcept { return items.cbegin(); }
    auto cend() const noexcept { return items.cend(); }
};

#endif
