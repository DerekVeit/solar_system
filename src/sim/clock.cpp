#include "sim/clock.hpp"

#include "core/constants.hpp"

namespace solar::sim {

SimulationClock::SimulationClock(core::Epoch start_epoch)
    : epoch_(start_epoch) {}

void SimulationClock::advance(double delta_seconds) {
    if (time_scale_ == TimeScale::paused) {
        return;
    }

    double effective_delta = delta_seconds;
    if (time_scale_ == TimeScale::accelerated) {
        effective_delta *= acceleration_;
    }

    epoch_.jd += effective_delta / core::kSecondsPerDay;
}

} // namespace solar::sim