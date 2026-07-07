#include "app/logging.hpp"

#include <fmt/os.h>

namespace solar::app::detail {

namespace {

constexpr const char* kLogFilePath = "/tmp/solar_system.log";

fmt::ostream& log_stream() {
    static fmt::ostream stream = fmt::output_file(
        kLogFilePath,
        fmt::file::WRONLY | fmt::file::CREATE | fmt::file::APPEND);
    return stream;
}

}  // namespace

void log_line(std::string line) {
    log_stream().print("{}", line);
    log_stream().flush();
}

}  // namespace solar::app::detail