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

plugin::plugins_list_type& plugin::get_plugins_list(){
    static plugins_list_type plugins_list;
    return plugins_list;
}

plugin::plugin(){
    auto& list = get_plugins_list();
    list.push_back(*this);
    this->iter = std::prev(list.end());
}

plugin::~plugin(){
    auto& list = get_plugins_list();
    list.erase(this->iter);
}
