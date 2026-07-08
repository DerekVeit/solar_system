#pragma once

#include <fmt/chrono.h>
#include <fmt/format.h>

#include <chrono>
#include <string>
#include <utility>

namespace solar::app {

namespace detail {

void log_line(std::string line);

} // namespace detail

/// Append one timestamped line to the log file (flushed for tail -f).
template <typename... T> void log(fmt::format_string<T...> fmt_str, T&&... args) {
    const auto now = std::chrono::system_clock::now();
    detail::log_line(fmt::format("{:%Y-%m-%d %H:%M:%S} {}\n", now,
                                 fmt::format(fmt_str, std::forward<T>(args)...)));
}

} // namespace solar::app