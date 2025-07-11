/*
cod - text editor

Copyright (C) 2021-2025  Ivan Gagis <igagis@gmail.com>

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

#include "application.hpp"

using namespace cod;

namespace {
constexpr auto initial_window_width = 1024;
constexpr auto initial_window_height = 768;
} // namespace

application::application(command_line_args cla) :
	ruisapp::application(
		"cod",
		{
			.dims = {initial_window_width, initial_window_height}
}
	),
	context(std::move(cla), *this)
{}
