#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include "core/constants.hpp"
#include "core/types.hpp"

using Catch::Matchers::ContainsSubstring;


TEST_CASE("Epoch.to_string returns ISO-like timestamp string for J2000", "[types]") {
    const solar::core::Epoch epoch{solar::core::kJ2000Jd};
    const std::string ts = epoch.to_string();
    CHECK_THAT(ts, ContainsSubstring("(approx)"));
    CHECK_THAT(ts, ContainsSubstring("2000-01-01"));
}

TEST_CASE("Epoch.to_string for a later date", "[types]") {
    const solar::core::Epoch epoch{solar::core::kJ2000Jd + 35};
    const std::string ts = epoch.to_string();
    CHECK_THAT(ts, ContainsSubstring("2000-02-05"));
}

TEST_CASE("Epoch.to_string for a fractional day later", "[types]") {
    const solar::core::Epoch epoch{solar::core::kJ2000Jd + 0.5};
    const std::string ts = epoch.to_string();
    CHECK_THAT(ts, ContainsSubstring("2000-01-02 00:00:00"));
}

