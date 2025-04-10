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

#include "tile.hpp"

#include <ruis/context.hpp>

#include "context.hpp"

using namespace cod;

tile::tile(utki::shared_ref<ruis::context> context) :
	ruis::widget(std::move(context), {}, {}),
	selection_vao(this->context.get().renderer)
{}

void tile::render(const ruis::matrix4& matrix) const
{
	// draw selection
	if (this->is_focused()) {
		constexpr auto selection_color = 0xffff8080;
		this->selection_vao.render(matrix, selection_color);
	}
}

void tile::set_selection_vao()
{
	this->selection_vao.set(this->rect().d, 2);
}

void tile::on_focus_change()
{
	if (this->is_focused()) {
		this->set_selection_vao();
	} else {
		// TODO: reset selection_vao to save resources
	}
}

void tile::on_resize()
{
	this->ruis::widget::on_resize();

	if (this->is_focused()) {
		this->set_selection_vao();
	}
}

bool tile::on_key(const ruis::key_event& e)
{
	if (!e.is_down) {
		return false;
	}

	if (context::inst().shortcuts.get("cod.tile.focus_left").combo == e.combo) {
		std::cout << "tile left" << std::endl;
		return true;
	} else if (context::inst().shortcuts.get("cod.tile.focus_right").combo == e.combo) {
		std::cout << "tile right" << std::endl;
		return true;
	} else if (context::inst().shortcuts.get("cod.tile.focus_up").combo == e.combo) {
		std::cout << "tile up" << std::endl;
		return true;
	} else if (context::inst().shortcuts.get("cod.tile.focus_down").combo == e.combo) {
		std::cout << "tile down" << std::endl;
		return true;
	}

	return false;
}
