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

#pragma once

#include <ruis/widget/group/tabbed_book.hpp>

#include "page.hpp"
#include "tile.hpp"

namespace cod {

class tabbed_book_tile :
	public tile, //
	private ruis::tabbed_book
{
public:
	struct all_parameters {
		ruis::layout_parameters layout_params;
		ruis::widget::parameters widget_params;
	};

	tabbed_book_tile(
		utki::shared_ref<ruis::context> context, //
		all_parameters params
	);

	void render(const ruis::matrix4& matrix) const override
	{
		this->ruis::tabbed_book::render(matrix);
		this->tile::render(matrix);
	}

	// TODO: pass by value
	void add(const utki::shared_ref<page>& p);

	size_t num_pages() const noexcept
	{
		return this->get_book().size();
	}
};

namespace make {
inline utki::shared_ref<cod::tabbed_book_tile> tabbed_book_tile(
	utki::shared_ref<ruis::context> context,
	cod::tabbed_book_tile::all_parameters params
)
{
	return utki::make_shared<cod::tabbed_book_tile>(
		std::move(context), //
		std::move(params)
	);
}
} // namespace make

} // namespace cod
