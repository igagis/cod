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

#include <ruis/util/oriented.hpp>
#include <ruis/widgets/container.hpp>

#include "tile.hpp"

namespace cod {

/*
The tile_area arranges tiles either vertially or horizontally.
The tiles are stored in the content container which is the first container child of the tile_area.
The rest of the children are dragger widgets for dragging tile borders within tile_area with mouse.
*/
class tiling_area : public tile, public ruis::oriented, private ruis::container
{
	utki::shared_ref<ruis::container> content_container;

public:
	const ruis::real min_tile_size;
	const ruis::real dragger_size;

	tiling_area(const utki::shared_ref<ruis::context>& c, const tml::forest& desc);

	ruis::container& content()
	{
		return this->content_container.get();
	}

	const ruis::container& content() const
	{
		return this->content_container.get();
	}

	void on_lay_out() override;

	ruis::vector2 measure(const ruis::vector2& quotum) const override;

	// override in order to avoid invalidation of layout when children list changes,
	// because default implementation of this method invalidates layout
	void on_children_change() override
	{
		// do nothing
	}

	void render(const ruis::matrix4& matrix) const override
	{
		this->ruis::container::render(matrix);
		this->tile::render(matrix);
	}

private:
};

} // namespace cod
