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

#pragma once

#include <list>

#include <ruis/widget/group/book.hpp>
#include <utki/singleton.hpp>
#include <utki/span.hpp>

#include "plugin.hpp"

namespace cod {

class plugin_manager : public utki::singleton<plugin_manager>
{
	friend class plugin;

	// called by plugin during loading the plugin
	static void register_plugin(plugin& p);
	static void unregister_plugin(plugin& p);

public:
	plugin_manager(utki::span<const std::string> plugins);
	~plugin_manager() override;

	plugin_manager(const plugin_manager&) = delete;
	plugin_manager& operator=(const plugin_manager&) = delete;

	plugin_manager(plugin_manager&&) = delete;
	plugin_manager& operator=(plugin_manager&&) = delete;

	std::shared_ptr<file_page> open_file(const std::string& file_name);
};

} // namespace cod
