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

namespace {
// TODO: fix this lint issue properly somehow?
// NOLINTNEXTLINE(cppcoreguidelines-interfaces-global-init)
const ruisapp::application_factory app_fac([](auto executbale, auto args) -> std::unique_ptr<ruisapp::application> {
	cod::command_line_args cla;

	clargs::parser p;

	bool help = false;

	p.add("help", "display help information", [&help]() {
		help = true;
	});
	p.add("plugin", "load specified plugin file", [&cla](std::string_view file_name) {
		cla.plugins.emplace_back(file_name);
	});

	ASSERT(!args.empty()) // first item is the executable filename

	auto fa = p.parse(args);

	if (help) {
		std::cout << p.description() << std::endl;
		return nullptr;
	}

	if (fa.size() > 1) {
		throw std::invalid_argument("error: more than one directory given");
	}

	if (fa.size() == 1) {
		cla.base_dir = std::move(fa.front());
	} else {
		cla.base_dir = "./";
	}

	// TODO: check that cla.base_dir exists and is a directory

	return std::make_unique<cod::application>(std::move(cla));
});
} // namespace
