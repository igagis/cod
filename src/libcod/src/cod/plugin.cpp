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

#include "plugin.hpp"

#include <functional>

using namespace cod;

plugin::plugin_list_type& plugin::get_plugin_list(){
    static plugin_list_type plugin_list;
    return plugin_list;
}

plugin::plugin(){
    // std::cout << "plugin::contructor(): enter" << std::endl;
    auto& list = get_plugin_list();
    list.push_back(*this);
    this->iter = std::prev(list.end());
}

plugin::~plugin(){
    auto& list = get_plugin_list();
    list.erase(this->iter);
}
