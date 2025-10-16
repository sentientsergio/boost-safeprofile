// Boost.SafeProfile - C++ Safety Profile conformance analysis tool
// Copyright (c) 2025 The Boost Authors
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "sarif.hpp"
#include <fstream>
#include <stdexcept>

namespace boost {
namespace safeprofile {
namespace emit {

json::object sarif_emitter::generate(
    const std::vector<analysis::finding>& findings,
    const std::vector<profile::rule>& rules) const {

    json::object sarif;

    // SARIF version and schema
    sarif["$schema"] = "https://raw.githubusercontent.com/oasis-tcs/sarif-spec/master/Schemata/sarif-schema-2.1.0.json";
    sarif["version"] = "2.1.0";

    // Create the run
    json::object run;
    run["tool"] = json::object{
        {"driver", create_tool_driver(rules)}
    };

    // Convert findings to results
    json::array results;
    for (const auto& f : findings) {
        results.push_back(create_result(f));
    }
    run["results"] = std::move(results);

    // Wrap run in runs array
    json::array runs;
    runs.push_back(std::move(run));
    sarif["runs"] = std::move(runs);

    return sarif;
}

void sarif_emitter::write_to_file(
    const json::object& sarif_doc,
    const fs::path& output_path) const {

    std::ofstream file(output_path.string());
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open SARIF output file: " + output_path.string());
    }

    // Pretty-print the JSON
    file << json::serialize(sarif_doc);
}

std::string sarif_emitter::severity_to_level(profile::severity sev) const {
    switch (sev) {
        case profile::severity::blocker:
            return "error";
        case profile::severity::major:
            return "warning";
        case profile::severity::minor:
            return "note";
        case profile::severity::info:
            return "none";
        default:
            return "warning";
    }
}

json::object sarif_emitter::create_tool_driver(
    const std::vector<profile::rule>& rules) const {

    json::object driver;
    driver["name"] = "Boost.SafeProfile";
    driver["version"] = "0.0.1";
    driver["informationUri"] = "https://github.com/boost/safeprofile";
    driver["semanticVersion"] = "0.0.1";

    // Add rules metadata
    json::array sarif_rules;
    for (const auto& rule : rules) {
        json::object sarif_rule;
        sarif_rule["id"] = rule.id;

        json::object short_desc;
        short_desc["text"] = rule.title;
        sarif_rule["shortDescription"] = std::move(short_desc);

        json::object full_desc;
        full_desc["text"] = rule.description;
        sarif_rule["fullDescription"] = std::move(full_desc);

        json::object default_config;
        default_config["level"] = severity_to_level(rule.level);
        sarif_rule["defaultConfiguration"] = std::move(default_config);

        sarif_rules.push_back(std::move(sarif_rule));
    }
    driver["rules"] = std::move(sarif_rules);

    return driver;
}

json::object sarif_emitter::create_result(const analysis::finding& f) const {
    json::object result;

    result["ruleId"] = f.rule_id;
    result["level"] = severity_to_level(f.severity);

    json::object message;
    message["text"] = f.snippet;
    result["message"] = std::move(message);

    // Location information
    json::object location;
    json::object physical_location;

    json::object artifact_location;
    artifact_location["uri"] = f.file_path.string();
    physical_location["artifactLocation"] = std::move(artifact_location);

    json::object region;
    region["startLine"] = f.line_number;
    region["startColumn"] = f.column_number;
    physical_location["region"] = std::move(region);

    location["physicalLocation"] = std::move(physical_location);

    json::array locations;
    locations.push_back(std::move(location));
    result["locations"] = std::move(locations);

    return result;
}

} // namespace emit
} // namespace safeprofile
} // namespace boost
