#ifndef YINI_REPL_H
#define YINI_REPL_H

#include <string>
#include "Parser/parser.h"

std::string process_repl_command(const std::string& line, Config& config, const std::string& filepath);
void run_repl(Config& config, const std::string& filepath);

#endif // YINI_REPL_H