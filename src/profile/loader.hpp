// Boost.SafeProfile - C++ Safety Profile conformance analysis tool
// Copyright (c) 2025 The Boost Authors
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_SAFEPROFILE_PROFILE_LOADER_HPP
#define BOOST_SAFEPROFILE_PROFILE_LOADER_HPP

#include "rule.hpp"
#include <vector>
#include <string>

namespace boost {
namespace safeprofile {
namespace profile {

/// Profile loader - loads safety profile rules
class loader {
public:
    /// Load a profile by name
    /// For Phase 0, returns a hardcoded "core-safety" profile with one rule
    static std::vector<rule> load_profile(const std::string& profile_name);

private:
    /// Get the built-in core-safety profile (hardcoded for Phase 0)
    static std::vector<rule> get_core_safety_profile();
};

} // namespace profile
} // namespace safeprofile
} // namespace boost

#endif // BOOST_SAFEPROFILE_PROFILE_LOADER_HPP
