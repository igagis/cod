#include "tile.hpp"

#include <morda/context.hpp>

#include "context.hpp"

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

bool tile::on_key(const morda::key_event& e){
	if(!e.is_down){
		return false;
	}

	if(context::inst().shortcuts.get("cod.tile.focus_left").combo == e.combo){
		std::cout << "tile left" << std::endl;
		return true;
	}else if(context::inst().shortcuts.get("cod.tile.focus_right").combo == e.combo){
		std::cout << "tile right" << std::endl;
		return true;
	}else if(context::inst().shortcuts.get("cod.tile.focus_up").combo == e.combo){
		std::cout << "tile up" << std::endl;
		return true;
	}else if(context::inst().shortcuts.get("cod.tile.focus_down").combo == e.combo){
		std::cout << "tile down" << std::endl;
		return true;
	}else if(context::inst().shortcuts.get("cod.tile.move_left").combo == e.combo){
		std::cout << "move tile left" << std::endl;
		return true;
	}else if(context::inst().shortcuts.get("cod.tile.move_right").combo == e.combo){
		std::cout << "move tile right" << std::endl;
		return true;
	}else if(context::inst().shortcuts.get("cod.tile.move_up").combo == e.combo){
		std::cout << "move tile up" << std::endl;
		return true;
	}else if(context::inst().shortcuts.get("cod.tile.move_down").combo == e.combo){
		std::cout << "move tile down" << std::endl;
		return true;
	}

	return false;
}
