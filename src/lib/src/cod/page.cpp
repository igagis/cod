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

#include "page.hpp"

#include "context.hpp"

using namespace cod;

page::page(const utki::shared_ref<morda::context>& context) :
	morda::widget(std::move(context), treeml::forest()),
	morda::page(this->context, treeml::forest())
{}

bool page::on_key(const morda::key_event& e)
{
	if (!e.is_down) {
		return false;
	}

	if (context::inst().shortcuts.get("cod.page.move_left").combo == e.combo) {
		std::cout << "move page left" << std::endl;
		return true;
	} else if (context::inst().shortcuts.get("cod.page.move_right").combo == e.combo) {
		std::cout << "move page right" << std::endl;
		this->context.get().run_from_ui_thread([p = utki::make_shared_from(*this)] {
			p.get().move_right();
		});
		return true;
	} else if (context::inst().shortcuts.get("cod.page.move_up").combo == e.combo) {
		std::cout << "move page up" << std::endl;
		return true;
	} else if (context::inst().shortcuts.get("cod.page.move_down").combo == e.combo) {
		std::cout << "move page down" << std::endl;
		return true;
	}

	return false;
}

void page::move_right()
{
	auto tbt = this->try_get_ancestor<tabbed_book_tile>();
	if (!tbt) {
		// the page is not added to tabbed book tile
		// std::cout << "page is not in book tile" << std::endl;
		return;
	}

	auto ta = tbt->try_get_ancestor<tiling_area>();
	if (!ta) {
		// the tabbed book tile is not in tiling area
		std::cout << "tabbed book tile is not in tiling area" << std::endl;
		return;
	}

	ASSERT(&ta->content() == tbt->parent())

	// auto tbtp = utki::make_shared_from(*tbt);

	std::cout << "actual move right" << std::endl;
}
