#pragma once
#ifndef LIANT_MODULE
#include <cstring>
#endif

namespace liant {
template <typename TValue>
class DynamicArray {
    TValue staticPart{};
    TValue* dynamicPart{};

public:
    DynamicArray(const TValue& value) {
        staticPart = value;
    }
    DynamicArray(const DynamicArray<TValue>& values, const TValue& value) {
        staticPart = values.staticPart;

        if (values.dynamicPart) {
            const std::size_t dynLen = values.dynamicPartSize();

            dynamicPart = new TValue[dynLen + 2];
            std::memcpy(dynamicPart, values.dynamicPart, dynLen * sizeof(TValue));
            dynamicPart[dynLen] = value;
            dynamicPart[dynLen + 1] = {};
        } else {
            dynamicPart = new TValue[2];
            dynamicPart[0] = value;
            dynamicPart[1] = {};
        }
    }
    DynamicArray(const DynamicArray& other)
        : staticPart(other.staticPart) {
        if (other.dynamicPart) {
            const std::size_t dynLen = other.dynamicPartSize();
            dynamicPart = new TValue[dynLen + 1];

            std::memcpy(dynamicPart, other.dynamicPart, dynLen * sizeof(TValue));
            dynamicPart[dynLen] = {};
        }
    }
    DynamicArray(DynamicArray&& other) noexcept
        : staticPart(other.staticPart) {
        dynamicPart = other.dynamicPart;
        other.dynamicPart = {};
    }
    DynamicArray& operator=(DynamicArray other) {
        using std::swap;

        swap(staticPart, other.staticPart);
        swap(dynamicPart, other.dynamicPart);

        return *this;
    }
    ~DynamicArray() {
        if (dynamicPart) {
            delete[] dynamicPart;
        }
    }

    std::size_t size() const noexcept {
        return dynamicPart ? dynamicPartSize() + 1 : 1;
    }

    const TValue& operator[](std::size_t idx) const {
        return idx == 0 ? staticPart : dynamicPart[idx - 1];
    }

private:
    std::size_t dynamicPartSize() const noexcept {
        std::size_t length = 0;
        while (dynamicPart[length]) {
            length++;
        }
        return length;
    }
};
} // namespace liant