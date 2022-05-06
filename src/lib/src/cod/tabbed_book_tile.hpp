#pragma once

#include <morda/widgets/group/tabbed_book.hpp>

#include "tile.hpp"

namespace cod{

class tabbed_book_tile :
		public tile,
		public morda::tabbed_book // TODO: make private
{
public:
	tabbed_book_tile(std::shared_ptr<morda::context> c, const treeml::forest& desc);

	void render(const morda::matrix4& matrix)const override{
		this->morda::tabbed_book::render(matrix);
		this->tile::render(matrix);
	}
};

}
