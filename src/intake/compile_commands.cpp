// Boost.SafeProfile - C++ Safety Profile conformance analysis tool
// Copyright (c) 2025 The Boost Authors
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "compile_commands.hpp"
#include <boost/json.hpp>
#include <fstream>
#include <sstream>
#include <iostream>

namespace boost {
namespace safeprofile {
namespace intake {

namespace json = boost::json;

bool compile_commands_reader::load_from_directory(const fs::path& directory) {
    // Look for compile_commands.json in the directory
    fs::path db_path = directory / "compile_commands.json";

    if (!fs::exists(db_path)) {
        return false;
    }

    // Read JSON file
    std::ifstream ifs(db_path.string());
    if (!ifs) {
        return false;
    }

    std::stringstream buffer;
    buffer << ifs.rdbuf();
    std::string content = buffer.str();

    try {
        // Parse JSON
        json::value doc = json::parse(content);

        if (!doc.is_array()) {
            std::cerr << "Warning: compile_commands.json is not a JSON array\n";
            return false;
        }

        const auto& entries = doc.as_array();

        // Process each compilation database entry
        for (const auto& entry : entries) {
            if (!entry.is_object()) continue;

            const auto& obj = entry.as_object();

            // Required fields: file, directory, command (or arguments)
            if (!obj.contains("file") || !obj.contains("directory")) {
                continue;
            }

            std::string file = obj.at("file").as_string().c_str();
            std::string directory_str = obj.at("directory").as_string().c_str();

            // Get command string (either "command" or "arguments" field)
            std::string command;
            if (obj.contains("command")) {
                command = obj.at("command").as_string().c_str();
            } else if (obj.contains("arguments")) {
                // Convert arguments array to command string
                const auto& args = obj.at("arguments").as_array();
                for (size_t i = 0; i < args.size(); ++i) {
                    if (i > 0) command += " ";
                    command += args[i].as_string().c_str();
                }
            } else {
                continue;  // No command available
            }

            // Parse flags from command
            compilation_flags flags = parse_command(command, directory_str);

            // Normalize file path to absolute
            fs::path file_path(file);
            if (file_path.is_relative()) {
                file_path = fs::path(directory_str) / file_path;
            }

            std::string normalized = normalize_path(file_path);
            commands_[normalized] = flags;
        }

        loaded_ = true;
        return true;

    } catch (const std::exception& e) {
        std::cerr << "Error parsing compile_commands.json: " << e.what() << "\n";
        return false;
    }
}

std::optional<compilation_flags> compile_commands_reader::get_flags_for_file(
    const fs::path& source_file
) const {
    if (!loaded_) {
        return std::nullopt;
    }

    std::string normalized = normalize_path(source_file);
    auto it = commands_.find(normalized);

    if (it != commands_.end()) {
        return it->second;
    }

    return std::nullopt;
}

compilation_flags compile_commands_reader::parse_command(
    const std::string& command,
    const std::string& working_directory
) const {
    compilation_flags flags;
    flags.working_directory = working_directory;
    flags.std_version = "c++20";  // Default fallback

    // Simple tokenization by spaces (good enough for most cases)
    std::istringstream iss(command);
    std::string token;
    std::string prev_token;

    while (iss >> token) {
        // -I include paths
        if (token.find("-I") == 0) {
            if (token.length() > 2) {
                // -Ipath format
                std::string path = token.substr(2);
                flags.include_paths.push_back(path);
            } else if (!prev_token.empty()) {
                // -I path format (next token is the path)
                // Will be picked up on next iteration
            }
        } else if (prev_token == "-I") {
            flags.include_paths.push_back(token);
        }

        // -D defines
        else if (token.find("-D") == 0) {
            if (token.length() > 2) {
                flags.defines.push_back(token.substr(2));
            }
        } else if (prev_token == "-D") {
            flags.defines.push_back(token);
        }

        // -std=c++XX
        else if (token.find("-std=") == 0) {
            flags.std_version = token.substr(5);
        }

        // -isystem (system include paths - treat like -I)
        else if (token == "-isystem") {
            // Next token will be the path
        } else if (prev_token == "-isystem") {
            flags.include_paths.push_back(token);
        }

        prev_token = token;
    }

    return flags;
}

std::string compile_commands_reader::normalize_path(const fs::path& path) const {
    try {
        // Convert to absolute canonical path for consistent lookup
        fs::path abs = fs::absolute(path);
        if (fs::exists(abs)) {
            return fs::canonical(abs).string();
        }
        return abs.string();
    } catch (...) {
        return path.string();
    }
}

} // namespace intake
} // namespace safeprofile
} // namespace boost
