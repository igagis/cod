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

tile::tile(const utki::shared_ref<ruis::context>& context) :
	ruis::widget(context, {}, {}),
	selection_vao(this->context.get().renderer, {
		.stroke_width = 2, // TODO: take from style?
	})
{}

void tile::render(const ruis::mat4& matrix) const
{
	// draw selection
	if (this->is_focused()) {
		constexpr auto selection_color = 0xffff8080;
		this->selection_vao.render(
			matrix, //
			this->rect().d,
			selection_color
		);
	}
}

ruis::event_status tile::on_key(const ruis::key_event& e)
{
	if (e.action == ruis::button_action::release) {
		return ruis::event_status::propagate;
	}

	if (context::inst().shortcuts.get("cod.tile.focus_left").combo == e.combo) {
		std::cout << "tile left" << std::endl;
		return ruis::event_status::consumed;
	} else if (context::inst().shortcuts.get("cod.tile.focus_right").combo == e.combo) {
		std::cout << "tile right" << std::endl;
		return ruis::event_status::consumed;
	} else if (context::inst().shortcuts.get("cod.tile.focus_up").combo == e.combo) {
		std::cout << "tile up" << std::endl;
		return ruis::event_status::consumed;
	} else if (context::inst().shortcuts.get("cod.tile.focus_down").combo == e.combo) {
		std::cout << "tile down" << std::endl;
		return ruis::event_status::consumed;
	}

	return ruis::event_status::propagate;
}
