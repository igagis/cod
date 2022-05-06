#include "tabbed_book_tile.hpp"

using namespace cod;

tabbed_book_tile::tabbed_book_tile(std::shared_ptr<morda::context> c, const treeml::forest& desc) :
		morda::widget(std::move(c), desc),
		tile(this->context, desc),
		tabbed_book(this->context, desc)
{

}
