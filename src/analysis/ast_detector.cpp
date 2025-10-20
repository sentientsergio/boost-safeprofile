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
    profile::rule rule_;  // Store by value to avoid dangling reference
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
    profile::rule rule_;  // Store by value to avoid dangling reference
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
    profile::rule rule_;  // Store by value to avoid dangling reference
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
    profile::rule rule_;  // Store by value to avoid dangling reference
};

// Callback for detecting return of reference/pointer to local variable
class ReturnLocalRefCallback : public MatchFinder::MatchCallback {
public:
    ReturnLocalRefCallback(
        std::vector<ast_finding>& findings,
        const fs::path& source_file,
        const profile::rule& rule
    ) : findings_(findings), source_file_(source_file), rule_(rule) {}

    void run(const MatchFinder::MatchResult& result) override {
        const auto* ret_stmt = result.Nodes.getNodeAs<ReturnStmt>("returnStmt");
        const auto* decl_ref = result.Nodes.getNodeAs<DeclRefExpr>("localVar");

        if (!ret_stmt || !decl_ref) return;

        // Get source location
        const SourceManager& sm = *result.SourceManager;
        SourceLocation loc = ret_stmt->getBeginLoc();

        // Skip if not in main file
        if (!sm.isInMainFile(loc)) {
            return;
        }

        unsigned int line = sm.getExpansionLineNumber(loc);
        unsigned int column = sm.getExpansionColumnNumber(loc);

        // Extract code snippet
        std::string snippet = extractSnippet(sm, ret_stmt);

        // Get variable name for better diagnostics
        const auto* var_decl = dyn_cast<VarDecl>(decl_ref->getDecl());
        std::string var_name = var_decl ? var_decl->getNameAsString() : "<unknown>";

        std::string message = rule_.description +
                            " Variable '" + var_name + "' will be destroyed.";

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
    std::string extractSnippet(const SourceManager& sm, const ReturnStmt* stmt) {
        SourceRange range = stmt->getSourceRange();
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

file_analysis_result ast_detector::analyze_file(
    const fs::path& source_file,
    const profile::rule& rule
) const {
    file_analysis_result result;
    result.file = source_file;
    result.success = false;

    std::vector<ast_finding> findings;

    // Set up match finder based on rule ID
    MatchFinder finder;

    // Allocate callback on stack but ensure it lives through the analysis
    std::unique_ptr<MatchFinder::MatchCallback> callback;

    if (rule.id == "SP-OWN-001") {
        // Naked new expression matcher
        auto matcher = cxxNewExpr(
            isExpansionInMainFile()
        ).bind("newExpr");

        callback = std::make_unique<NewExprCallback>(findings, source_file, rule);
        finder.addMatcher(matcher, callback.get());
    }
    else if (rule.id == "SP-OWN-002") {
        // Naked delete expression matcher
        auto matcher = cxxDeleteExpr(
            isExpansionInMainFile()
        ).bind("deleteExpr");

        callback = std::make_unique<DeleteExprCallback>(findings, source_file, rule);
        finder.addMatcher(matcher, callback.get());
    }
    else if (rule.id == "SP-BOUNDS-001") {
        // C-style array declaration matcher
        auto matcher = varDecl(
            hasType(arrayType()),
            isExpansionInMainFile()
        ).bind("arrayDecl");

        callback = std::make_unique<CStyleArrayCallback>(findings, source_file, rule);
        finder.addMatcher(matcher, callback.get());
    }
    else if (rule.id == "SP-TYPE-001") {
        // C-style cast matcher
        auto matcher = cStyleCastExpr(
            isExpansionInMainFile()
        ).bind("cStyleCast");

        callback = std::make_unique<CStyleCastCallback>(findings, source_file, rule);
        finder.addMatcher(matcher, callback.get());
    }
    else if (rule.id == "SP-LIFE-003") {
        // Return reference/pointer to local variable matcher
        // Matches: return &local_var or return local_ref
        // Excludes parameters (they're safe to return)
        auto matcher = returnStmt(
            hasReturnValue(
                anyOf(
                    // Case 1: return &local_var (address-of local)
                    unaryOperator(
                        hasOperatorName("&"),
                        hasUnaryOperand(declRefExpr(
                            to(varDecl(
                                hasAutomaticStorageDuration(),
                                unless(parmVarDecl())  // Exclude parameters
                            ))
                        ).bind("localVar"))
                    ),
                    // Case 2: return local_ref (reference already)
                    declRefExpr(
                        to(varDecl(
                            hasAutomaticStorageDuration(),
                            unless(parmVarDecl())  // Exclude parameters
                        ))
                    ).bind("localVar")
                )
            ),
            isExpansionInMainFile()
        ).bind("returnStmt");

        callback = std::make_unique<ReturnLocalRefCallback>(findings, source_file, rule);
        finder.addMatcher(matcher, callback.get());
    }
    else {
        // Unsupported rule
        result.error_message = "Unsupported rule: " + rule.id;
        return result;
    }

    // Get compiler arguments (from compilation database or defaults)
    std::vector<std::string> args;

    if (compile_db_ && compile_db_->is_loaded()) {
        auto flags_opt = compile_db_->get_flags_for_file(source_file);
        if (flags_opt) {
            args = build_compiler_args(*flags_opt);
        } else {
            // File not in compilation database, use defaults
            args = get_default_compiler_args();
        }
    } else {
        args = get_default_compiler_args();
    }

    return analyze_file_with_flags(source_file, rule, args);
}

file_analysis_result ast_detector::analyze_file_with_flags(
    const fs::path& source_file,
    const profile::rule& rule,
    const std::vector<std::string>& compiler_args
) const {
    file_analysis_result result;
    result.file = source_file;
    result.success = false;

    std::vector<ast_finding> findings;

    // Set up match finder (same logic as before)
    MatchFinder finder;
    std::unique_ptr<MatchFinder::MatchCallback> callback;

    // [Matcher setup code - same as in analyze_file above]
    if (rule.id == "SP-OWN-001") {
        auto matcher = cxxNewExpr(isExpansionInMainFile()).bind("newExpr");
        callback = std::make_unique<NewExprCallback>(findings, source_file, rule);
        finder.addMatcher(matcher, callback.get());
    }
    else if (rule.id == "SP-OWN-002") {
        auto matcher = cxxDeleteExpr(isExpansionInMainFile()).bind("deleteExpr");
        callback = std::make_unique<DeleteExprCallback>(findings, source_file, rule);
        finder.addMatcher(matcher, callback.get());
    }
    else if (rule.id == "SP-BOUNDS-001") {
        auto matcher = varDecl(hasType(arrayType()), isExpansionInMainFile()).bind("arrayDecl");
        callback = std::make_unique<CStyleArrayCallback>(findings, source_file, rule);
        finder.addMatcher(matcher, callback.get());
    }
    else if (rule.id == "SP-TYPE-001") {
        auto matcher = cStyleCastExpr(isExpansionInMainFile()).bind("cStyleCast");
        callback = std::make_unique<CStyleCastCallback>(findings, source_file, rule);
        finder.addMatcher(matcher, callback.get());
    }
    else if (rule.id == "SP-LIFE-003") {
        auto matcher = returnStmt(
            hasReturnValue(
                anyOf(
                    unaryOperator(
                        hasOperatorName("&"),
                        hasUnaryOperand(declRefExpr(
                            to(varDecl(
                                hasAutomaticStorageDuration(),
                                unless(parmVarDecl())
                            ))
                        ).bind("localVar"))
                    ),
                    declRefExpr(
                        to(varDecl(
                            hasAutomaticStorageDuration(),
                            unless(parmVarDecl())
                        ))
                    ).bind("localVar")
                )
            ),
            isExpansionInMainFile()
        ).bind("returnStmt");
        callback = std::make_unique<ReturnLocalRefCallback>(findings, source_file, rule);
        finder.addMatcher(matcher, callback.get());
    }
    else {
        result.error_message = "Unsupported rule: " + rule.id;
        return result;
    }

    // Read source file content
    std::ifstream ifs(source_file.string());
    if (!ifs) {
        result.error_message = "Failed to read file";
        return result;
    }

    std::stringstream buffer;
    buffer << ifs.rdbuf();
    std::string source_code = buffer.str();

    // Run Clang tooling with provided compiler args
    auto factory = tooling::newFrontendActionFactory(&finder);
    if (!tooling::runToolOnCodeWithArgs(
        factory->create(),
        source_code,
        compiler_args,
        source_file.filename().string()
    )) {
        // Analysis failed - compilation error
        result.error_message = "Compilation failed (syntax error, missing includes, or type error)";
        return result;
    }

    // Success!
    result.success = true;
    result.findings = std::move(findings);
    return result;
}

std::vector<ast_finding> ast_detector::analyze_files(
    const std::vector<fs::path>& source_files,
    const std::vector<profile::rule>& rules,
    std::vector<file_analysis_result>& failed_files
) const {
    std::vector<ast_finding> all_findings;
    failed_files.clear();

    for (const auto& file : source_files) {
        for (const auto& rule : rules) {
            auto result = analyze_file(file, rule);

            if (result.success) {
                // Add findings from successful analysis
                all_findings.insert(
                    all_findings.end(),
                    result.findings.begin(),
                    result.findings.end()
                );
            } else {
                // Track failed file (only record once per file, not per rule)
                bool already_recorded = false;
                for (const auto& failed : failed_files) {
                    if (failed.file == file) {
                        already_recorded = true;
                        break;
                    }
                }
                if (!already_recorded) {
                    failed_files.push_back(result);
                }
            }
        }
    }

    return all_findings;
}

std::vector<std::string> ast_detector::get_default_compiler_args() const {
    return {
        "-std=c++20",
        "-fsyntax-only",
        "-Wno-everything"  // Suppress warnings, we only want AST
    };
}

std::vector<std::string> ast_detector::build_compiler_args(
    const intake::compilation_flags& flags
) const {
    std::vector<std::string> args;

    // C++ standard
    args.push_back("-std=" + flags.std_version);

    // Include paths
    for (const auto& include : flags.include_paths) {
        args.push_back("-I" + include);
    }

    // Defines
    for (const auto& define : flags.defines) {
        args.push_back("-D" + define);
    }

    // Always add these
    args.push_back("-fsyntax-only");
    args.push_back("-Wno-everything");

    return args;
}

} // namespace analysis
} // namespace safeprofile
} // namespace boost
