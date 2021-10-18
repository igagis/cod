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

#include <utki/span.hpp>

#include <morda/widgets/group/book.hpp>

#include "plugin.hpp"

namespace cod{

class plugin_manager{
    void load(const std::string& file_name);
public:
    plugin_manager(utki::span<const std::string> plugins);

    std::shared_ptr<morda::page> open_file(const std::shared_ptr<morda::context> context, const std::string& file_name);
};

}
