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

    // SP-OWN-001: Naked new expression
    rule naked_new;
    naked_new.id = "SP-OWN-001";
    naked_new.title = "Naked new expression";
    naked_new.description =
        "Direct use of 'new' expression without RAII wrapper. "
        "Prefer std::make_unique, std::make_shared, or container allocation.";
    naked_new.level = severity::blocker;
    naked_new.pattern = " new ";  // Legacy pattern (AST detector doesn't use this)

    rules.push_back(naked_new);

    // SP-OWN-002: Naked delete expression
    rule naked_delete;
    naked_delete.id = "SP-OWN-002";
    naked_delete.title = "Naked delete expression";
    naked_delete.description =
        "Direct use of 'delete' expression indicates manual memory management. "
        "Prefer RAII with smart pointers or containers for automatic cleanup.";
    naked_delete.level = severity::blocker;
    naked_delete.pattern = " delete ";  // Legacy pattern (AST detector doesn't use this)

    rules.push_back(naked_delete);

    // SP-BOUNDS-001: C-style arrays
    rule c_array;
    c_array.id = "SP-BOUNDS-001";
    c_array.title = "C-style array declaration";
    c_array.description =
        "C-style array declaration lacks bounds checking. "
        "Prefer std::array for fixed-size or std::vector for dynamic arrays.";
    c_array.level = severity::major;
    c_array.pattern = "";  // AST-only

    rules.push_back(c_array);

    // SP-TYPE-001: C-style casts
    rule c_cast;
    c_cast.id = "SP-TYPE-001";
    c_cast.title = "C-style cast";
    c_cast.description =
        "C-style cast bypasses type safety checks. "
        "Prefer static_cast, const_cast, or reinterpret_cast for explicit intent.";
    c_cast.level = severity::major;
    c_cast.pattern = "";  // AST-only

    rules.push_back(c_cast);

    return rules;
}

} // namespace profile
} // namespace safeprofile
} // namespace boost
