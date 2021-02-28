#pragma once

#include <mordavokne/application.hpp>

#include "code_edit.hpp"
#include "file_tree.hpp"

namespace cod{

struct command_line_arguments{
	std::string base_dir;
};

class application : public mordavokne::application{
public:
	const command_line_arguments cla;

	application(command_line_arguments&& cla);
};

}
