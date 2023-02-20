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

#include <mordavokne/application.hpp>

#include "tiling_area.hpp"
#include "tabbed_book_tile.hpp"
#include "file_page.hpp"

namespace cod{

class context;

class gui{
    friend class context;
    friend class file_opener;

    std::shared_ptr<tabbed_book_tile> editors_tabbed_book;

    gui(mordavokne::application& app);

    void open_editor(utki::shared_ref<file_page> page);
public:
    const std::shared_ptr<morda::context> morda_context;
};

}
