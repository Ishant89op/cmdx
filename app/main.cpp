#include "../core/tool_registry.hpp"
#include "../core/command_builder.hpp"
#include "../core/tool_def.hpp"
#include "../platform/linux/cli.hpp"
#include "../ui/main_window.hpp"
#include <QApplication>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <sstream>
#include <algorithm>
#include <cstdlib>

static std::string prompt(const std::string& message) {
    std::cout << message;
    std::string input;
    std::getline(std::cin, input);
    return input;
}

static std::set<int> parse_number_list(const std::string& input, int max) {
    std::set<int> result;
    if (input == "all") {
        for (int i = 1; i <= max; i++) {
            result.insert(i);
        }
        return result;
    }
    std::istringstream ss(input);
    std::string token;
    while (std::getline(ss, token, ',')) {
        try {
            int n = std::stoi(token);
            if (n >= 1 && n <= max) {
                result.insert(n);
            }
        } catch (...) {}
    }
    return result;
}

static void print_options_table(
    const std::vector<core::option_def>& options,
    const std::vector<core::group_def>& groups
) {
    std::map<std::string, std::string> group_labels;
    for (const auto& g : groups) {
        group_labels[g.id] = g.label;
    }

    std::string current_group;
    int num = 1;

    for (const auto& opt : options) {
        if (opt.group != current_group) {
            current_group = opt.group;
            std::string label = current_group;
            auto it = group_labels.find(current_group);
            if (it != group_labels.end()) {
                label = it->second;
            }
            std::cout << "\n  [" << label << "]" << std::endl;
        }

        std::string req = opt.required ? " *" : "  ";
        std::string type_str;
        switch (opt.type) {
            case core::option_type::BOOL: type_str = "bool"; break;
            case core::option_type::STRING: type_str = "string"; break;
            case core::option_type::INT: type_str = "int"; break;
            case core::option_type::FLOAT: type_str = "float"; break;
            case core::option_type::ENUM: type_str = "enum"; break;
            case core::option_type::FILE_PATH: type_str = "file"; break;
            case core::option_type::DIR_PATH: type_str = "dir"; break;
            case core::option_type::STRING_LIST: type_str = "list"; break;
        }

        printf("  %s %3d. %-6s %-20s %-8s %s\n",
               req.c_str(), num, opt.flag.c_str(), opt.label.c_str(),
               type_str.c_str(), opt.description.c_str());
        num++;
    }
}

static std::string prompt_option_value(const core::option_def& opt) {
    switch (opt.type) {
        case core::option_type::BOOL:
            return "true";

        case core::option_type::ENUM: {
            std::string choices_str;
            for (size_t i = 0; i < opt.choices.size(); i++) {
                if (i > 0) choices_str += "/";
                choices_str += opt.choices[i];
            }
            std::string def_str;
            if (opt.default_value.has_value()) {
                def_str = " (default: " + opt.default_value.value() + ")";
            }
            return prompt("    [" + choices_str + "]" + def_str + ": ");
        }

        case core::option_type::STRING:
        case core::option_type::FILE_PATH:
        case core::option_type::DIR_PATH: {
            std::string def_str;
            if (opt.default_value.has_value()) {
                def_str = " (default: " + opt.default_value.value() + ")";
            }
            return prompt("    Enter value" + def_str + ": ");
        }

        case core::option_type::INT:
        case core::option_type::FLOAT: {
            std::string def_str;
            if (opt.default_value.has_value()) {
                def_str = " (default: " + opt.default_value.value() + ")";
            }
            std::string range_str;
            if (opt.min.has_value() || opt.max.has_value()) {
                range_str = " [";
                if (opt.min.has_value()) range_str += std::to_string((int)opt.min.value());
                range_str += "..";
                if (opt.max.has_value()) range_str += std::to_string((int)opt.max.value());
                range_str += "]";
            }
            return prompt("    Enter value" + range_str + def_str + ": ");
        }

        case core::option_type::STRING_LIST:
            return prompt("    Enter values comma-separated: ");
    }
    return "";
}

static void run_terminal_mode(const std::string& cmd_name) {
    core::tool_registry registry;
    auto tool_opt = registry.load_tool(cmd_name);
    if (!tool_opt.has_value()) {
        std::cerr << "Unknown command: " << cmd_name << std::endl;
        std::cerr << "Available commands:" << std::endl;
        for (const auto& t : registry.list_tools()) {
            std::cerr << "  " << t << std::endl;
        }
        return;
    }

    auto& tool = tool_opt.value();
    std::cout << "\n  " << tool.meta.cmd << " - " << tool.meta.description << "\n" << std::endl;

    const std::vector<core::option_def>* options = &tool.options;
    const std::vector<core::group_def>* groups = &tool.groups;
    const std::vector<core::positional_def>* positionals = &tool.positional;
    const std::vector<core::mutex_group_def>* mutex_groups = &tool.mutex_groups;
    std::optional<std::string> subcommand_name;

    if (tool.meta.has_subcommands && !tool.subcommands.empty()) {
        std::cout << "  Subcommands:" << std::endl;
        for (size_t i = 0; i < tool.subcommands.size(); i++) {
            std::cout << "    " << (i + 1) << ". " << tool.subcommands[i].name
                      << " - " << tool.subcommands[i].description << std::endl;
        }
        std::string pick = prompt("\n  Select subcommand: ");
        int idx = std::stoi(pick) - 1;
        if (idx < 0 || idx >= static_cast<int>(tool.subcommands.size())) {
            std::cerr << "Invalid selection." << std::endl;
            return;
        }
        const auto& sc = tool.subcommands[idx];
        subcommand_name = sc.name;
        options = &sc.options;
        groups = &sc.groups;
        positionals = &sc.positional;
        mutex_groups = &sc.mutex_groups;
        std::cout << "\n  Selected: " << sc.name << std::endl;
    }

    std::cout << "\n  Options (* = required):" << std::endl;
    print_options_table(*options, *groups);

    std::cout << std::endl;
    std::string selection = prompt("  Enter option numbers to configure (e.g. 1,3,5) or 'all': ");
    auto selected = parse_number_list(selection, static_cast<int>(options->size()));

    for (size_t i = 0; i < options->size(); i++) {
        if ((*options)[i].required) {
            selected.insert(static_cast<int>(i + 1));
        }
    }

    std::map<std::string, std::string> values;
    std::cout << std::endl;

    for (int num : selected) {
        int idx = num - 1;
        if (idx < 0 || idx >= static_cast<int>(options->size())) {
            continue;
        }
        const auto& opt = (*options)[idx];
        std::cout << "  " << opt.flag << " (" << opt.label << "): " << opt.description << std::endl;
        std::string value = prompt_option_value(opt);
        if (!value.empty()) {
            values[opt.id] = value;
        } else if (opt.default_value.has_value()) {
            values[opt.id] = opt.default_value.value();
        }
    }

    for (const auto& opt : *options) {
        if (!opt.depends_on.has_value()) {
            continue;
        }
        auto it = values.find(opt.id);
        if (it == values.end() || it->second.empty()) {
            continue;
        }
        auto dep_it = values.find(opt.depends_on.value());
        if (dep_it == values.end() || dep_it->second.empty() || dep_it->second == "false") {
            std::string dep_label;
            for (const auto& o : *options) {
                if (o.id == opt.depends_on.value()) {
                    dep_label = o.label;
                    break;
                }
            }
            std::cout << "\n  Warning: " << opt.label << " depends on " << dep_label
                      << " which is not enabled." << std::endl;
            std::string answer = prompt("  Enable " + dep_label + " automatically? [Y/n]: ");
            if (answer.empty() || answer[0] == 'Y' || answer[0] == 'y') {
                values[opt.depends_on.value()] = "true";
            }
        }
    }

    for (const auto& mg : *mutex_groups) {
        std::vector<std::string> enabled_in_group;
        for (const auto& opt : *options) {
            if (opt.mutex_group.has_value() && opt.mutex_group.value() == mg.id) {
                auto it = values.find(opt.id);
                if (it != values.end() && !it->second.empty() && it->second != "false") {
                    enabled_in_group.push_back(opt.id);
                }
            }
        }
        if (enabled_in_group.size() > 1) {
            std::cout << "\n  Conflict in group '" << mg.label << "': "
                      << mg.description << std::endl;
            for (size_t i = 0; i < enabled_in_group.size(); i++) {
                for (const auto& opt : *options) {
                    if (opt.id == enabled_in_group[i]) {
                        std::cout << "    " << (i + 1) << ". " << opt.label << std::endl;
                    }
                }
            }
            std::string pick_str = prompt("  Keep which one? ");
            int pick = std::stoi(pick_str) - 1;
            for (size_t i = 0; i < enabled_in_group.size(); i++) {
                if (static_cast<int>(i) != pick) {
                    values.erase(enabled_in_group[i]);
                }
            }
        }
    }

    std::map<std::string, std::string> positional_values;
    if (!positionals->empty()) {
        std::cout << "\n  Positional arguments:" << std::endl;
        for (const auto& pos : *positionals) {
            std::string def_str;
            if (pos.default_value.has_value()) {
                def_str = " (default: " + pos.default_value.value() + ")";
            }
            std::string req_str = pos.required ? " *" : "";
            std::string value = prompt("  " + pos.label + req_str + def_str + ": ");
            if (value.empty() && pos.default_value.has_value()) {
                value = pos.default_value.value();
            }
            if (!value.empty()) {
                positional_values[pos.id] = value;
            }
        }
    }

    std::string command = core::build_command(tool, values, subcommand_name, positional_values);
    std::cout << "\n  Command: " << command << std::endl;

    std::string confirm = prompt("\n  Run? [Y/n]: ");
    if (confirm.empty() || confirm[0] == 'Y' || confirm[0] == 'y') {
        platform::run_replace_process(command);
    }
}

static void run_gui_mode(const std::string& cmd_name, int argc, char* argv[]) {
    core::tool_registry registry;
    auto tool_opt = registry.load_tool(cmd_name);
    if (!tool_opt.has_value()) {
        std::cerr << "Unknown command: " << cmd_name << std::endl;
        std::cerr << "Available commands:" << std::endl;
        for (const auto& t : registry.list_tools()) {
            std::cerr << "  " << t << std::endl;
        }
        return;
    }

    QApplication app(argc, argv);
    app.setApplicationName("cmdx");
    app.setApplicationVersion("1.0.0");

    MainWindow window(tool_opt.value());
    window.show();
    app.exec();

    if (window.was_run_requested()) {
        std::string cmd = window.get_command();
        if (!cmd.empty()) {
            platform::run_replace_process(cmd);
        }
    }
}

static void print_usage() {
    std::cout << "Usage:" << std::endl;
    std::cout << "  cmdx options <command>       Terminal mode" << std::endl;
    std::cout << "  cmdx options-gui <command>   GUI mode" << std::endl;
    std::cout << "  cmdx list                    List available commands" << std::endl;
}

int main(int argc, char* argv[]) {
    setenv("LC_ALL", "C.UTF-8", 0);

    if (argc < 2) {
        print_usage();
        return 1;
    }

    std::string mode = argv[1];

    if (mode == "list") {
        core::tool_registry registry;
        auto tools = registry.list_tools();
        std::cout << "Available commands:" << std::endl;
        for (const auto& t : tools) {
            auto tool = registry.load_tool(t);
            if (tool.has_value()) {
                std::cout << "  " << t << " - " << tool->meta.description << std::endl;
            }
        }
        return 0;
    }

    if (mode == "options") {
        if (argc < 3) {
            std::cerr << "Usage: cmdx options <command>" << std::endl;
            return 1;
        }
        run_terminal_mode(argv[2]);
        return 0;
    }

    if (mode == "options-gui") {
        if (argc < 3) {
            std::cerr << "Usage: cmdx options-gui <command>" << std::endl;
            return 1;
        }
        run_gui_mode(argv[2], argc, argv);
        return 0;
    }

    print_usage();
    return 1;
}

