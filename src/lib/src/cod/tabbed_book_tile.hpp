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

#include <morda/widgets/group/tabbed_book.hpp>

#include "tile.hpp"
#include "page.hpp"

namespace cod{

class tabbed_book_tile :
		public tile,
		public morda::tabbed_book
{
public:
	tabbed_book_tile(std::shared_ptr<morda::context> c, const treeml::forest& desc);

	void render(const morda::matrix4& matrix)const override{
		this->morda::tabbed_book::render(matrix);
		this->tile::render(matrix);
	}

	void add(std::shared_ptr<page> p);
};

}
