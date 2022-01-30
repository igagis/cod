#include "editors_book.hpp"

using namespace cod;

editors_book::editors_book(std::shared_ptr<morda::context> c, const treeml::forest& desc) :
		morda::widget(std::move(c), desc),
		tile(this->context, desc),
		tabbed_book(this->context, desc)
{

}
