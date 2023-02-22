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

#include <morda/widgets/base/oriented_widget.hpp>
#include <morda/widgets/container.hpp>

#include "tile.hpp"

namespace cod {

/*
The tile_area arranges tiles either vertially or horizontally.
The tiles are stored in the content container which is the first container child of the tile_area.
The rest of the children are dragger widgets for dragging tile borders within tile_area with mouse.
*/
class tiling_area : public tile, public morda::oriented_widget, private morda::container
{
	utki::shared_ref<morda::container> content_container;

public:
	const morda::real min_tile_size;
	const morda::real dragger_size;

	tiling_area(const utki::shared_ref<morda::context>& c, const treeml::forest& desc);

	morda::container& content()
	{
		return this->content_container.get();
	}

	const morda::container& content() const
	{
		return this->content_container.get();
	}

	void lay_out() override;

	morda::vector2 measure(const morda::vector2& quotum) const override;

	// override in order to avoid invalidation of layout when children list changes,
	// because default implementation of this method invalidates layout
	void on_children_change() override
	{
		// do nothing
	}

	void render(const morda::matrix4& matrix) const override
	{
		this->morda::container::render(matrix);
		this->tile::render(matrix);
	}

private:
};

} // namespace cod
