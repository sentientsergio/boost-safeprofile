// Boost.SafeProfile - C++ Safety Profile conformance analysis tool
// Copyright (c) 2025 The Boost Authors
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "ast_detector.hpp"
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Lex/Lexer.h>
#include <fstream>
#include <sstream>

namespace boost {
namespace safeprofile {
namespace analysis {

using namespace clang;
using namespace clang::ast_matchers;

// Callback for handling matched AST nodes
class NewExprCallback : public MatchFinder::MatchCallback {
public:
    NewExprCallback(
        std::vector<ast_finding>& findings,
        const fs::path& source_file,
        const profile::rule& rule
    ) : findings_(findings), source_file_(source_file), rule_(rule) {}

    void run(const MatchFinder::MatchResult& result) override {
        const auto* new_expr = result.Nodes.getNodeAs<CXXNewExpr>("newExpr");
        if (!new_expr) return;

        // Skip placement new (it has placement arguments)
        if (new_expr->getNumPlacementArgs() > 0) {
            return;
        }

        // Get source location
        const SourceManager& sm = *result.SourceManager;
        SourceLocation loc = new_expr->getBeginLoc();

        // Skip if not in main file (avoid stdlib/headers)
        if (!sm.isInMainFile(loc)) {
            return;
        }

        unsigned int line = sm.getExpansionLineNumber(loc);
        unsigned int column = sm.getExpansionColumnNumber(loc);

        // Extract code snippet
        std::string snippet = extractSnippet(sm, new_expr);

        // Determine if it's array or scalar new
        std::string message = rule_.description;
        if (new_expr->isArray()) {
            message += " (array form)";
        }

        findings_.push_back(ast_finding{
            source_file_,
            line,
            column,
            message,
            rule_.id,
            rule_.level,
            snippet
        });
    }

private:
    std::string extractSnippet(const SourceManager& sm, const CXXNewExpr* expr) {
        SourceRange range = expr->getSourceRange();
        CharSourceRange char_range = CharSourceRange::getTokenRange(range);

        StringRef snippet_ref = Lexer::getSourceText(char_range, sm, LangOptions());
        if (snippet_ref.empty()) {
            return "<code unavailable>";
        }

        // Limit snippet length
        std::string snippet = snippet_ref.str();
        if (snippet.length() > 80) {
            snippet = snippet.substr(0, 77) + "...";
        }

        return snippet;
    }

    std::vector<ast_finding>& findings_;
    fs::path source_file_;
    profile::rule rule_;
};

// Callback for handling delete expressions
class DeleteExprCallback : public MatchFinder::MatchCallback {
public:
    DeleteExprCallback(
        std::vector<ast_finding>& findings,
        const fs::path& source_file,
        const profile::rule& rule
    ) : findings_(findings), source_file_(source_file), rule_(rule) {}

    void run(const MatchFinder::MatchResult& result) override {
        const auto* delete_expr = result.Nodes.getNodeAs<CXXDeleteExpr>("deleteExpr");
        if (!delete_expr) return;

        // Get source location
        const SourceManager& sm = *result.SourceManager;
        SourceLocation loc = delete_expr->getBeginLoc();

        // Skip if not in main file (avoid stdlib/headers)
        if (!sm.isInMainFile(loc)) {
            return;
        }

        unsigned int line = sm.getExpansionLineNumber(loc);
        unsigned int column = sm.getExpansionColumnNumber(loc);

        // Extract code snippet
        std::string snippet = extractSnippet(sm, delete_expr);

        // Determine if it's array or scalar delete
        std::string message = rule_.description;
        if (delete_expr->isArrayForm()) {
            message += " (array form)";
        }

        findings_.push_back(ast_finding{
            source_file_,
            line,
            column,
            message,
            rule_.id,
            rule_.level,
            snippet
        });
    }

private:
    std::string extractSnippet(const SourceManager& sm, const CXXDeleteExpr* expr) {
        SourceRange range = expr->getSourceRange();
        CharSourceRange char_range = CharSourceRange::getTokenRange(range);

        StringRef snippet_ref = Lexer::getSourceText(char_range, sm, LangOptions());
        if (snippet_ref.empty()) {
            return "<code unavailable>";
        }

        // Limit snippet length
        std::string snippet = snippet_ref.str();
        if (snippet.length() > 80) {
            snippet = snippet.substr(0, 77) + "...";
        }

        return snippet;
    }

    std::vector<ast_finding>& findings_;
    fs::path source_file_;
    profile::rule rule_;
};

// Callback for handling C-style array declarations
class CStyleArrayCallback : public MatchFinder::MatchCallback {
public:
    CStyleArrayCallback(
        std::vector<ast_finding>& findings,
        const fs::path& source_file,
        const profile::rule& rule
    ) : findings_(findings), source_file_(source_file), rule_(rule) {}

    void run(const MatchFinder::MatchResult& result) override {
        const auto* var_decl = result.Nodes.getNodeAs<VarDecl>("arrayDecl");
        if (!var_decl) return;

        // Get source location
        const SourceManager& sm = *result.SourceManager;
        SourceLocation loc = var_decl->getBeginLoc();

        // Skip if not in main file (avoid stdlib/headers)
        if (!sm.isInMainFile(loc)) {
            return;
        }

        unsigned int line = sm.getExpansionLineNumber(loc);
        unsigned int column = sm.getExpansionColumnNumber(loc);

        // Extract code snippet
        std::string snippet = extractSnippet(sm, var_decl);

        // Get array type information
        const auto* array_type = var_decl->getType()->getAsArrayTypeUnsafe();
        std::string message = rule_.description;

        if (const auto* const_array = dyn_cast_or_null<ConstantArrayType>(array_type)) {
            // Fixed-size array - suggest std::array
            uint64_t size = const_array->getSize().getZExtValue();
            message += " Consider std::array<T, " + std::to_string(size) + ">.";
        } else {
            // Variable-length or incomplete array
            message += " Consider std::vector<T>.";
        }

        findings_.push_back(ast_finding{
            source_file_,
            line,
            column,
            message,
            rule_.id,
            rule_.level,
            snippet
        });
    }

private:
    std::string extractSnippet(const SourceManager& sm, const VarDecl* decl) {
        SourceRange range = decl->getSourceRange();
        CharSourceRange char_range = CharSourceRange::getTokenRange(range);

        StringRef snippet_ref = Lexer::getSourceText(char_range, sm, LangOptions());
        if (snippet_ref.empty()) {
            return "<code unavailable>";
        }

        // Limit snippet length
        std::string snippet = snippet_ref.str();
        if (snippet.length() > 80) {
            snippet = snippet.substr(0, 77) + "...";
        }

        return snippet;
    }

    std::vector<ast_finding>& findings_;
    fs::path source_file_;
    profile::rule rule_;
};

// Callback for handling C-style casts
class CStyleCastCallback : public MatchFinder::MatchCallback {
public:
    CStyleCastCallback(
        std::vector<ast_finding>& findings,
        const fs::path& source_file,
        const profile::rule& rule
    ) : findings_(findings), source_file_(source_file), rule_(rule) {}

    void run(const MatchFinder::MatchResult& result) override {
        const auto* cast_expr = result.Nodes.getNodeAs<CStyleCastExpr>("cStyleCast");
        if (!cast_expr) return;

        // Get source location
        const SourceManager& sm = *result.SourceManager;
        SourceLocation loc = cast_expr->getBeginLoc();

        // Skip if not in main file (avoid stdlib/headers)
        if (!sm.isInMainFile(loc)) {
            return;
        }

        unsigned int line = sm.getExpansionLineNumber(loc);
        unsigned int column = sm.getExpansionColumnNumber(loc);

        // Extract code snippet
        std::string snippet = extractSnippet(sm, cast_expr);

        // Build message with type information
        std::string message = rule_.description;

        // Get cast type names for better diagnostics
        QualType source_type = cast_expr->getSubExpr()->getType();
        QualType dest_type = cast_expr->getType();

        message += " Casting from '" + source_type.getAsString() +
                   "' to '" + dest_type.getAsString() + "'.";

        findings_.push_back(ast_finding{
            source_file_,
            line,
            column,
            message,
            rule_.id,
            rule_.level,
            snippet
        });
    }

private:
    std::string extractSnippet(const SourceManager& sm, const CStyleCastExpr* expr) {
        SourceRange range = expr->getSourceRange();
        CharSourceRange char_range = CharSourceRange::getTokenRange(range);

        StringRef snippet_ref = Lexer::getSourceText(char_range, sm, LangOptions());
        if (snippet_ref.empty()) {
            return "<code unavailable>";
        }

        // Limit snippet length
        std::string snippet = snippet_ref.str();
        if (snippet.length() > 80) {
            snippet = snippet.substr(0, 77) + "...";
        }

        return snippet;
    }

    std::vector<ast_finding>& findings_;
    fs::path source_file_;
    profile::rule rule_;
};

std::vector<ast_finding> ast_detector::analyze_file(
    const fs::path& source_file,
    const profile::rule& rule
) const {
    std::vector<ast_finding> findings;

    // Set up match finder based on rule ID
    MatchFinder finder;

    if (rule.id == "SP-OWN-001") {
        // Naked new expression matcher
        auto matcher = cxxNewExpr(
            isExpansionInMainFile()
        ).bind("newExpr");

        NewExprCallback callback(findings, source_file, rule);
        finder.addMatcher(matcher, &callback);
    }
    else if (rule.id == "SP-OWN-002") {
        // Naked delete expression matcher
        auto matcher = cxxDeleteExpr(
            isExpansionInMainFile()
        ).bind("deleteExpr");

        DeleteExprCallback callback(findings, source_file, rule);
        finder.addMatcher(matcher, &callback);
    }
    else if (rule.id == "SP-BOUNDS-001") {
        // C-style array declaration matcher
        auto matcher = varDecl(
            hasType(arrayType()),
            isExpansionInMainFile()
        ).bind("arrayDecl");

        CStyleArrayCallback callback(findings, source_file, rule);
        finder.addMatcher(matcher, &callback);
    }
    else if (rule.id == "SP-TYPE-001") {
        // C-style cast matcher
        auto matcher = cStyleCastExpr(
            isExpansionInMainFile()
        ).bind("cStyleCast");

        CStyleCastCallback callback(findings, source_file, rule);
        finder.addMatcher(matcher, &callback);
    }
    else {
        // Unsupported rule
        return findings;
    }

    // Read source file content
    std::ifstream ifs(source_file.string());
    if (!ifs) {
        return findings;  // File not readable
    }

    std::stringstream buffer;
    buffer << ifs.rdbuf();
    std::string source_code = buffer.str();

    // Run Clang tooling
    std::vector<std::string> args = {
        "-std=c++20",
        "-fsyntax-only",
        "-Wno-everything"  // Suppress warnings, we only want AST
    };

    // Create virtual file for analysis
    auto factory = tooling::newFrontendActionFactory(&finder);
    if (!tooling::runToolOnCodeWithArgs(
        factory->create(),
        source_code,
        args,
        source_file.filename().string()
    )) {
        // Analysis failed - might be due to syntax errors
        // Return empty findings rather than crash
        return {};
    }

    return findings;
}

std::vector<ast_finding> ast_detector::analyze_files(
    const std::vector<fs::path>& source_files,
    const std::vector<profile::rule>& rules
) const {
    std::vector<ast_finding> all_findings;

    for (const auto& file : source_files) {
        for (const auto& rule : rules) {
            auto findings = analyze_file(file, rule);
            all_findings.insert(
                all_findings.end(),
                findings.begin(),
                findings.end()
            );
        }
    }

    return all_findings;
}

} // namespace analysis
} // namespace safeprofile
} // namespace boost
