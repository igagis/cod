/*
cod - text editor

Copyright (C) 2021-2024  Ivan Gagis <igagis@gmail.com>

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

#include "gui.hpp"

#include <ruis/widget/group/tabbed_book.hpp>

#include "context.hpp"
#include "file_tree_page.hpp"
#include "tabbed_book_tile.hpp"

using namespace cod;

gui::gui(ruisapp::application& app) :
	ruis_context(app.gui.context)
{
	app.gui.init_standard_widgets(*app.get_res_file());

	app.gui.context.get().inflater.register_widget<tiling_area>("tiling_area");
	app.gui.context.get().inflater.register_widget<tabbed_book_tile>("tabbed_book_tile");

	app.gui.context.get().loader.mount_res_pack(*app.get_res_file("res/"));

	app.gui.set_root(app.gui.context.get().inflater.inflate(*app.get_res_file("res/main.gui")));

	auto& c = app.gui.get_root();

	{
		auto ft = utki::make_shared<file_tree_page>(c.context);

		ft.get().file_select_handler = [](std::string file_name) {
			// std::cout << "file = " << file_name << '\n';

			if (papki::is_dir(file_name)) {
				return;
			}

			context::inst().file_opener.open(file_name);
		};

		c.get_widget_as<tabbed_book_tile>("left_panel").add(ft);
	}

	this->editors_tabbed_book = c.try_get_widget_as<tabbed_book_tile>("tabbed_book");
}

void gui::open_editor(utki::shared_ref<file_page> page)
{
	this->editors_tabbed_book->add(page);
}
