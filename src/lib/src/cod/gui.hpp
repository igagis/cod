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

#include <ruisapp/application.hpp>

#include "file_page.hpp"
#include "tabbed_book_tile.hpp"
#include "tiling_area.hpp"

namespace cod {

class context;

class gui
{
	friend class context;
	friend class file_opener;

	std::shared_ptr<tabbed_book_tile> editors_tabbed_book;

	gui(ruisapp::application& app);

	void open_editor(utki::shared_ref<file_page> page);

public:
	const utki::shared_ref<ruis::context> ruis_context;
};

} // namespace cod
