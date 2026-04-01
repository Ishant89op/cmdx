#include "command_builder.hpp"
#include <sstream>

namespace core {

static void append_option(std::ostringstream& cmd, const option_def& opt, const std::string& value) {
    if (opt.type == option_type::BOOL) {
        if (value == "true") {
            cmd << " " << opt.flag;
        }
        return;
    }

    if (value.empty()) {
        return;
    }

    if (opt.repeatable) {
        std::istringstream stream(value);
        std::string item;
        while (std::getline(stream, item, ',')) {
            if (!item.empty()) {
                if (opt.separator.has_value()) {
                    cmd << " " << opt.flag << opt.separator.value() << item;
                } else {
                    cmd << " " << opt.flag << " " << item;
                }
            }
        }
        return;
    }

    if (opt.separator.has_value()) {
        cmd << " " << opt.flag << opt.separator.value() << value;
    } else {
        cmd << " " << opt.flag << " " << value;
    }
}

std::string build_command(
    const tool_def& tool,
    const std::map<std::string, std::string>& values,
    const std::optional<std::string>& subcommand_name,
    const std::map<std::string, std::string>& positional_values
) {
    std::ostringstream cmd;
    cmd << tool.meta.binary;

    const std::vector<option_def>* options = &tool.options;
    const std::vector<positional_def>* positionals = &tool.positional;

    const subcommand_def* subcmd = nullptr;
    if (subcommand_name.has_value() && tool.meta.has_subcommands) {
        cmd << " " << subcommand_name.value();
        for (const auto& sc : tool.subcommands) {
            if (sc.name == subcommand_name.value()) {
                subcmd = &sc;
                break;
            }
        }
        if (subcmd) {
            options = &subcmd->options;
            positionals = &subcmd->positional;
        }
    }

    for (const auto& opt : *options) {
        auto it = values.find(opt.id);
        if (it != values.end()) {
            std::string value = it->second;
            if (value.empty() && opt.default_value.has_value()) {
                continue;
            }
            append_option(cmd, opt, value);
        }
    }

    for (const auto& pos : *positionals) {
        auto it = positional_values.find(pos.id);
        if (it != positional_values.end() && !it->second.empty()) {
            if (pos.repeatable) {
                std::istringstream stream(it->second);
                std::string item;
                while (std::getline(stream, item, ',')) {
                    if (!item.empty()) {
                        cmd << " " << item;
                    }
                }
            } else {
                cmd << " " << it->second;
            }
        }
    }

    return cmd.str();
}

} // namespace core
