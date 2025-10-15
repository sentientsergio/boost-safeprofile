// Boost.SafeProfile - C++ Safety Profile conformance analysis tool
// Copyright (c) 2025 The Boost Authors
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <boost/safeprofile/version.hpp>
#include <iostream>

int main(int /*argc*/, char* /*argv*/[]) {
    std::cout << "boost-safeprofile " << boost::safeprofile::version::string << "\n";
    std::cout << "Safety Profile conformance analysis tool\n";
    return 0;
}
