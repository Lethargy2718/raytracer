#pragma once

#include <chrono>

class Timer {
public:
    using clock = std::chrono::high_resolution_clock;

    Timer() : start(clock::now()) {}

    [[nodiscard]] double elapsed() const {
        auto end = clock::now();
        return std::chrono::duration<double>(end - start).count(); // seconds
    }

    [[nodiscard]] double elapsed_ms() const {
        auto end = clock::now();
        return std::chrono::duration<double, std::milli>(end - start).count(); // ms
    }

private:
    clock::time_point start;
};