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

#include "plugin_manager.hpp"

#include <dlfcn.h>

#include "context.hpp"
#include "plugin.hpp"

using namespace std::string_literals;

using namespace cod;

// TODO: remove?
// namespace{
// const unsigned soname =
// #include "../../soname.txt"
//     ;
// }

namespace {
plugin* just_loaded_plugin = nullptr;
std::string_view just_loaded_plugin_file_name;

struct plugin_info {
	plugin& instance;
	void* dl_handle;
};

using plugin_list_type = std::list<plugin_info>;

plugin_list_type plugin_list;
} // namespace

void plugin_manager::register_plugin(plugin& p)
{
	if (just_loaded_plugin) {
		std::stringstream ss;
		ss << "tried creating more than one plugin instance while loading plugin shared library: "
		   << just_loaded_plugin_file_name;
		throw std::logic_error(ss.str());
	}
	just_loaded_plugin = &p;
}

namespace {
void load_plugin(const std::string& file_name)
{
	// std::cout << "loading plugin " << file_name << std::endl;

	ASSERT(!just_loaded_plugin)

	// When loading shared library file it will construct static objects, but in case those constructors
	// throw exception, the exception is not thrown by dlopen(), instead it is considered uncaught and terminate() is
	// called. Because it is not possible to catch the exception from outside of dlopen(), we need to "inject"
	// information needed for informative error reporting to the throwing code inside shared library via static
	// variables.

	// save plugin file name for informative error reporting
	just_loaded_plugin_file_name = file_name;

	auto handle = dlopen(
		file_name.c_str(),
		RTLD_NOW | RTLD_GLOBAL // allow global visibility, as some plugins may depend on another ones
	);
	if (handle == nullptr) {
		throw std::runtime_error("could not load plugin: "s + file_name + "\n    " + dlerror());
	}
	ASSERT(just_loaded_plugin)

	plugin_list.push_back(plugin_info{.instance = *just_loaded_plugin, .dl_handle = handle});

	just_loaded_plugin = nullptr;

	// std::cout << "plugin loaded" << std::endl;
}
} // namespace

plugin_manager::plugin_manager(utki::span<const std::string> plugins)
{
	for (const auto& plugin_file_name : plugins) {
		load_plugin(plugin_file_name);
	}
}

plugin_manager::~plugin_manager()
{
	while (!plugin_list.empty()) {
		if (dlclose(plugin_list.back().dl_handle) != 0) {
			ASSERT(false, [](auto& o) {
				o << "dlclose() failed: " << dlerror();
			})
		}
		plugin_list.pop_back();
	}
}

std::shared_ptr<file_page> plugin_manager::open_file(const std::string& file_name)
{
	// std::cout << "plugin_manager::open_file(): enter" << std::endl;
	for (auto i = plugin_list.rbegin(); i != plugin_list.rend(); ++i) {
		// std::cout << "trying plugin" << std::endl;
		auto page = i->instance.open_file(context::inst().gui.morda_context, file_name);
		if (page) {
			return page;
		}
	}

	// std::cout << "plugin_manager::open_file(): return nullptr" << std::endl;
	return nullptr;
}
