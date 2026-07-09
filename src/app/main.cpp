#include "app/logging.hpp"
#include "app/run.hpp"

#include <fmt/core.h>

int main() {
    solar::app::log("----------------------------------------");
    try {
        solar::app::AppObjects app = solar::app::make_app();
        app.run_loop();
        app.log_shutdown_report();
        return 0;
    } catch (const std::exception& error) {
        fmt::print(stderr, "fatal error: {}\n", error.what());
        return 1;
    }
}