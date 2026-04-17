#include "tool_registry.hpp"
#include "json_parser.hpp"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

namespace fs = std::filesystem;

namespace core {

static std::string detect_platform() {
#if defined(__linux__)
    return "linux";
#elif defined(__APPLE__)
    return "macos";
#elif defined(_WIN32)
    return "windows";
#else
    return "linux";
#endif
}

tool_registry::tool_registry() {
    commands_dir_ = find_commands_dir();
}

std::string tool_registry::find_commands_dir() const {
    std::vector<std::string> candidates = {
        "./commands",
        "../commands",
        "../../commands",
    };

    // Also check relative to the executable location
#if defined(__linux__)
    auto exe_path = fs::read_symlink("/proc/self/exe").parent_path();
#elif defined(__APPLE__)
    // _NSGetExecutablePath approach would go here; fallback to cwd
    auto exe_path = fs::current_path();
#elif defined(_WIN32)
    // GetModuleFileName approach would go here; fallback to cwd
    auto exe_path = fs::current_path();
#else
    auto exe_path = fs::current_path();
#endif

    candidates.insert(candidates.begin(), (exe_path / "commands").string());
    candidates.insert(candidates.begin(), (exe_path / ".." / "commands").string());

    for (const auto& path : candidates) {
        if (fs::exists(path) && fs::is_directory(path)) {
            return fs::canonical(path).string();
        }
    }

    return "./commands";
}

static void scan_json_files(const fs::path& dir, std::vector<std::pair<std::string, fs::path>>& results) {
    if (!fs::exists(dir) || !fs::is_directory(dir)) {
        return;
    }

    for (const auto& entry : fs::recursive_directory_iterator(dir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".json") {
            std::string name = entry.path().stem().string();
            results.emplace_back(name, entry.path());
        }
    }
}

std::optional<tool_def> tool_registry::load_tool(const std::string& cmd_name) {
    auto it = cache_.find(cmd_name);
    if (it != cache_.end()) {
        return it->second;
    }

    std::string platform = detect_platform();

    // Search order: platform-specific first, then cross-platform
    std::vector<fs::path> search_dirs = {
        fs::path(commands_dir_) / platform,
        fs::path(commands_dir_) / "cross-platform",
    };

    for (const auto& dir : search_dirs) {
        if (!fs::exists(dir) || !fs::is_directory(dir)) {
            continue;
        }

        // Recursively search subdirectories
        for (const auto& entry : fs::recursive_directory_iterator(dir)) {
            if (!entry.is_regular_file() || entry.path().extension() != ".json") {
                continue;
            }
            if (entry.path().stem().string() != cmd_name) {
                continue;
            }

            std::ifstream file(entry.path());
            if (!file.is_open()) {
                continue;
            }

            std::stringstream buffer;
            buffer << file.rdbuf();

            try {
                auto tool = parse_tool_json(buffer.str());
                cache_[cmd_name] = tool;
                return tool;
            } catch (const std::exception& e) {
                std::cerr << "Failed to parse " << entry.path().string()
                          << ": " << e.what() << std::endl;
                return std::nullopt;
            }
        }
    }

    return std::nullopt;
}

std::vector<std::string> tool_registry::list_tools() const {
    std::string platform = detect_platform();

    std::vector<std::pair<std::string, fs::path>> results;

    // Collect from platform-specific directory
    scan_json_files(fs::path(commands_dir_) / platform, results);

    // Collect from cross-platform directory
    scan_json_files(fs::path(commands_dir_) / "cross-platform", results);

    // Deduplicate (platform-specific wins over cross-platform)
    std::map<std::string, fs::path> unique_tools;
    for (const auto& [name, path] : results) {
        // First occurrence wins (platform-specific is scanned first)
        if (unique_tools.find(name) == unique_tools.end()) {
            unique_tools[name] = path;
        }
    }

    std::vector<std::string> tools;
    for (const auto& [name, _] : unique_tools) {
        tools.push_back(name);
    }

    std::sort(tools.begin(), tools.end());
    return tools;
}

} // namespace core
