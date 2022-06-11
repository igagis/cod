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
	}

	return false;
}
