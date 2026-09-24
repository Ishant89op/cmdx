#include "cli.hpp"
#include <cstdlib>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#elif defined(__APPLE__)
#include <unistd.h>
#include <sys/wait.h>
#else
// Linux
#include <unistd.h>
#include <sys/wait.h>
#include <vector>
#include <cstring>
#include <filesystem>
#endif

namespace platform {

#ifdef _WIN32

void run_in_terminal(const std::string& command) {
    std::string full = "cmd.exe /C " + command;
    system(full.c_str());
}

void run_in_terminal_interactive(const std::string& command) {
    std::string full = "cmd.exe /C " + command + " & pause";
    system(full.c_str());
}

void run_replace_process(const std::string& command) {
    std::string full = "cmd.exe /C " + command;
    int rc = system(full.c_str());
    exit(rc);
}

int run_command_interactive(const std::string& command) {
    std::string full = "cmd.exe /C " + command;
    return system(full.c_str());
}

#elif defined(__APPLE__)

void run_in_terminal(const std::string& command) {
    system(command.c_str());
}

void run_in_terminal_interactive(const std::string& command) {
    system(command.c_str());
}

void run_replace_process(const std::string& command) {
    execl("/bin/bash", "bash", "-c", command.c_str(), nullptr);
    std::cerr << "Failed to execute: " << command << std::endl;
    _exit(1);
}

int run_command_interactive(const std::string& command) {
    pid_t pid = fork();
    if (pid == 0) {
        execl("/bin/bash", "bash", "-c", command.c_str(), nullptr);
        _exit(127);
    } else if (pid > 0) {
        int status = 0;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        }
        if (WIFSIGNALED(status)) {
            return 128 + WTERMSIG(status);
        }
        return -1;
    }
    return -1;
}

#else
// Linux

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
            execlp(term_name.c_str(), term_name.c_str(),
                   info.exec_flag.c_str(), command.c_str(),
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

int run_command_interactive(const std::string& command) {
    pid_t pid = fork();
    if (pid == 0) {
        execl("/bin/bash", "bash", "-c", command.c_str(), nullptr);
        _exit(127);
    } else if (pid > 0) {
        int status = 0;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        }
        if (WIFSIGNALED(status)) {
            return 128 + WTERMSIG(status);
        }
        return -1;
    }
    return -1;
}

#endif

} // namespace platform
