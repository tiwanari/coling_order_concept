#pragma once
#include <iostream>
#include <chrono>
#include <sstream>

namespace order_concepts {
namespace util {
template<typename TimeT = std::chrono::milliseconds,
    typename ClockT=std::chrono::high_resolution_clock,
    typename DurationT=double>

class Stopwatch {
private:
    std::chrono::time_point<ClockT> m_start, m_end;
    std::string showUnit(std::chrono::seconds s) const { return "s"; }
    std::string showUnit(std::chrono::milliseconds s) const { return "ms"; }
    std::string showUnit(std::chrono::microseconds s) const { return "us"; }
    std::string showUnit(std::chrono::nanoseconds s) const { return "ns"; }
public:
    Stopwatch() { start(); }
    void start() { m_start = m_end = ClockT::now(); }
    DurationT stop() {
        m_end = ClockT::now();
        return elapsed();
    }
    DurationT elapsed() const {
        auto delta = std::chrono::duration_cast<TimeT>(m_end - m_start);
        return delta.count();
    }
    std::string showElapsedTime() const {
        std::stringstream ss;
        ss << elapsed() << " [" << showUnit(TimeT()) << "]";
        return ss.str();
    }
    std::string unit() const { return showUnit(TimeT()); }
};
}   // namespace util
}   // namespace order_concepts
