#pragma once
#include <string>
#include <deque>
#include <mutex>
#include <chrono>
#include <ctime>

namespace Log {
    enum Level { INFO, OK, WARN, ERR };

    struct Entry {
        Level lvl;
        std::string msg;
        std::string time;
    };

    inline std::deque<Entry> buffer;
    inline std::mutex mtx;
    inline const size_t kMax = 400;

    inline std::string Now() {
        auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        std::tm tm{};
        localtime_s(&tm, &t);
        char buf[32];
        strftime(buf, sizeof(buf), "%H:%M:%S", &tm);
        return buf;
    }

    inline void Push(Level l, const std::string& s) {
        std::lock_guard<std::mutex> g(mtx);
        buffer.push_back({ l, s, Now() });
        while (buffer.size() > kMax) buffer.pop_front();
    }

    inline void Info(const std::string& s) { Push(INFO, s); }
    inline void Ok(const std::string& s)   { Push(OK,   s); }
    inline void Warn(const std::string& s) { Push(WARN, s); }
    inline void Err(const std::string& s)  { Push(ERR,  s); }
    inline void Clear() { std::lock_guard<std::mutex> g(mtx); buffer.clear(); }
}