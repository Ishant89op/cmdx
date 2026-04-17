#include "cli.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <vector>

namespace fs = std::filesystem;

namespace platform {

struct terminal_info {
    std::string name;
    std::string exec_flag;
    std::string hold_suffix;
};

static std::vector<terminal_info> known_terminals() {
    return {
        {"xterm",            "-e", "; exec bash"},
        {"gnome-terminal",   "--", "; exec bash"},
        {"konsole",          "-e", "; exec bash"},
        {"xfce4-terminal",   "-e", "; read -p 'Press Enter to close...'"},
    };
}

static std::string find_terminal() {
    for (const auto& t : known_terminals()) {
        std::string check = "which " + t.name + " > /dev/null 2>&1";
        if (system(check.c_str()) == 0) {
            return t.name;
        }
    }
    return "";
}

static terminal_info get_terminal_info(const std::string& name) {
    for (const auto& t : known_terminals()) {
        if (t.name == name) {
            return t;
        }
    }
    return {"xterm", "-e", "; exec bash"};
}

void run_in_terminal(const std::string& command) {
    std::string term_name = find_terminal();
    if (term_name.empty()) {
        std::cerr << "No supported terminal emulator found." << std::endl;
        return;
    }

    auto info = get_terminal_info(term_name);

    pid_t pid = fork();
    if (pid == 0) {
        if (term_name == "gnome-terminal") {
            execlp(term_name.c_str(), term_name.c_str(),
                   info.exec_flag.c_str(), "bash", "-c", command.c_str(),
                   nullptr);
        } else {
            std::string full_cmd = command;
            execlp(term_name.c_str(), term_name.c_str(),
                   info.exec_flag.c_str(), full_cmd.c_str(),
                   nullptr);
        }
        _exit(1);
    } else if (pid > 0) {
        waitpid(pid, nullptr, 0);
    }
}

void run_in_terminal_interactive(const std::string& command) {
    std::string term_name = find_terminal();
    if (term_name.empty()) {
        std::cerr << "No supported terminal emulator found." << std::endl;
        return;
    }

    auto info = get_terminal_info(term_name);
    std::string full_cmd = command + " " + info.hold_suffix;

    pid_t pid = fork();
    if (pid == 0) {
        if (term_name == "gnome-terminal") {
            execlp(term_name.c_str(), term_name.c_str(),
                   info.exec_flag.c_str(), "bash", "-c", full_cmd.c_str(),
                   nullptr);
        } else {
            execlp(term_name.c_str(), term_name.c_str(),
                   info.exec_flag.c_str(), full_cmd.c_str(),
                   nullptr);
        }
        _exit(1);
    } else if (pid > 0) {
        waitpid(pid, nullptr, 0);
    }
}

void run_replace_process(const std::string& command) {
    execl("/bin/bash", "bash", "-c", command.c_str(), nullptr);
    std::cerr << "Failed to execute: " << command << std::endl;
    _exit(1);
}

} // namespace platform