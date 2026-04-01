#ifndef CMDX_JSON_PARSER_HPP
#define CMDX_JSON_PARSER_HPP

#include "tool_def.hpp"
#include <string>

namespace core {

tool_def parse_tool_json(const std::string& json_content);

} // namespace core

#endif // CMDX_JSON_PARSER_HPP
