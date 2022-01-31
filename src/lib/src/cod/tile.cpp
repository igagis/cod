#include "tile.hpp"

#include <morda/context.hpp>

using namespace cod;

tile::tile(std::shared_ptr<morda::context> c, const treeml::forest& desc) :
		morda::widget(std::move(c), desc)
{}

void tile::render(const morda::matrix4& matrix)const{
	// draw selection

	// TODO: clean up
	if(this->is_focused()){
	// 	ASSERT(this->select_index < this->content().size())

	// 	const auto& wp = this->content().children()[this->select_index];
	// 	ASSERT(wp)

	// 	const auto& w = *wp;

		if(this->selection_vao.empty()){
			this->selection_vao = morda::frame_vao(this->context->renderer, this->rect().d, 2);
		}

		this->selection_vao.render(matrix, 0xffff8080);
	}
}
