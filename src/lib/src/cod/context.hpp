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

#pragma once

#include <utki/singleton.hpp>

#include "command_line_args.hpp"
#include "file_opener.hpp"
#include "gui.hpp"
#include "plugin_manager.hpp"
#include "shortcut_resolver.hpp"

namespace cod {

// TODO: make intrusive_singleton
class context : public utki::singleton<context>
{
	friend class application;

	context(command_line_args cla, ruisapp::application& app);

public:
	const std::string base_dir;

	const std::unique_ptr<papki::file> res_file;

	cod::gui gui;

	cod::file_opener file_opener;

	shortcut_resolver shortcuts;

	// this goes as last member to be sure the all the other members are initialized
	// before loading plugins
	plugin_manager plugins;
};

} // namespace cod
