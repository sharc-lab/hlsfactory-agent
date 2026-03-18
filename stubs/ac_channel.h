/*
 * Minimal Catapult ac_channel stub
 * For compilation testing with clang++ only - NOT for synthesis
 */

#ifndef __AC_CHANNEL_H__
#define __AC_CHANNEL_H__

#include <cstddef>
#include <queue>

template <typename T>
class ac_channel {
public:
    ac_channel() = default;

    void write(const T& value) {
        data_.push(value);
    }

    T read() {
        if (data_.empty()) {
            return T{};
        }
        T value = data_.front();
        data_.pop();
        return value;
    }

    bool available(std::size_t count = 1) const {
        return data_.size() >= count;
    }

    bool empty() const {
        return data_.empty();
    }

private:
    std::queue<T> data_;
};

#endif // __AC_CHANNEL_H__
