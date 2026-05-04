#ifndef CMDX_PLATFORM_CLI_HPP
#define CMDX_PLATFORM_CLI_HPP

#include <string>

namespace platform {

void run_in_terminal(const std::string& command);
void run_in_terminal_interactive(const std::string& command);
void run_replace_process(const std::string& command);

} // namespace platform

#endif // CMDX_PLATFORM_CLI_HPP
