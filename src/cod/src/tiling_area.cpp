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

#include <morda/context.hpp>
#include <morda/widgets/label/color.hpp>

using namespace cod;

namespace{
const morda::real minimal_tile_size_dp = 100;
const morda::real dragger_size_dp = 5;
}

namespace{
class dragger : public morda::color{
    bool grabbed = false;
    morda::vector2 grab_point;

    tiling_area& owner;

    // TODO: use morda::mouse_cursor_manager::iterator
    std::list<morda::mouse_cursor>::iterator arrows_cursor_iter;
public:
    std::shared_ptr<morda::widget> prev_widget;
    std::shared_ptr<morda::widget> next_widget;

    dragger(std::shared_ptr<morda::context> c, tiling_area& owner) :
            morda::widget(std::move(c), treeml::forest()),
            morda::color(this->context, treeml::forest()),
            owner(owner)
    {
        this->set_color(0xff00ff00);
    }

    bool on_mouse_button(const morda::mouse_button_event& e)override{
        if(e.button != morda::mouse_button::left){
            return false;
        }

        this->grabbed = e.is_down;
        this->grab_point = e.pos;

        if(!this->grabbed){
            if(!this->is_hovered()){
                this->context->cursor_manager.pop(this->arrows_cursor_iter);
            }
        }
        
        return true;
    }

    bool on_mouse_move(const morda::mouse_move_event& e)override{
        if(!this->grabbed){
            return false;
        }

        auto delta = e.pos - this->grab_point;

        // TODO: use this->owner.get_long/trans_index()
        auto trans_index = this->owner.is_vertical() ? 0 : 1;
        auto long_index = this->owner.is_vertical() ? 1 : 0;

        delta[trans_index] = morda::real(0);

        auto new_prev_dims = this->prev_widget->rect().d + delta;
        auto new_next_dims = this->next_widget->rect().d - delta;

        using std::max;

        // clamp new tile dimensions to minimum tile size
        new_prev_dims = max(new_prev_dims, morda::vector2(this->owner.min_tile_size));
        new_next_dims = max(new_next_dims, morda::vector2(this->owner.min_tile_size));

        if(delta[long_index] >= 0){
            delta = min(delta, this->next_widget->rect().d - new_next_dims);
        }else{
            delta = max(delta, new_prev_dims - this->prev_widget->rect().d);
        }

        this->move_by(delta);

        this->prev_widget->resize_by(delta);

        this->next_widget->resize_by(-delta);
        this->next_widget->move_by(delta);
        
        return true;
    }

    void on_hover_change(unsigned pointer_id)override{
        if(this->grabbed){
            return;
        }

        if(this->is_hovered() || grabbed){
            this->arrows_cursor_iter = this->context->cursor_manager.push(
                    this->owner.is_vertical() ?
                            morda::mouse_cursor::up_down_arrow
                        :
                            morda::mouse_cursor::left_right_arrow
                );
        }else{
            this->context->cursor_manager.pop(this->arrows_cursor_iter);
        }
    }

    void render(const morda::matrix4& matrix)const override{
        if(this->is_hovered() || this->grabbed){
            this->morda::color::render(matrix);
        }
    }
};
}

tiling_area::tiling_area(std::shared_ptr<morda::context> c, const treeml::forest& desc) :
        morda::widget(std::move(c), desc),
        morda::oriented_widget(this->context, treeml::forest(), false),
        morda::container(this->context, treeml::forest()),
        min_tile_size(this->context->units.dp_to_px(minimal_tile_size_dp)),
        dragger_size(this->context->units.dp_to_px(dragger_size_dp))
{
    for(const auto& p : desc){
        if(!morda::is_property(p)){
            continue;
        }

        if(p.value == "vertical"){
            this->set_vertical(morda::get_property_value(p).to_bool());
        }
    }

    this->content_container = std::make_shared<morda::container>(this->context, treeml::forest());
    this->morda::container::push_back(this->content_container);
    this->content_container->move_to({0, 0});

    this->content_container->push_back_inflate(desc);
}

void tiling_area::lay_out(){
    auto long_index = this->get_long_index();
    auto trans_index = this->get_trans_index();

    using std::max;

    // calculate current length of all tiles
    morda::real tiles_length = 0;

    for(const auto& t : *this->content_container){
        tiles_length += max(t->rect().d[long_index], this->min_tile_size);
    }

    const auto& content_dims = this->rect().d;

    using std::round;

    // arrange tiles
    if(content_dims[long_index] >= tiles_length){
        morda::vector2 pos{0, 0};
        for(auto& t : *this->content_container){
            morda::real tile_length = max(t->rect().d[long_index], this->min_tile_size);

            ASSERT(tiles_length > 0)

            morda::vector2 dims;
            dims[trans_index] = content_dims[trans_index];
            dims[long_index] = content_dims[long_index] * (tile_length / tiles_length);
            dims = round(dims);
            t->resize(dims);
            t->move_to(pos);
            pos[long_index] += dims[long_index];
        }
    }else{
        morda::real left_length = content_dims[long_index];

        morda::vector2 pos{0, 0};

        for(auto& t : *this->content_container){
            morda::real tile_length = max(t->rect().d[long_index], this->min_tile_size);

            ASSERT(tiles_length > 0)

            morda::vector2 dims;
            dims[trans_index] = content_dims[trans_index];
            dims[long_index] = left_length * (tile_length / tiles_length);
            if(dims[long_index] <= this->min_tile_size){
                dims[long_index] = this->min_tile_size;
            }
            tiles_length -= tile_length;
            left_length -= dims[long_index];
            dims = round(dims);

            t->resize(dims);
            t->move_to(pos);
            pos[long_index] += dims[long_index];
        }
    }

    this->content_container->resize(content_dims);

    // lay out draggers

    ASSERT(this->size() >= 1)

    auto num_draggers = this->content_container->size() == 0 ? 0 : this->content_container->size() - 1;

    // remove redundant draggers
    while(this->size() - 1 > num_draggers){
        this->pop_back();
    }

    // add missing draggers
    while(this->size() - 1 < num_draggers){
        this->push_back(std::make_shared<dragger>(this->context, *this));
    }

    morda::vector2 dragger_dims;
    dragger_dims[long_index] = this->dragger_size;
    dragger_dims[trans_index] = this->rect().d[trans_index];

    for(auto i = std::next(this->begin()); i != this->end(); ++i){
        auto index = size_t(std::distance(this->begin(), i)) - 1;

        ASSERT(index < this->content().size())

        auto& dragger = dynamic_cast<::dragger &>(*(*i));

        dragger.prev_widget = this->content().children()[index];
        dragger.next_widget = this->content().children()[index + 1];

        dragger.resize(dragger_dims);

        morda::vector2 dragger_pos;
        dragger_pos[trans_index] = morda::real(0);
        dragger_pos[long_index] = round(dragger.next_widget->rect().p[long_index] - this->dragger_size / 2);
        dragger.move_to(dragger_pos);
    }
}

morda::vector2 tiling_area::measure(const morda::vector2& quotum)const{
    auto long_index = this->get_long_index();

    morda::vector2 ret;

    for(size_t i = 0; i != quotum.size(); ++i){
        if(quotum[i] < 0){
            ret[i] = this->min_tile_size;

            if(i == long_index){
                ret[i] *= this->content_container->size();
            }
        }else{
            ret[i] = quotum[i];
        }
    }

    return ret;
}
