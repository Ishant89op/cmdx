#ifndef CMDX_COMMAND_BUILDER_HPP
#define CMDX_COMMAND_BUILDER_HPP

#include "tool_def.hpp"
#include <map>
#include <string>
#include <vector>
#include <optional>

namespace core {

std::string build_command(
    const tool_def& tool,
    const std::map<std::string, std::string>& values,
    const std::optional<std::string>& subcommand_name = std::nullopt,
    const std::map<std::string, std::string>& positional_values = {}
);

} // namespace core

#endif // CMDX_COMMAND_BUILDER_HPP
