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

#include "plugin.hpp"

using namespace std::string_literals;

using namespace cod;

namespace{
const unsigned soname =
#include "../../soname.txt"
    ;
}

plugin_manager::plugin_manager(utki::span<const std::string> plugins){
    for(const auto& fn : plugins){
        this->load(fn);
    }
}

void plugin_manager::load(const std::string& file_name){
    // std::cout << "loading plugin " << file_name << std::endl;
    auto handle = dlopen(
            file_name.c_str(),
            RTLD_NOW
                    | RTLD_GLOBAL // allow global visibility, as some plugins may depend on another ones
        );
    if(handle == nullptr){
        throw std::runtime_error("could not load plugin"s + file_name);
    }
    // std::cout << "plugin loaded" << std::endl;
}

std::shared_ptr<morda::page> plugin_manager::open_file(const std::shared_ptr<morda::context> context, const std::string& file_name){
    // std::cout << "plugin_manager::open_file(): enter" << std::endl;
    auto& plugins = plugin::get_plugin_list();
    for(auto i = plugins.rbegin(); i != plugins.rend(); ++i){
        // std::cout << "trying plugin" << std::endl;
        auto page = i->get().open_file(context, file_name);
        if(page){
            return page;
        }
    }

    // std::cout << "plugin_manager::open_file(): return nullptr" << std::endl;
    return nullptr;
}
