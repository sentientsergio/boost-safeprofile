// Boost.SafeProfile - C++ Safety Profile conformance analysis tool
// Copyright (c) 2025 The Boost Authors
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "arguments.hpp"
#include <boost/safeprofile/version.hpp>
#include <boost/program_options.hpp>
#include <iostream>

namespace po = boost::program_options;

namespace boost {
namespace safeprofile {
namespace cli {

std::optional<analyze_args> parse_arguments(int argc, char* argv[]) {
    analyze_args args;

    try {
        po::options_description general("General Options");
        general.add_options()
            ("help,h", "Show this help message")
            ("version,v", "Show version information")
        ;

        po::options_description analysis("Analysis Options");
        analysis.add_options()
            ("profile,p", po::value<std::string>()->default_value("core-safety"),
             "Safety Profile to use (e.g., core-safety, memory-safety)")
            ("config,c", po::value<std::string>(),
             "Path to configuration file (boostsafe.yaml)")
            ("offline", po::bool_switch()->default_value(true),
             "Run in offline mode (no network access)")
            ("online", "Enable online mode (for AI assistance)")
        ;

        po::options_description output("Output Options");
        output.add_options()
            ("sarif", po::value<std::string>(),
             "Output SARIF file path")
            ("report", po::value<std::string>(),
             "Output HTML report path")
            ("evidence", po::value<std::string>(),
             "Evidence pack output directory")
        ;

        po::options_description hidden("Hidden Options");
        hidden.add_options()
            ("target", po::value<std::string>(), "Target path or repository")
        ;

        po::positional_options_description positional;
        positional.add("target", 1);

        po::options_description cmdline_options;
        cmdline_options.add(general).add(analysis).add(output).add(hidden);

        po::options_description visible_options("boost-safeprofile - C++ Safety Profile conformance analysis tool");
        visible_options.add(general).add(analysis).add(output);

        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv)
                     .options(cmdline_options)
                     .positional(positional)
                     .run(),
                  vm);
        po::notify(vm);

        // Handle --version
        if (vm.count("version")) {
            std::cout << "boost-safeprofile " << version::string << "\n";
            std::cout << "C++ Safety Profile conformance analysis tool\n";
            std::cout << "License: Boost Software License 1.0\n";
            return std::nullopt;
        }

        // Handle --help
        if (vm.count("help") || !vm.count("target")) {
            std::cout << visible_options << "\n";
            std::cout << "Usage:\n";
            std::cout << "  boost-safeprofile [options] <path|repository>\n\n";
            std::cout << "Examples:\n";
            std::cout << "  boost-safeprofile ./my-project\n";
            std::cout << "  boost-safeprofile --profile memory-safety --sarif out.sarif ./src\n";
            std::cout << "  boost-safeprofile --evidence ./evidence https://github.com/user/repo\n";
            return std::nullopt;
        }

        // Extract arguments
        args.target_path = vm["target"].as<std::string>();
        args.profile = vm["profile"].as<std::string>();

        if (vm.count("config")) {
            args.config_file = vm["config"].as<std::string>();
        }

        if (vm.count("sarif")) {
            args.sarif_output = vm["sarif"].as<std::string>();
        }

        if (vm.count("report")) {
            args.html_output = vm["report"].as<std::string>();
        }

        if (vm.count("evidence")) {
            args.evidence_dir = vm["evidence"].as<std::string>();
        }

        // Handle online/offline mode
        if (vm.count("online")) {
            args.offline = false;
        } else {
            args.offline = vm["offline"].as<bool>();
        }

        return args;

    } catch (const po::error& e) {
        std::cerr << "Error parsing arguments: " << e.what() << "\n";
        std::cerr << "Use --help for usage information.\n";
        return std::nullopt;
    }
}

} // namespace cli
} // namespace safeprofile
} // namespace boost
