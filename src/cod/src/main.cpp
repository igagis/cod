/*
cod - text editor

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

#include <clargs/parser.hpp>

#include "application.hpp"

namespace{
mordavokne::application_factory app_fac([](auto args) -> std::unique_ptr<mordavokne::application>{
	cod::command_line_arguments cla;

	clargs::parser p;

	bool help = false;

	p.add("help", "display help information", [&help](){help = true;});
	p.add("plugin", "load specified plugin file", [&cla](std::string&& file_name){cla.plugins.push_back(std::move(file_name));});

	auto fa = p.parse(args);

	if(help){
		std::cout << p.description() << std::endl;
		return nullptr;
	}

	if(fa.size() > 1){
		throw std::invalid_argument("error: more than one directory given");
	}

	if(fa.size() == 1){
		cla.base_dir = std::move(fa.front());
	}else{
		cla.base_dir = "./";
	}

	return std::make_unique<cod::application>(std::move(cla));
});
}
