// Boost.SafeProfile - AST detector tests
// Copyright (c) 2025 The Boost Authors
// Distributed under the Boost Software License, Version 1.0.

#include <boost/test/unit_test.hpp>
#include "analysis/ast_detector.hpp"
#include "profile/rule.hpp"
#include <boost/filesystem.hpp>
#include <fstream>

namespace fs = boost::filesystem;
using namespace boost::safeprofile;

BOOST_AUTO_TEST_SUITE(ast_detector_tests)

// Helper to create temporary test files
struct temp_file {
    fs::path path;

    temp_file(const std::string& name, const std::string& content) {
        path = fs::temp_directory_path() / name;
        std::ofstream ofs(path.string());
        ofs << content;
    }

    ~temp_file() {
        if (fs::exists(path)) {
            fs::remove(path);
        }
    }
};

BOOST_AUTO_TEST_CASE(test_detect_naked_delete_scalar) {
    temp_file test_cpp("test_delete_scalar.cpp", R"(
void leak() {
    int* p = new int(42);
    delete p;  // SP-OWN-002 violation
}
)");

    profile::rule delete_rule;
    delete_rule.id = "SP-OWN-002";
    delete_rule.title = "Naked delete expression";
    delete_rule.description = "Direct use of 'delete' expression";
    delete_rule.level = profile::severity::blocker;

    analysis::ast_detector detector;
    auto findings = detector.analyze_file(test_cpp.path, delete_rule);

    BOOST_REQUIRE_EQUAL(findings.size(), 1);
    BOOST_TEST(findings[0].rule_id == "SP-OWN-002");
    BOOST_TEST(findings[0].line == 4);
}

BOOST_AUTO_TEST_CASE(test_detect_naked_delete_array) {
    temp_file test_cpp("test_delete_array.cpp", R"(
void leak() {
    int* arr = new int[10];
    delete[] arr;  // SP-OWN-002 violation (array form)
}
)");

    profile::rule delete_rule;
    delete_rule.id = "SP-OWN-002";
    delete_rule.title = "Naked delete expression";
    delete_rule.description = "Direct use of 'delete' expression";
    delete_rule.level = profile::severity::blocker;

    analysis::ast_detector detector;
    auto findings = detector.analyze_file(test_cpp.path, delete_rule);

    BOOST_REQUIRE_EQUAL(findings.size(), 1);
    BOOST_TEST(findings[0].rule_id == "SP-OWN-002");
    BOOST_TEST(findings[0].message.find("array form") != std::string::npos);
}

BOOST_AUTO_TEST_CASE(test_no_delete_safe_code) {
    temp_file test_cpp("test_no_delete.cpp", R"(
#include <memory>

void safe() {
    auto p = std::make_unique<int>(42);
    // No manual delete needed - RAII
}
)");

    profile::rule delete_rule;
    delete_rule.id = "SP-OWN-002";
    delete_rule.title = "Naked delete expression";
    delete_rule.description = "Direct use of 'delete' expression";
    delete_rule.level = profile::severity::blocker;

    analysis::ast_detector detector;
    auto findings = detector.analyze_file(test_cpp.path, delete_rule);

    BOOST_TEST(findings.empty());
}

BOOST_AUTO_TEST_CASE(test_multiple_deletes) {
    temp_file test_cpp("test_multiple_deletes.cpp", R"(
void bad() {
    int* p1 = new int(1);
    int* p2 = new int(2);
    delete p1;  // Violation 1
    delete p2;  // Violation 2
}
)");

    profile::rule delete_rule;
    delete_rule.id = "SP-OWN-002";
    delete_rule.title = "Naked delete expression";
    delete_rule.description = "Direct use of 'delete' expression";
    delete_rule.level = profile::severity::blocker;

    analysis::ast_detector detector;
    auto findings = detector.analyze_file(test_cpp.path, delete_rule);

    BOOST_REQUIRE_EQUAL(findings.size(), 2);
}

BOOST_AUTO_TEST_CASE(test_detect_naked_new) {
    temp_file test_cpp("test_new.cpp", R"(
void leak() {
    int* p = new int(42);  // SP-OWN-001 violation
}
)");

    profile::rule new_rule;
    new_rule.id = "SP-OWN-001";
    new_rule.title = "Naked new expression";
    new_rule.description = "Direct use of 'new' expression";
    new_rule.level = profile::severity::blocker;

    analysis::ast_detector detector;
    auto findings = detector.analyze_file(test_cpp.path, new_rule);

    BOOST_REQUIRE_EQUAL(findings.size(), 1);
    BOOST_TEST(findings[0].rule_id == "SP-OWN-001");
    BOOST_TEST(findings[0].line == 3);
}

BOOST_AUTO_TEST_CASE(test_detect_c_array_fixed_size) {
    temp_file test_cpp("test_c_array.cpp", R"(
void unsafe() {
    int arr[10];  // SP-BOUNDS-001 violation
    arr[0] = 42;
}
)");

    profile::rule array_rule;
    array_rule.id = "SP-BOUNDS-001";
    array_rule.title = "C-style array declaration";
    array_rule.description = "C-style array lacks bounds checking";
    array_rule.level = profile::severity::major;

    analysis::ast_detector detector;
    auto findings = detector.analyze_file(test_cpp.path, array_rule);

    BOOST_REQUIRE_EQUAL(findings.size(), 1);
    BOOST_TEST(findings[0].rule_id == "SP-BOUNDS-001");
    BOOST_TEST(findings[0].line == 3);
    BOOST_TEST(findings[0].message.find("std::array<T, 10>") != std::string::npos);
}

BOOST_AUTO_TEST_CASE(test_detect_c_array_multidimensional) {
    temp_file test_cpp("test_c_array_2d.cpp", R"(
void matrix() {
    int grid[5][5];  // SP-BOUNDS-001 violation
}
)");

    profile::rule array_rule;
    array_rule.id = "SP-BOUNDS-001";
    array_rule.title = "C-style array declaration";
    array_rule.description = "C-style array lacks bounds checking";
    array_rule.level = profile::severity::major;

    analysis::ast_detector detector;
    auto findings = detector.analyze_file(test_cpp.path, array_rule);

    BOOST_REQUIRE_EQUAL(findings.size(), 1);
    BOOST_TEST(findings[0].rule_id == "SP-BOUNDS-001");
}

BOOST_AUTO_TEST_CASE(test_safe_std_array) {
    temp_file test_cpp("test_std_array.cpp", R"(
#include <array>

void safe() {
    std::array<int, 10> arr;
    arr[0] = 42;
}
)");

    profile::rule array_rule;
    array_rule.id = "SP-BOUNDS-001";
    array_rule.title = "C-style array declaration";
    array_rule.description = "C-style array lacks bounds checking";
    array_rule.level = profile::severity::major;

    analysis::ast_detector detector;
    auto findings = detector.analyze_file(test_cpp.path, array_rule);

    BOOST_TEST(findings.empty());
}

BOOST_AUTO_TEST_CASE(test_safe_std_vector) {
    temp_file test_cpp("test_std_vector.cpp", R"(
#include <vector>

void safe() {
    std::vector<int> vec(10);
    vec[0] = 42;
}
)");

    profile::rule array_rule;
    array_rule.id = "SP-BOUNDS-001";
    array_rule.title = "C-style array declaration";
    array_rule.description = "C-style array lacks bounds checking";
    array_rule.level = profile::severity::major;

    analysis::ast_detector detector;
    auto findings = detector.analyze_file(test_cpp.path, array_rule);

    BOOST_TEST(findings.empty());
}

BOOST_AUTO_TEST_CASE(test_multiple_c_arrays) {
    temp_file test_cpp("test_multiple_arrays.cpp", R"(
void bad() {
    int arr1[5];   // Violation 1
    char arr2[10]; // Violation 2
    double arr3[3]; // Violation 3
}
)");

    profile::rule array_rule;
    array_rule.id = "SP-BOUNDS-001";
    array_rule.title = "C-style array declaration";
    array_rule.description = "C-style array lacks bounds checking";
    array_rule.level = profile::severity::major;

    analysis::ast_detector detector;
    auto findings = detector.analyze_file(test_cpp.path, array_rule);

    BOOST_REQUIRE_EQUAL(findings.size(), 3);
}

BOOST_AUTO_TEST_CASE(test_detect_c_cast_int) {
    temp_file test_cpp("test_c_cast_int.cpp", R"(
void unsafe() {
    double d = 3.14;
    int i = (int)d;  // SP-TYPE-001 violation
}
)");

    profile::rule cast_rule;
    cast_rule.id = "SP-TYPE-001";
    cast_rule.title = "C-style cast";
    cast_rule.description = "C-style cast bypasses type safety";
    cast_rule.level = profile::severity::major;

    analysis::ast_detector detector;
    auto findings = detector.analyze_file(test_cpp.path, cast_rule);

    BOOST_REQUIRE_EQUAL(findings.size(), 1);
    BOOST_TEST(findings[0].rule_id == "SP-TYPE-001");
    BOOST_TEST(findings[0].line == 4);
}

BOOST_AUTO_TEST_CASE(test_detect_c_cast_pointer) {
    temp_file test_cpp("test_c_cast_ptr.cpp", R"(
void unsafe() {
    const char* str = "hello";
    char* mutable_str = (char*)str;  // SP-TYPE-001 violation
}
)");

    profile::rule cast_rule;
    cast_rule.id = "SP-TYPE-001";
    cast_rule.title = "C-style cast";
    cast_rule.description = "C-style cast bypasses type safety";
    cast_rule.level = profile::severity::major;

    analysis::ast_detector detector;
    auto findings = detector.analyze_file(test_cpp.path, cast_rule);

    BOOST_REQUIRE_EQUAL(findings.size(), 1);
    BOOST_TEST(findings[0].message.find("const char *") != std::string::npos);
}

BOOST_AUTO_TEST_CASE(test_safe_static_cast) {
    temp_file test_cpp("test_static_cast.cpp", R"(
void safe() {
    double d = 3.14;
    int i = static_cast<int>(d);
}
)");

    profile::rule cast_rule;
    cast_rule.id = "SP-TYPE-001";
    cast_rule.title = "C-style cast";
    cast_rule.description = "C-style cast bypasses type safety";
    cast_rule.level = profile::severity::major;

    analysis::ast_detector detector;
    auto findings = detector.analyze_file(test_cpp.path, cast_rule);

    BOOST_TEST(findings.empty());
}

BOOST_AUTO_TEST_CASE(test_safe_const_cast) {
    temp_file test_cpp("test_const_cast.cpp", R"(
void safe() {
    const int* cp = nullptr;
    int* p = const_cast<int*>(cp);
}
)");

    profile::rule cast_rule;
    cast_rule.id = "SP-TYPE-001";
    cast_rule.title = "C-style cast";
    cast_rule.description = "C-style cast bypasses type safety";
    cast_rule.level = profile::severity::major;

    analysis::ast_detector detector;
    auto findings = detector.analyze_file(test_cpp.path, cast_rule);

    BOOST_TEST(findings.empty());
}

BOOST_AUTO_TEST_CASE(test_multiple_c_casts) {
    temp_file test_cpp("test_multiple_casts.cpp", R"(
void bad() {
    double d = 3.14;
    int i = (int)d;           // Violation 1
    float f = (float)d;       // Violation 2
    char c = (char)i;         // Violation 3
}
)");

    profile::rule cast_rule;
    cast_rule.id = "SP-TYPE-001";
    cast_rule.title = "C-style cast";
    cast_rule.description = "C-style cast bypasses type safety";
    cast_rule.level = profile::severity::major;

    analysis::ast_detector detector;
    auto findings = detector.analyze_file(test_cpp.path, cast_rule);

    BOOST_REQUIRE_EQUAL(findings.size(), 3);
}

BOOST_AUTO_TEST_CASE(test_return_address_of_local) {
    temp_file test_cpp("test_return_local_addr.cpp", R"(
int* dangerous() {
    int x = 42;
    return &x;  // SP-LIFE-003 violation
}
)");

    profile::rule lifetime_rule;
    lifetime_rule.id = "SP-LIFE-003";
    lifetime_rule.title = "Return reference to local";
    lifetime_rule.description = "Returning reference to local variable";
    lifetime_rule.level = profile::severity::blocker;

    analysis::ast_detector detector;
    auto findings = detector.analyze_file(test_cpp.path, lifetime_rule);

    BOOST_REQUIRE_EQUAL(findings.size(), 1);
    BOOST_TEST(findings[0].rule_id == "SP-LIFE-003");
    BOOST_TEST(findings[0].line == 4);
}

BOOST_AUTO_TEST_CASE(test_return_reference_to_local) {
    temp_file test_cpp("test_return_local_ref.cpp", R"(
int& dangerous() {
    int x = 42;
    return x;  // SP-LIFE-003 violation
}
)");

    profile::rule lifetime_rule;
    lifetime_rule.id = "SP-LIFE-003";
    lifetime_rule.title = "Return reference to local";
    lifetime_rule.description = "Returning reference to local variable";
    lifetime_rule.level = profile::severity::blocker;

    analysis::ast_detector detector;
    auto findings = detector.analyze_file(test_cpp.path, lifetime_rule);

    BOOST_REQUIRE_EQUAL(findings.size(), 1);
    BOOST_TEST(findings[0].message.find("'x'") != std::string::npos);
}

BOOST_AUTO_TEST_CASE(test_safe_return_heap) {
    temp_file test_cpp("test_return_heap.cpp", R"(
int* safe() {
    static int x = 42;
    return &x;  // OK - static storage
}
)");

    profile::rule lifetime_rule;
    lifetime_rule.id = "SP-LIFE-003";
    lifetime_rule.title = "Return reference to local";
    lifetime_rule.description = "Returning reference to local variable";
    lifetime_rule.level = profile::severity::blocker;

    analysis::ast_detector detector;
    auto findings = detector.analyze_file(test_cpp.path, lifetime_rule);

    BOOST_TEST(findings.empty());
}

BOOST_AUTO_TEST_CASE(test_safe_return_parameter) {
    temp_file test_cpp("test_return_param.cpp", R"(
int& safe(int& param) {
    return param;  // OK - parameter, not local
}
)");

    profile::rule lifetime_rule;
    lifetime_rule.id = "SP-LIFE-003";
    lifetime_rule.title = "Return reference to local";
    lifetime_rule.description = "Returning reference to local variable";
    lifetime_rule.level = profile::severity::blocker;

    analysis::ast_detector detector;
    auto findings = detector.analyze_file(test_cpp.path, lifetime_rule);

    BOOST_TEST(findings.empty());
}

BOOST_AUTO_TEST_CASE(test_multiple_return_local) {
    temp_file test_cpp("test_multiple_return_local.cpp", R"(
int* bad1() {
    int x = 1;
    return &x;  // Violation 1
}

int& bad2() {
    int y = 2;
    return y;   // Violation 2
}
)");

    profile::rule lifetime_rule;
    lifetime_rule.id = "SP-LIFE-003";
    lifetime_rule.title = "Return reference to local";
    lifetime_rule.description = "Returning reference to local variable";
    lifetime_rule.level = profile::severity::blocker;

    analysis::ast_detector detector;
    auto findings = detector.analyze_file(test_cpp.path, lifetime_rule);

    BOOST_REQUIRE_EQUAL(findings.size(), 2);
}

BOOST_AUTO_TEST_SUITE_END()
