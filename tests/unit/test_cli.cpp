// Boost.SafeProfile - CLI argument parsing tests
// Copyright (c) 2025 The Boost Authors
// Distributed under the Boost Software License, Version 1.0.

#include <boost/test/unit_test.hpp>
#include "cli/arguments.hpp"

BOOST_AUTO_TEST_SUITE(cli_tests)

BOOST_AUTO_TEST_CASE(test_help_option) {
    const char* argv[] = {"boost-safeprofile", "--help"};
    int argc = 2;

    auto args = boost::safeprofile::cli::parse_arguments(argc, const_cast<char**>(argv));

    BOOST_TEST(!args.has_value()); // Should return empty when help is shown
}

BOOST_AUTO_TEST_CASE(test_version_option) {
    const char* argv[] = {"boost-safeprofile", "--version"};
    int argc = 2;

    auto args = boost::safeprofile::cli::parse_arguments(argc, const_cast<char**>(argv));

    BOOST_TEST(!args.has_value()); // Should return empty when version is shown
}

BOOST_AUTO_TEST_CASE(test_profile_option) {
    const char* argv[] = {"boost-safeprofile", "--profile", "memory-safety", "."};
    int argc = 4;

    auto args = boost::safeprofile::cli::parse_arguments(argc, const_cast<char**>(argv));

    BOOST_REQUIRE(args.has_value());
    BOOST_TEST(args->profile == "memory-safety");
}

BOOST_AUTO_TEST_CASE(test_sarif_output) {
    const char* argv[] = {"boost-safeprofile", "--sarif", "out.sarif", "."};
    int argc = 4;

    auto args = boost::safeprofile::cli::parse_arguments(argc, const_cast<char**>(argv));

    BOOST_REQUIRE(args.has_value());
    BOOST_TEST(args->sarif_output.has_value());
    BOOST_TEST(args->sarif_output.value() == "out.sarif");
}

BOOST_AUTO_TEST_CASE(test_offline_mode_default) {
    const char* argv[] = {"boost-safeprofile", "."};
    int argc = 2;

    auto args = boost::safeprofile::cli::parse_arguments(argc, const_cast<char**>(argv));

    BOOST_REQUIRE(args.has_value());
    BOOST_TEST(args->offline == true); // Offline is default
}

BOOST_AUTO_TEST_CASE(test_online_mode) {
    const char* argv[] = {"boost-safeprofile", "--online", "."};
    int argc = 3;

    auto args = boost::safeprofile::cli::parse_arguments(argc, const_cast<char**>(argv));

    BOOST_REQUIRE(args.has_value());
    BOOST_TEST(args->offline == false);
}

BOOST_AUTO_TEST_CASE(test_missing_path) {
    const char* argv[] = {"boost-safeprofile"};
    int argc = 1;

    auto args = boost::safeprofile::cli::parse_arguments(argc, const_cast<char**>(argv));

    BOOST_TEST(!args.has_value()); // Should fail without path
}

BOOST_AUTO_TEST_SUITE_END()
