#pragma once

#include <morda/widgets/group/tabbed_book.hpp>

#include "tile.hpp"

namespace cod{

class editors_book :
		public tile,
		public morda::tabbed_book // TODO: make private
{
public:
	editors_book(std::shared_ptr<morda::context> c, const treeml::forest& desc);

	void render(const morda::matrix4& matrix)const override{
		this->morda::tabbed_book::render(matrix);
		this->tile::render(matrix);
	}
};

}
