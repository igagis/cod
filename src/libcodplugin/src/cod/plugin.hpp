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
#include <memory>

#include <morda/widgets/group/book.hpp>

namespace cod{

class plugin{
    typedef std::list<std::reference_wrapper<plugin>> plugins_list_type;

    static plugins_list_type& get_plugins_list();

    plugins_list_type::iterator iter;
public:
    plugin();
    virtual ~plugin();

    virtual std::shared_ptr<morda::page> open_file(std::string_view file_name){
        return nullptr;
    }
};

}
