// Boost.SafeProfile - C++ Safety Profile conformance analysis tool
// Copyright (c) 2025 The Boost Authors
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_SAFEPROFILE_VERSION_HPP
#define BOOST_SAFEPROFILE_VERSION_HPP

#define BOOST_SAFEPROFILE_VERSION_MAJOR 0
#define BOOST_SAFEPROFILE_VERSION_MINOR 0
#define BOOST_SAFEPROFILE_VERSION_PATCH 1

#define BOOST_SAFEPROFILE_VERSION_STRING "0.0.1"

namespace boost {
namespace safeprofile {

struct version {
    static constexpr int major = BOOST_SAFEPROFILE_VERSION_MAJOR;
    static constexpr int minor = BOOST_SAFEPROFILE_VERSION_MINOR;
    static constexpr int patch = BOOST_SAFEPROFILE_VERSION_PATCH;
    static constexpr const char* string = BOOST_SAFEPROFILE_VERSION_STRING;
};

} // namespace safeprofile
} // namespace boost

#endif // BOOST_SAFEPROFILE_VERSION_HPP
