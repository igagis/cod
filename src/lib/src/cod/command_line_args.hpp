#pragma once

#include <string>
#include <vector>

namespace cod{
struct command_line_args{
	std::string base_dir;
	std::vector<std::string> plugins;
};
}
