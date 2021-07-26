/*
cod text editor

Copyright (C) 2021  Ivan Gagis <igagis@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/* ================ LICENSE END ================ */

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

	static application& inst(){
		return static_cast<application&>(mordavokne::application::inst());
	}
};

}
