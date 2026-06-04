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

#include "plugin_manager.hpp"

#include <ranges>

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
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
std::vector<plugin*> plugins_to_register;
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
std::string_view plugin_being_loaded_file_name;

struct plugin_info {
	plugin& instance;
	void* dl_handle;
};

using plugin_list_type = std::list<plugin_info>;

// TOOD: make plugin_manager a singleton
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
plugin_list_type plugin_list;
} // namespace

void plugin_manager::register_plugin(plugin& p)
{
	if (!plugin_being_loaded_file_name.empty()) {
		// Plugin filename is not empty means that we are loading a plugin as shared library.
		// In this case, all the built-in plugins should have been already registered, so
		// there should be no plugins to register.
		// If there are some plugins to register, it means the shared library tries to create more
		// than one plugin instance.
		// Check for this situation and throw an exception if it is the case.
		if (!plugins_to_register.empty()) {
			throw std::logic_error(utki::cat(
				"tried creating more than one plugin instance while loading plugin shared library: ",
				plugin_being_loaded_file_name
			));
		}
	}
	plugins_to_register.push_back(&p);
}

namespace {
void register_pending_plugins(void* handle = nullptr)
{
	utki::assert((handle && plugins_to_register.size() == 1) || !handle, [&](auto& o) {
		o << "invalid state: handle is " << (handle ? "not null" : "null")
		  << ", plugins_to_register.size() = " << plugins_to_register.size();
	});

	for (auto& p : plugins_to_register) {
		plugin_list.push_back(plugin_info{
			.instance = *p, //
			.dl_handle = handle
		});
	}

	plugins_to_register.clear();
}
} // namespace

namespace {
void load_plugin(const std::string& file_name)
{
	// std::cout << "loading plugin " << file_name << std::endl;

	utki::assert(plugins_to_register.empty());

	// When loading shared library file it will construct static objects, but in case those constructors
	// throw exception, the exception is not thrown by dlopen(), instead it is considered uncaught and terminate() is
	// called. Because it is not possible to catch the exception from outside of dlopen(), we need to "inject"
	// information needed for informative error reporting to the throwing code inside shared library via static
	// variables.

	// save plugin file name for informative error reporting
	plugin_being_loaded_file_name = file_name;

	auto handle = dlopen(
		file_name.c_str(),
		RTLD_NOW | RTLD_GLOBAL // allow global visibility, as some plugins may depend on another ones
	);
	if (handle == nullptr) {
		throw std::runtime_error("could not load plugin: "s + file_name + "\n    " + dlerror());
	}
	utki::assert(plugins_to_register.size() == 1);
	utki::assert(plugins_to_register.front() != nullptr);

	plugin_being_loaded_file_name = {};

	register_pending_plugins(handle);

	// std::cout << "plugin loaded" << std::endl;
}
} // namespace

plugin_manager::plugin_manager(utki::span<const std::string> plugins)
{
	// At this point built-in plugin static objects should be already constructed and pending registration,
	// so we can register them now.
	register_pending_plugins();

	// register plugins from shared libraries
	for (const auto& plugin_file_name : plugins) {
		load_plugin(plugin_file_name);
	}
}

plugin_manager::~plugin_manager()
{
	while (!plugin_list.empty()) {
		if (plugin_list.back().dl_handle) {
			if (dlclose(plugin_list.back().dl_handle) != 0) {
				utki::assert(false, [](auto& o) {
					o << "dlclose() failed: " << dlerror();
				});
			}
		}
		plugin_list.pop_back();
	}
}

std::shared_ptr<file_page> plugin_manager::open_file(const std::string& file_name)
{
	// std::cout << "plugin_manager::open_file(): enter" << std::endl;
	for (auto& plugin : std::views::reverse(plugin_list)) {
		// std::cout << "trying plugin" << std::endl;
		auto page = plugin.instance.open_file(context::inst().gui.ruis_context, file_name);
		if (page) {
			return page;
		}
	}

	// std::cout << "plugin_manager::open_file(): return nullptr" << std::endl;
	return nullptr;
}
