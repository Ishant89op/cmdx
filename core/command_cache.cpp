#include "command_cache.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <chrono>
#include <cstdlib>

namespace core {

std::filesystem::path command_cache::get_cache_dir() {
#ifdef _WIN32
    const char* local_app_data = std::getenv("LOCALAPPDATA");
    if (local_app_data && local_app_data[0] != '\0') {
        return std::filesystem::path(local_app_data) / "cmdx" / "cache" / "tools";
    }
    const char* user_profile = std::getenv("USERPROFILE");
    if (user_profile && user_profile[0] != '\0') {
        return std::filesystem::path(user_profile) / ".cache" / "cmdx" / "tools";
    }
    return std::filesystem::current_path() / ".cache" / "cmdx" / "tools";
#elif defined(__APPLE__)
    const char* home = std::getenv("HOME");
    if (home && home[0] != '\0') {
        return std::filesystem::path(home) / "Library" / "Caches" / "cmdx" / "tools";
    }
    return std::filesystem::current_path() / ".cache" / "cmdx" / "tools";
#else
    const char* xdg_cache = std::getenv("XDG_CACHE_HOME");
    if (xdg_cache && xdg_cache[0] != '\0') {
        return std::filesystem::path(xdg_cache) / "cmdx" / "tools";
    }
    const char* home = std::getenv("HOME");
    if (home && home[0] != '\0') {
        return std::filesystem::path(home) / ".cache" / "cmdx" / "tools";
    }
    return std::filesystem::current_path() / ".cache" / "cmdx" / "tools";
#endif
}

std::filesystem::path command_cache::get_cache_path(const std::string& tool_name) {
    std::string safe_name = std::filesystem::path(tool_name).filename().string();
    return get_cache_dir() / (safe_name + ".json");
}

bool command_cache::save(const cached_command_config& config) {
    if (config.tool.empty()) {
        return false;
    }

    std::filesystem::path dir = get_cache_dir();
    std::error_code ec;
    std::filesystem::create_directories(dir, ec);
    if (ec) {
        return false;
    }

    std::filesystem::path file_path = get_cache_path(config.tool);

    nlohmann::json j;
    j["version"] = 1;
    j["tool"] = config.tool;
    if (config.subcommand.has_value()) {
        j["subcommand"] = *config.subcommand;
    } else {
        j["subcommand"] = nullptr;
    }
    j["sudo"] = config.sudo;
    j["options"] = config.options;
    j["positionals"] = config.positionals;
    j["command_string"] = config.command_string;
    j["timestamp"] = config.timestamp != 0 ? config.timestamp :
        std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();

    std::ofstream out(file_path);
    if (!out.is_open()) {
        return false;
    }

    out << j.dump(2) << std::endl;
    return true;
}

std::optional<cached_command_config> command_cache::load(const std::string& tool_name) {
    if (tool_name.empty()) {
        return std::nullopt;
    }

    std::filesystem::path file_path = get_cache_path(tool_name);
    std::error_code ec;
    if (!std::filesystem::exists(file_path, ec) || ec) {
        return std::nullopt;
    }

    std::ifstream in(file_path);
    if (!in.is_open()) {
        return std::nullopt;
    }

    try {
        nlohmann::json j;
        in >> j;

        cached_command_config config;
        config.tool = j.value("tool", tool_name);

        if (j.contains("subcommand") && j["subcommand"].is_string()) {
            config.subcommand = j["subcommand"].get<std::string>();
        } else {
            config.subcommand = std::nullopt;
        }

        config.sudo = j.value("sudo", false);

        if (j.contains("options") && j["options"].is_object()) {
            for (auto it = j["options"].begin(); it != j["options"].end(); ++it) {
                if (it.value().is_string()) {
                    config.options[it.key()] = it.value().get<std::string>();
                }
            }
        }

        if (j.contains("positionals") && j["positionals"].is_object()) {
            for (auto it = j["positionals"].begin(); it != j["positionals"].end(); ++it) {
                if (it.value().is_string()) {
                    config.positionals[it.key()] = it.value().get<std::string>();
                }
            }
        }

        config.command_string = j.value("command_string", "");
        config.timestamp = j.value("timestamp", 0LL);

        return config;
    } catch (...) {
        return std::nullopt;
    }
}

bool command_cache::clear(const std::string& tool_name) {
    if (tool_name.empty()) {
        return false;
    }
    std::filesystem::path file_path = get_cache_path(tool_name);
    std::error_code ec;
    return std::filesystem::remove(file_path, ec);
}

} // namespace core
