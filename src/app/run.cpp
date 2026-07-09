#include "app/run.hpp"

#include "app/logging.hpp"

namespace solar::app {

void log_shutdown_report(const sim::SolarSystem& simulation) {
    log("Earth position at shutdown: {}", simulation.state("Earth").to_string());
    log("Venus position at shutdown: {}", simulation.state("Venus").to_string());
    log("Mars position at shutdown: {}", simulation.state("Mars").to_string());
    log("Jupiter position at shutdown: {}", simulation.state("Jupiter").to_string());

    log("Simulation epoch JD: {:.4f}", simulation.clock().epoch().jd);
    log("Simulation epoch: {}", simulation.clock().epoch().to_string());
}

} // namespace solar::app
