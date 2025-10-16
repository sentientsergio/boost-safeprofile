// Boost.SafeProfile - C++ Safety Profile conformance analysis tool
// Copyright (c) 2025 The Boost Authors
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "loader.hpp"
#include <stdexcept>

namespace boost {
namespace safeprofile {
namespace profile {

std::vector<rule> loader::load_profile(const std::string& profile_name) {
    if (profile_name == "core-safety" || profile_name == "memory-safety") {
        // Phase 0: Both profiles return the same hardcoded rule
        return get_core_safety_profile();
    }

    throw std::runtime_error("Unknown profile: " + profile_name);
}

std::vector<rule> loader::get_core_safety_profile() {
    std::vector<rule> rules;

    // Phase 0: Single hardcoded rule for naked new detection
    rule naked_new;
    naked_new.id = "SP-OWN-001";
    naked_new.title = "Naked new expression";
    naked_new.description =
        "Direct use of 'new' expression without RAII wrapper. "
        "Prefer std::make_unique, std::make_shared, or container allocation.";
    naked_new.level = severity::blocker;
    naked_new.pattern = " new ";  // Simple keyword search for Phase 0

    rules.push_back(naked_new);

    return rules;
}

} // namespace profile
} // namespace safeprofile
} // namespace boost
