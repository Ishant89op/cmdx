#ifndef CMDX_COMMAND_CACHE_HPP
#define CMDX_COMMAND_CACHE_HPP

#include <string>
#include <map>
#include <optional>
#include <filesystem>
#include <cstdint>

namespace core {

struct cached_command_config {
    std::string tool;
    std::optional<std::string> subcommand;
    bool sudo = false;
    std::map<std::string, std::string> options;
    std::map<std::string, std::string> positionals;
    std::string command_string;
    int64_t timestamp = 0;
};

class command_cache {
public:
    static std::filesystem::path get_cache_dir();
    static std::filesystem::path get_cache_path(const std::string& tool_name);

    static bool save(const cached_command_config& config);
    static std::optional<cached_command_config> load(const std::string& tool_name);
    static bool clear(const std::string& tool_name);
};

} // namespace core

#endif // CMDX_COMMAND_CACHE_HPP
