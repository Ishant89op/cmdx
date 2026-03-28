#include "tool_registry.hpp"
#include "json_parser.hpp"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

namespace fs = std::filesystem;

namespace core {

tool_registry::tool_registry() {
    commands_dir_ = find_commands_dir();
}

std::string tool_registry::find_commands_dir() const {
    // change this path if you move the commands directory
    std::vector<std::string> candidates = {
        "./commands/linux",
        "../commands/linux",
        "../../commands/linux",
    };

    auto exe_path = fs::read_symlink("/proc/self/exe").parent_path();
    candidates.insert(candidates.begin(), (exe_path / "commands" / "linux").string());
    candidates.insert(candidates.begin(), (exe_path / ".." / "commands" / "linux").string());

    for (const auto& path : candidates) {
        if (fs::exists(path) && fs::is_directory(path)) {
            return fs::canonical(path).string();
        }
    }

    return "./commands/linux";
}

std::optional<tool_def> tool_registry::load_tool(const std::string& cmd_name) {
    auto it = cache_.find(cmd_name);
    if (it != cache_.end()) {
        return it->second;
    }

    auto json_path = fs::path(commands_dir_) / (cmd_name + ".json");
    if (!fs::exists(json_path)) {
        return std::nullopt;
    }

    std::ifstream file(json_path);
    if (!file.is_open()) {
        return std::nullopt;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    try {
        auto tool = parse_tool_json(buffer.str());
        cache_[cmd_name] = tool;
        return tool;
    } catch (const std::exception& e) {
        std::cerr << "Failed to parse " << json_path.string() << ": " << e.what() << std::endl;
        return std::nullopt;
    }
}

std::vector<std::string> tool_registry::list_tools() const {
    std::vector<std::string> tools;
    if (!fs::exists(commands_dir_) || !fs::is_directory(commands_dir_)) {
        return tools;
    }

    for (const auto& entry : fs::directory_iterator(commands_dir_)) {
        if (entry.path().extension() == ".json") {
            tools.push_back(entry.path().stem().string());
        }
    }

    std::sort(tools.begin(), tools.end());
    return tools;
}

} // namespace core
