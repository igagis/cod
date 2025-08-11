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

#include "gui.hpp"

#include <ruis/widget/group/tabbed_book.hpp>

#include "context.hpp"
#include "file_tree_page.hpp"
#include "tabbed_book_tile.hpp"

using namespace std::string_literals;
using namespace ruis::length_literals;

using namespace cod;

namespace m {
using namespace ruis::make;
using namespace cod::make;
} // namespace m

namespace {
utki::shared_ref<ruis::widget> make_root_widget(const utki::shared_ref<ruis::context> c)
{
	// clang-format off
	return m::tiling_area(c,
		{
			m::tabbed_book_tile(c,
				{
					.widget_params{
						.id = "left_panel"s,
						.rectangle = {
							{0, 0},
							{(200_pp).get(c.get()), 0} // NOLINT(cppcoreguidelines-avoid-magic-numbers)
						}
					}
				}
			),
			m::tabbed_book_tile(c,
				{
					.widget_params{
						.id = "tabbed_book"s,
						.rectangle = {
							{0, 0},
							{(600_pp).get(c.get()), 0} // NOLINT(cppcoreguidelines-avoid-magic-numbers)
						}
					}
				}
			)
		}
	);
	// clang-format on
}
} // namespace

gui::gui(ruisapp::application& app) :
	ruis_context(app.gui.context)
{
	app.gui.init_standard_widgets(*app.get_res_file());

	app.gui.context.get().loader().mount_res_pack(*app.get_res_file("res/"));

	app.gui.set_root(make_root_widget(app.gui.context));

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
	this->editors_tabbed_book->add(std::move(page));
}
