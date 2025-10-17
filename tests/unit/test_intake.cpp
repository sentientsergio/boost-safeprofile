// Boost.SafeProfile - Intake module tests
// Copyright (c) 2025 The Boost Authors
// Distributed under the Boost Software License, Version 1.0.

#include <boost/test/unit_test.hpp>
#include "intake/repository.hpp"
#include <boost/filesystem.hpp>
#include <fstream>

namespace fs = boost::filesystem;

BOOST_AUTO_TEST_SUITE(intake_tests)

struct TempDirFixture {
    fs::path temp_dir;

    TempDirFixture() {
        temp_dir = fs::temp_directory_path() / fs::unique_path("safeprofile_test_%%%%-%%%%");
        fs::create_directories(temp_dir);
    }

    ~TempDirFixture() {
        if (fs::exists(temp_dir)) {
            fs::remove_all(temp_dir);
        }
    }

    void create_file(const std::string& relative_path, const std::string& content = "") {
        auto full_path = temp_dir / relative_path;
        fs::create_directories(full_path.parent_path());
        std::ofstream ofs(full_path.string());
        ofs << content;
    }
};

BOOST_FIXTURE_TEST_CASE(test_discovers_cpp_files, TempDirFixture) {
    create_file("main.cpp");
    create_file("utils.cpp");
    create_file("header.hpp");

    boost::safeprofile::intake::repository repo(temp_dir);
    auto files = repo.discover_sources();

    BOOST_TEST(files.size() == 3u);
}

BOOST_FIXTURE_TEST_CASE(test_discovers_various_extensions, TempDirFixture) {
    create_file("test.cpp");
    create_file("test.cc");
    create_file("test.cxx");
    create_file("test.hpp");
    create_file("test.h");
    create_file("test.hxx");

    boost::safeprofile::intake::repository repo(temp_dir);
    auto files = repo.discover_sources();

    BOOST_TEST(files.size() == 6u);
}

BOOST_FIXTURE_TEST_CASE(test_ignores_non_cpp_files, TempDirFixture) {
    create_file("code.cpp");
    create_file("readme.txt");
    create_file("data.json");
    create_file("script.py");

    boost::safeprofile::intake::repository repo(temp_dir);
    auto files = repo.discover_sources();

    BOOST_TEST(files.size() == 1u); // Only code.cpp
}

BOOST_FIXTURE_TEST_CASE(test_recursive_discovery, TempDirFixture) {
    create_file("main.cpp");
    create_file("src/utils.cpp");
    create_file("src/core/engine.cpp");
    create_file("include/header.hpp");

    boost::safeprofile::intake::repository repo(temp_dir);
    auto files = repo.discover_sources();

    BOOST_TEST(files.size() == 4u);
}

BOOST_FIXTURE_TEST_CASE(test_empty_directory, TempDirFixture) {
    boost::safeprofile::intake::repository repo(temp_dir);
    auto files = repo.discover_sources();

    BOOST_TEST(files.empty());
}

BOOST_AUTO_TEST_CASE(test_root_path_accessor) {
    fs::path test_path = fs::temp_directory_path();
    boost::safeprofile::intake::repository repo(test_path);

    BOOST_TEST(repo.root() == test_path);
}

BOOST_AUTO_TEST_SUITE_END()
