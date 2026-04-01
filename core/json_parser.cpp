#include "json_parser.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace core {

static option_type parse_option_type(const std::string& s) {
    if (s == "BOOL") return option_type::BOOL;
    if (s == "STRING") return option_type::STRING;
    if (s == "INT") return option_type::INT;
    if (s == "FLOAT") return option_type::FLOAT;
    if (s == "ENUM") return option_type::ENUM;
    if (s == "FILE_PATH") return option_type::FILE_PATH;
    if (s == "DIR_PATH") return option_type::DIR_PATH;
    if (s == "STRING_LIST") return option_type::STRING_LIST;
    return option_type::STRING;
}

static positional_type parse_positional_type(const std::string& s) {
    if (s == "DIR_PATH") return positional_type::DIR_PATH;
    if (s == "FILE_PATH") return positional_type::FILE_PATH;
    return positional_type::STRING;
}

static std::optional<std::string> get_optional_string(const json& j, const std::string& key) {
    if (j.contains(key) && !j[key].is_null()) {
        return j[key].get<std::string>();
    }
    return std::nullopt;
}

static std::optional<double> get_optional_double(const json& j, const std::string& key) {
    if (j.contains(key) && !j[key].is_null()) {
        return j[key].get<double>();
    }
    return std::nullopt;
}

static positional_def parse_positional(const json& j) {
    positional_def p;
    p.id = j["id"].get<std::string>();
    p.label = j["label"].get<std::string>();
    p.description = j["description"].get<std::string>();
    p.placeholder = get_optional_string(j, "placeholder");
    p.type = parse_positional_type(j["type"].get<std::string>());
    p.required = j["required"].get<bool>();
    if (j.contains("default") && !j["default"].is_null()) {
        p.default_value = j["default"].get<std::string>();
    }
    p.repeatable = j["repeatable"].get<bool>();
    return p;
}

static group_def parse_group(const json& j) {
    group_def g;
    g.id = j["id"].get<std::string>();
    g.label = j["label"].get<std::string>();
    g.collapsible = j["collapsible"].get<bool>();
    g.collapsed_by_default = j["collapsed_by_default"].get<bool>();
    return g;
}

static mutex_group_def parse_mutex_group(const json& j) {
    mutex_group_def m;
    m.id = j["id"].get<std::string>();
    m.label = j["label"].get<std::string>();
    m.description = j["description"].get<std::string>();
    m.required = j["required"].get<bool>();
    return m;
}

static option_def parse_option(const json& j) {
    option_def o;
    o.id = j["id"].get<std::string>();
    o.flag = j["flag"].get<std::string>();
    o.label = j["label"].get<std::string>();
    o.description = j["description"].get<std::string>();
    o.type = parse_option_type(j["type"].get<std::string>());
    o.required = j["required"].get<bool>();

    if (j.contains("default") && !j["default"].is_null()) {
        if (j["default"].is_boolean()) {
            o.default_value = j["default"].get<bool>() ? "true" : "false";
        } else if (j["default"].is_number()) {
            o.default_value = std::to_string(j["default"].get<double>());
        } else {
            o.default_value = j["default"].get<std::string>();
        }
    }

    o.separator = get_optional_string(j, "separator");
    o.depends_on = get_optional_string(j, "depends_on");
    o.mutex_group = get_optional_string(j, "mutex_group");

    if (j.contains("choices") && !j["choices"].is_null()) {
        for (const auto& c : j["choices"]) {
            o.choices.push_back(c.get<std::string>());
        }
    }

    o.min = get_optional_double(j, "min");
    o.max = get_optional_double(j, "max");
    o.repeatable = j["repeatable"].get<bool>();
    o.placeholder = get_optional_string(j, "placeholder");
    o.group = j["group"].get<std::string>();
    return o;
}

static subcommand_def parse_subcommand(const json& j) {
    subcommand_def s;
    s.name = j["name"].get<std::string>();
    s.description = j["description"].get<std::string>();

    if (j.contains("positional")) {
        for (const auto& p : j["positional"]) {
            s.positional.push_back(parse_positional(p));
        }
    }
    if (j.contains("groups")) {
        for (const auto& g : j["groups"]) {
            s.groups.push_back(parse_group(g));
        }
    }
    if (j.contains("mutex_groups")) {
        for (const auto& m : j["mutex_groups"]) {
            s.mutex_groups.push_back(parse_mutex_group(m));
        }
    }
    if (j.contains("options")) {
        for (const auto& o : j["options"]) {
            s.options.push_back(parse_option(o));
        }
    }
    return s;
}

tool_def parse_tool_json(const std::string& json_content) {
    json j = json::parse(json_content);
    tool_def tool;

    const auto& m = j["meta"];
    tool.meta.cmd = m["cmd"].get<std::string>();
    tool.meta.binary = m["binary"].get<std::string>();
    tool.meta.description = m["description"].get<std::string>();
    tool.meta.version_flag = get_optional_string(m, "version_flag");
    tool.meta.has_subcommands = m["has_subcommands"].get<bool>();
    for (const auto& p : m["platforms"]) {
        tool.meta.platforms.push_back(p.get<std::string>());
    }

    if (j.contains("positional")) {
        for (const auto& p : j["positional"]) {
            tool.positional.push_back(parse_positional(p));
        }
    }

    if (j.contains("groups")) {
        for (const auto& g : j["groups"]) {
            tool.groups.push_back(parse_group(g));
        }
    }

    if (j.contains("mutex_groups")) {
        for (const auto& mg : j["mutex_groups"]) {
            tool.mutex_groups.push_back(parse_mutex_group(mg));
        }
    }

    if (j.contains("subcommands")) {
        for (const auto& s : j["subcommands"]) {
            tool.subcommands.push_back(parse_subcommand(s));
        }
    }

    if (j.contains("options")) {
        for (const auto& o : j["options"]) {
            tool.options.push_back(parse_option(o));
        }
    }

    return tool;
}

} // namespace core