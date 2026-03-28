#ifndef CMDX_TOOL_DEF_HPP
#define CMDX_TOOL_DEF_HPP

#include <string>
#include <vector>
#include <optional>

namespace core {

enum class option_type {
    BOOL,
    STRING,
    INT,
    FLOAT,
    ENUM,
    FILE_PATH,
    DIR_PATH,
    STRING_LIST
};

enum class positional_type {
    STRING,
    DIR_PATH,
    FILE_PATH
};

struct positional_def {
    std::string id;
    std::string label;
    std::string description;
    std::optional<std::string> placeholder;
    positional_type type;
    bool required;
    std::optional<std::string> default_value;
    bool repeatable;
};

struct group_def {
    std::string id;
    std::string label;
    bool collapsible;
    bool collapsed_by_default;
};

struct mutex_group_def {
    std::string id;
    std::string label;
    std::string description;
    bool required;
};

struct option_def {
    std::string id;
    std::string flag;
    std::string label;
    std::string description;
    option_type type;
    bool required;
    std::optional<std::string> default_value;
    std::optional<std::string> separator;
    std::optional<std::string> depends_on;
    std::optional<std::string> mutex_group;
    std::vector<std::string> choices;
    std::optional<double> min;
    std::optional<double> max;
    bool repeatable;
    std::optional<std::string> placeholder;
    std::string group;
};

struct meta_def {
    std::string cmd;
    std::string binary;
    std::string description;
    std::optional<std::string> version_flag;
    bool has_subcommands;
    std::vector<std::string> platforms;
};

struct subcommand_def {
    std::string name;
    std::string description;
    std::vector<positional_def> positional;
    std::vector<group_def> groups;
    std::vector<mutex_group_def> mutex_groups;
    std::vector<option_def> options;
};

struct tool_def {
    meta_def meta;
    std::vector<positional_def> positional;
    std::vector<group_def> groups;
    std::vector<mutex_group_def> mutex_groups;
    std::vector<subcommand_def> subcommands;
    std::vector<option_def> options;
};

} // namespace core

#endif // CMDX_TOOL_DEF_HPP
