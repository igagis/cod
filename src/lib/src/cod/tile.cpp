#include "tile.hpp"

using namespace cod;

tile::tile(std::shared_ptr<morda::context> c, const treeml::forest& desc) :
		morda::widget(std::move(c), desc)
{}

void tile::render(const morda::matrix4& matrix)const{
	
}
