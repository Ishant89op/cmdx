#ifndef CMDX_TOOL_REGISTRY_HPP
#define CMDX_TOOL_REGISTRY_HPP

#include "tool_def.hpp"
#include <map>
#include <string>
#include <vector>
#include <optional>

namespace core {

class tool_registry {
public:
    tool_registry();

    std::optional<tool_def> load_tool(const std::string& cmd_name);
    std::vector<std::string> list_tools() const;

private:
    std::string commands_dir_;
    std::map<std::string, tool_def> cache_;

    std::string find_commands_dir() const;
};

} // namespace core

#endif // CMDX_TOOL_REGISTRY_HPP
