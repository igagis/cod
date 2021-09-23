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

#include "tiling_area.hpp"

using namespace cod;

tiling_area::tiling_area(std::shared_ptr<morda::context> c, const treeml::forest& desc) :
        morda::widget(std::move(c), desc),
        morda::oriented_widget(this->context, treeml::forest(), false),
        morda::container(this->context, treeml::forest())
{
    for(const auto& p : desc){
        if(!morda::is_property(p)){
            continue;
        }

        if(p.value == "vertical"){
            this->set_vertical(morda::get_property_value(p).to_bool());
        }
    }

    this->content = std::make_shared<morda::container>(this->context, treeml::forest());
    this->morda::container::push_back(this->content);
    this->content->move_to({0, 0});

    this->content->push_back_inflate(desc);
}

void tiling_area::push_back(std::shared_ptr<widget> w){
    this->content->push_back(std::move(w));
    this->update_draggers();
}

void tiling_area::lay_out(){
    this->morda::container::lay_out();
    this->update_draggers();
}

void tiling_area::on_resize(){
    this->content->resize(this->rect().d);
    this->update_draggers();
}

void tiling_area::update_draggers(){
    // TODO:
}

morda::vector2 tiling_area::measure(const morda::vector2& quotum)const{
    return this->content->measure(quotum);
}
