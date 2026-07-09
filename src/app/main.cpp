#include "app/logging.hpp"
#include "app/run.hpp"

#include <fmt/core.h>

namespace {

using solar::app::log;

} // namespace

int main() {
    log("----------------------------------------");
    try {
        solar::app::AppObjects app = solar::app::make_app();
        solar::app::run_loop(app.context);
        solar::app::log_shutdown_report(app.simulation);

        return 0;
    } catch (const std::exception& error) {
        fmt::print(stderr, "fatal error: {}\n", error.what());
        return 1;
    }
}