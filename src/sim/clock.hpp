#pragma once

#include "core/types.hpp"

namespace solar::sim {

enum class TimeScale {
    paused,
    real_time,
    accelerated,
};

class SimulationClock {
  public:
    explicit SimulationClock(core::Epoch start_epoch);

    [[nodiscard]] core::Epoch epoch() const { return epoch_; }
    [[nodiscard]] TimeScale time_scale() const { return time_scale_; }
    [[nodiscard]] double acceleration() const { return acceleration_; }

    void set_time_scale(TimeScale scale) { time_scale_ = scale; }
    void set_acceleration(double factor) { acceleration_ = factor; }
    void advance(double delta_seconds);

  private:
    core::Epoch epoch_{};
    TimeScale time_scale_{TimeScale::real_time};
    double acceleration_{1.0};
};

}  // namespace solar::sim