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

#include <morda/paint/frame_vao.hpp>
#include <morda/widget.hpp>

namespace cod {

class tile : virtual public morda::widget
{
	mutable morda::frame_vao selection_vao;

	// only these 2 tiles are supposed to exist, so declare those as friends and make tile constructor private
	friend class tabbed_book_tile;
	friend class tiling_area;

	tile(const utki::shared_ref<morda::context>& c, const treeml::forest& desc);

	void set_selection_vao();

public:
	void render(const morda::matrix4& matrix) const override;

	bool on_key(const morda::key_event& e) override;

	void on_focus_change() override;

	void on_resize() override;
};

} // namespace cod
