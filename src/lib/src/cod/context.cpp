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

#include "context.hpp"

using namespace cod;

context::context(command_line_args cla, mordavokne::application& app) :
	base_dir(std::move(cla.base_dir)),
	res_file(app.get_res_file()),
	gui(app),
	shortcuts(*app.get_res_file("res/shortcuts.tml")), // TODO: how to inject shortcuts filename?
	plugins(cla.plugins)
{}
