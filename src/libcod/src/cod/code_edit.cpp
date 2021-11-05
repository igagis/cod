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

#include "code_edit.hpp"

#include <algorithm>

#include <utki/string.hpp>
#include <utki/linq.hpp>

#include <morda/widgets/label/text.hpp>
#include <morda/widgets/base/fraction_band_widget.hpp>

using namespace cod;

namespace{
constexpr uint16_t cursor_blink_period_ms = 500;
constexpr morda::real cursor_thickness_dp = 2.0f;
}

code_edit::code_edit(std::shared_ptr<morda::context> c, const treeml::forest& desc) :
		widget(std::move(c), desc),
		character_input_widget(this->context),
		text_widget(this->context, desc),
		column(this->context, treeml::forest()),
		lines_provider(std::make_shared<provider>(*this))
{
	this->set_font(this->context->loader.load<morda::res::font>("fnt_monospace"));
	this->on_font_change();

	this->push_back_inflate(treeml::read(R"qwertyuiop(
			@row{
				layout{dx{fill} dy{0} weight{1}}

				@scroll_area{
					id{scroll_area}
					layout{dx{0} dy{fill} weight{1}}
					clip{true}
					@list{
						id{lines}
						layout{dx{min} dy{fill}}
					}
				}
				@vertical_scroll_bar{
					id{vertical_scroll}

					layout{
						dx{min} dy{max}
					}
				}
			}
			@horizontal_scroll_bar{
				id{horizontal_scroll}

				layout{
					dx{fill} dy{min}
				}
			}
		)qwertyuiop"));

	this->list = utki::make_shared_from(this->get_widget_as<morda::list_widget>("lines"));
	this->list->set_provider(this->lines_provider);

	auto& vs = this->get_widget_as<morda::fraction_band_widget>("vertical_scroll");

	vs.fraction_change_handler =
			[lw = utki::make_weak(this->list)](morda::fraction_widget& fw){
				if(auto w = lw.lock()){
					w->set_scroll_factor(fw.fraction());
				}
			};

	this->list->scroll_change_handler = [sw = utki::make_weak_from(vs)](morda::list_widget& lw){
		if(auto s = sw.lock()){
			s->set_fraction(lw.get_scroll_factor(), false);
			s->set_band_fraction(lw.get_scroll_band());
		}
	};
	
	this->scroll_area = utki::make_shared_from(this->get_widget_as<morda::scroll_area>("scroll_area"));

	auto& hs = this->get_widget_as<morda::fraction_band_widget>("horizontal_scroll");
	
	hs.fraction_change_handler =
			[saw = utki::make_weak(this->scroll_area)](morda::fraction_widget& fw){
				if(auto sa = saw.lock()){
					sa->set_scroll_factor(fw.fraction());
				}
			};
	
	this->scroll_area->scroll_change_handler = [sw = utki::make_weak_from(hs)](morda::scroll_area& sa){
		if(auto s = sw.lock()){
			s->set_fraction(sa.get_scroll_factor().x(), false);
			s->set_band_fraction(sa.get_visible_area_fraction().x());
		}
	};
}

void code_edit::set_text(std::u32string&& text){
	this->lines = utki::linq(utki::split(std::u32string_view(text), U'\n')).select([this](auto&& s){
			auto size = s.size();
			return line{
				.str = std::move(s),
				.spans = {
					synhi::line_span{
						.length = size,
						.style = this->text_style
					}
				}
			};
		}).get();
	this->on_text_change();
}

std::u32string code_edit::get_text()const{
	std::u32string ret;

	for(auto& l : this->lines){
		ret.append(l.str);
	}

	return ret;
}

std::shared_ptr<morda::widget> code_edit::provider::get_widget(size_t index){
	auto w = std::make_shared<code_edit::line_widget>(this->owner.context, this->owner, index);
	return w;
}

namespace{
size_t char_pos_to_glyph_pos(size_t p, const std::u32string& str, size_t tab_size){
	size_t x = 0;
	for(size_t i = 0; i < str.size() && i != p; ++i){
		ASSERT(i < str.size())
		if(str[i] == U'\t'){
			x += tab_size - x % tab_size;
		}else{
			++x;
		}
	}
	return x;
}
}

namespace{
size_t string_length_glyphs(const std::u32string& str, size_t tab_size){
	return char_pos_to_glyph_pos(str.size(), str, tab_size);
}
}

void code_edit::line_widget::render(const morda::matrix4& matrix)const{	
	// find cursors
	auto cursors = this->owner.find_cursors(this->line_num);

	// render selection
	for(auto c : cursors){
		auto& sel = std::get<cursor::selection>(c).segment;

		if(sel.p1.y() > this->line_num || this->line_num > sel.p2.y()){
			continue;
		}

		size_t start, length;
		if(sel.p1.y() == this->line_num){
			start = sel.p1.x();
		}else{
			start = 0;
		}
		if(sel.p2.y() == this->line_num){
			length = sel.p2.x() - start;
		}else{
			length = string_length_glyphs(
					this->owner.lines[this->line_num].str,
					this->owner.settings.tab_size
				) - start;
		}

		morda::matrix4 matr(matrix);

		auto pos = morda::real(start) * this->owner.font_info.glyph_dims.x();
		matr.translate(pos, 0);
		matr.scale(morda::vector2(morda::real(length), 1).comp_mul(this->owner.font_info.glyph_dims));

		auto& r = *this->context->renderer;
		r.shader->color_pos->render(matr, *r.pos_quad_01_vao, 0xff804000);
	}

	const auto& l = this->owner.lines[this->line_num];
	const auto& str = l.str;

	// render text
	size_t cur_char_pos = 0;
	size_t cur_char_index = 0;
	for(const auto& s : l.spans){
		const auto& font = this->owner.get_font().get(s.style->style);

		morda::matrix4 matr(matrix);
		matr.translate(
				cur_char_pos * this->owner.font_info.glyph_dims.x(),
				this->owner.font_info.baseline
			);
		auto res = font.render(
				matr,
				morda::color_to_vec4f(s.style->color),
				std::u32string_view(str.c_str() + cur_char_index, s.length),
				this->owner.settings.tab_size,
				cur_char_pos
			);
		cur_char_index += s.length;
		cur_char_pos += res.length;
	}

	// render cursors
	if(this->owner.cursor_blink_visible){
		for(auto c : cursors){
			auto cp = std::get<cursor::selection>(c).get_cursor_pos_glyphs();

			if(cp.y() != this->line_num){
				continue;
			}

			morda::matrix4 matr(matrix);

			auto pos = morda::real(cp.x()) * this->owner.font_info.glyph_dims.x();
			matr.translate(pos, 0);
			matr.scale(morda::vector2(cursor_thickness_dp * this->context->units.dots_per_dp, this->owner.font_info.glyph_dims.y()));

			auto& r = *this->context->renderer;
			r.shader->color_pos->render(matr, *r.pos_quad_01_vao, 0xffffffff);
		}
	}
}

morda::vector2 code_edit::line_widget::measure(const morda::vector2& quotum)const noexcept{
	morda::vector2 ret = this->owner.font_info.glyph_dims;
	ret.x() *= this->owner.lines[this->line_num].str.size() + 1; // for empty strings the widget will still have size of one glyph

	for(unsigned i = 0; i != ret.size(); ++i){
		if(quotum[i] >= 0){
			ret[i] = quotum[i];
		}
	}

	return ret;
}

void code_edit::update(uint32_t dt){
	this->cursor_blink_visible = !this->cursor_blink_visible;
}

void code_edit::on_focus_change(){
	if(this->is_focused()){
		this->start_cursor_blinking();
	}else{
		this->context->updater->stop(*this);
	}
}

void code_edit::start_cursor_blinking(){
	this->context->updater->stop(*this);
	this->cursor_blink_visible = true;
	this->context->updater->start(
			utki::make_weak_from(*static_cast<updateable*>(this)),
			cursor_blink_period_ms
		);
}

void code_edit::for_each_cursor(const std::function<void(cursor&)>& func){
	ASSERT(func)
	for(auto& c : this->cursors){
		func(c);
		// TODO: check if cursors do not intersect
	}
	if(this->text_changed){
		this->text_changed = false;
		this->notify_text_change();
	}
}

std::vector<std::tuple<const code_edit::cursor*, code_edit::cursor::selection>> code_edit::find_cursors(size_t line_num){
	std::vector<std::tuple<const cursor*, cursor::selection>> ret;

	for(auto& c : this->cursors){
		auto s = c.get_selection_glyphs();
		if(s.segment.p1.y() <= line_num && line_num <= s.segment.p2.y()){
			ret.push_back(std::make_tuple(&c, s));
		}
	}

	return ret;
}

void code_edit::insert(cursor& c, const std::u32string& str){
	auto strs = utki::split(std::u32string_view(str), U'\n');
	ASSERT(!strs.empty())

	auto cp = c.get_pos_chars();
	c.set_pos_chars(cp);

	if(strs.size() == 1){
		auto& l = this->lines[cp.y()];
		l.str.insert(cp.x(), strs.front());
		l.extend_span(cp.x(), strs.front().size());
		c.move_right_by(strs.front().size());
	}else{
		// TODO: multiline insert (from clipboard)
	}

	// TODO: correct cursors

	this->text_changed = true;
}

void code_edit::erase_forward(cursor& c, size_t num){
	auto cp = c.get_pos_chars();

	auto& l = this->lines[cp.y()];

	ASSERT(cp.x() <= l.size())
	if(cp.x() == l.size()){
		ASSERT(cp.y() < this->lines.size())
		if(cp.y() + 1 == this->lines.size()){
			return;
		}
		auto i = std::next(this->lines.begin(), cp.y() + 1);
		auto ll = std::move(*i);
		this->lines.erase(i);
		l.append(std::move(ll));
	}else{
		size_t s;
		ASSERT(cp.x() <= l.size());
		size_t to_end = l.size() - cp.x();
		if(num > to_end){
			s = to_end;
		}else{
			s = num;
		}
		l.erase(cp.x(), s);
	}

	// TODO: correct cursors

	this->text_changed = true;
}

void code_edit::erase_backward(cursor& c, size_t num){
	auto cp = c.get_pos_chars();

	if(cp.x() == 0){
		if(cp.y() == 0){
			return;
		}
		auto i = std::next(this->lines.begin(), cp.y());
		auto ll = std::move(*i);
		this->lines.erase(i);
		--cp.y();
		auto& l = this->lines[cp.y()];
		cp.x() = l.size();
		l.append(std::move(ll));
		c.set_pos_chars(cp);
	}else{
		auto& l = this->lines[cp.y()];
		size_t p;
		size_t s;
		if(cp.x() >= num){
			p = cp.x() - num;
			s = num;
		}else{
			p = 0;
			s = cp.x();
		}
		cp.x() -= s;
		c.set_pos_chars(cp);

		l.erase(p, s);
	}

	// TODO: correct cursors

	this->text_changed = true;
}

void code_edit::put_new_line(cursor& c){
	auto cp = c.get_pos_chars();

	ASSERT(cp.y() < this->lines.size())

	auto i = std::next(this->lines.begin(), cp.y());

	auto nl = i->cut_tail(cp.x());
	this->lines.insert(std::next(i), std::move(nl));

	++cp.y();
	cp.x() = 0;
	c.set_pos_chars(cp);

	// TODO: correct cursors

	this->text_changed = true;
}

void code_edit::render(const morda::matrix4& matrix)const{
	this->base_container::render(matrix);
}

r4::vector2<size_t> code_edit::mouse_pos_to_glyph_pos(const morda::vector2& mouse_pos)const noexcept{
	auto corrected_mouse_pos = mouse_pos +
			morda::vector2{
				this->scroll_area->get_scroll_pos().x(),
				this->list->get_pos_offset()
			};

	using std::max;
	corrected_mouse_pos = max(corrected_mouse_pos, 0); // clamp to positive values

	// LOG("corrected_pos = " << corrected_pos << std::endl)

	using std::round;
	using std::floor;
	auto glyph_pos_real = corrected_mouse_pos.comp_div(this->font_info.glyph_dims);
	glyph_pos_real.x() = round(glyph_pos_real.x());
	glyph_pos_real.y() = floor(glyph_pos_real.y());
	auto glyph_pos = glyph_pos_real.to<size_t>();
	glyph_pos.y() += this->list->get_pos_index();

	return glyph_pos;
}

bool code_edit::on_mouse_button(const morda::mouse_button_event& event){
	if(this->base_container::on_mouse_button(event)){
		return true;
	}

	morda::real scroll_direction = 1;
	switch(event.button){
		case morda::mouse_button::left:
			if(event.is_down){
				this->cursors.clear();

				this->cursors.push_back(cursor(*this, this->mouse_pos_to_glyph_pos(event.pos)));
				this->mouse_selection = true;

				this->focus();
				this->start_cursor_blinking();
			}else{
				this->mouse_selection = false;
			}
			break;
		case morda::mouse_button::wheel_up:
			scroll_direction = -1;
			[[fallthrough]];
		case morda::mouse_button::wheel_down:
			if(event.is_down){
				this->list->scroll_by(
						scroll_direction * this->font_info.glyph_dims.y() * 3
					);
			}
			break;
		case morda::mouse_button::wheel_left:
			scroll_direction = -1;
			[[fallthrough]];
		case morda::mouse_button::wheel_right:
			if(event.is_down){
				this->scroll_area->set_scroll_pos(
					this->scroll_area->get_scroll_pos() + morda::vector2{
						scroll_direction * this->font_info.glyph_dims.x() * 3,
						0
					}
				);
			}
			break;
		default:
			return false;
	}
	
	return true;
}

bool code_edit::on_mouse_move(const morda::mouse_move_event& event){
	if(this->base_container::on_mouse_move(event)){
		return true;
	}

	if(this->mouse_selection){
		ASSERT(!this->cursors.empty())
		this->cursors.front().set_pos_glyphs(this->mouse_pos_to_glyph_pos(event.pos));
		return true;
	}
	return false;
}

void code_edit::cursor::update_selection(){
	bool selection = this->owner.mouse_selection || this->owner.modifiers.get(code_edit::modifier::selection);
	if(selection){
		return;
	}
	this->sel_pos_glyphs = this->get_pos_glyphs();
}

namespace{
size_t glyph_pos_to_char_pos(size_t p, const std::u32string& str, size_t tab_size){
	size_t x = 0;
	size_t i = 0;
	for(; i != str.size(); ++i){
		size_t d;

		if(str[i] == U'\t'){
			d = tab_size - x % tab_size;
		}else{
			d = 1;
		}

		size_t px = p - x;
		if(px <= d){
			if(px <= d / 2)
				return i;
			else
				return i + 1;
		}

		x += d;
	}
	return i;
}
}

size_t code_edit::cursor::get_line_num()const noexcept{
	ASSERT(!this->owner.lines.empty())
	if(this->pos.y() >= this->owner.lines.size()){
		return this->owner.lines.size() - 1;
	}
	return this->pos.y();
}

code_edit::cursor::selection code_edit::cursor::get_selection_glyphs()const noexcept{
	selection ret;

	auto cp = this->get_pos_glyphs();

	if(cp.y() < this->sel_pos_glyphs.y() || (cp.y() == this->sel_pos_glyphs.y() && cp.x() < this->sel_pos_glyphs.x())){
		ret.segment.p1 = cp;
		ret.segment.p2 = this->sel_pos_glyphs;
		ret.is_left_to_right = false;
	}else{
		ret.segment.p1 = this->sel_pos_glyphs;
		ret.segment.p2 = cp;
		ret.is_left_to_right = true;
	}

	return ret;
}

r4::vector2<size_t> code_edit::cursor::get_pos_chars()const noexcept{
	size_t line_num = this->get_line_num();

	ASSERT(line_num < this->owner.lines.size())
	
	using std::min;

	ASSERT(!this->owner.lines.empty())
	return {
		glyph_pos_to_char_pos(
				this->pos.x(),
				this->owner.lines[line_num].str,
				this->owner.settings.tab_size
			),
		min(this->pos.y(), this->owner.lines.size() - 1)
	};
}

void code_edit::cursor::set_pos_chars(r4::vector2<size_t> p)noexcept{
	ASSERT(!this->owner.lines.empty())
	ASSERT(p.y() < this->owner.lines.size())

	// LOG("p = " << p << std::endl)

	this->pos = {
		char_pos_to_glyph_pos(
				p.x(),
				this->owner.lines[p.y()].str,
				this->owner.settings.tab_size
			),
		p.y()
	};

	this->update_selection();

	this->owner.scroll_to(this->pos);

	this->owner.start_cursor_blinking();
}

void code_edit::cursor::set_pos_glyphs(r4::vector2<size_t> p)noexcept{
	this->pos = p;

	this->update_selection();
}

r4::vector2<size_t> code_edit::cursor::get_pos_glyphs()const noexcept{
	ASSERT(!this->owner.lines.empty())
	auto p = this->get_pos_chars();

	ASSERT_INFO(p.y() < this->owner.lines.size(), "this->owner.lines.size() = " << this->owner.lines.size() << ", p.y() = " << p.y())

	return {
		char_pos_to_glyph_pos(
				p.x(),
				this->owner.lines[p.y()].str,
				this->owner.settings.tab_size
			),
		p.y()
	};
}

bool code_edit::on_key(bool is_down, morda::key key){
	switch(key){
		case morda::key::left_control:
		case morda::key::right_control:
			this->modifiers.set(modifier::word_navigation, is_down);
			break;
		case morda::key::left_shift:
		case morda::key::right_shift:
			this->modifiers.set(modifier::selection, is_down);
			break;
		default:
			break;
	}
	return false;
}

void code_edit::line::extend_span(size_t at_char_index, size_t by_length){
	size_t cur_span_end = 0;
	for(auto& s : this->spans){
		cur_span_end += s.length;
		if(at_char_index < cur_span_end){
			s.length += by_length;
			return;
		}
	}
	// extend last span if adding chars to the end of the string
	ASSERT(!this->spans.empty())
	this->spans.back().length += by_length;
}

void code_edit::line::erase_spans(size_t at_char_index, size_t by_length){
	size_t span_end = 0;
	for(auto i = this->spans.begin(); i != this->spans.end() && by_length != 0;){
		span_end += i->length;
		if(at_char_index < span_end){
			size_t to_end = span_end - at_char_index;
			ASSERT(to_end <= i->length)
			if(by_length < to_end){
				ASSERT(i->length > by_length)
				i->length -= by_length;
				return;
			}else{
				by_length -= to_end;
				if(to_end == i->length){
					if(this->spans.size() == 1){ // do not remove the last span
						this->spans.back().length = 0;
						break;
					}
					// remove span
					i = this->spans.erase(i);
					continue;
				}else{
					ASSERT(to_end < i->length)
					i->length -= to_end;
				}
			}
		}
		++i;
	}
}

void code_edit::line::append(line&& l){
	this->str.append(std::move(l.str));
	this->spans.insert(
			this->spans.end(),
			std::make_move_iterator(l.spans.begin()),
			std::make_move_iterator(l.spans.end())
		);
}

code_edit::line code_edit::line::cut_tail(size_t pos){
	if(pos >= this->size()){
		return line{
			spans: {synhi::line_span{ style: this->spans.back().style }}
		};
	}

	line ret;
	ret.str = this->str.substr(pos);
	this->str = this->str.substr(0, pos);

	size_t cur_span_end = 0;
	for(auto i = this->spans.begin(); i != this->spans.end(); ++i){
		cur_span_end += i->length;
		if(pos < cur_span_end){
			size_t to_end = cur_span_end - pos;
			ret.spans.push_back(synhi::line_span(*i));
			ret.spans.back().length = to_end;
			i->length -= to_end;
			// LOG("this->spans.size() = " << this->spans.size() << std::endl)
			// LOG("ret.spans.size() = " << ret.spans.size() << std::endl)
			// LOG("dst = " << std::distance(this->spans.begin(), i) << std::endl)
			ret.spans.insert(
					ret.spans.end(),
					std::make_move_iterator(std::next(i)),
					std::make_move_iterator(this->spans.end())
				);
			this->spans.erase(std::next(i), this->spans.end());
			// LOG("this->spans.size() = " << this->spans.size() << std::endl)
			// LOG("ret.spans.size() = " << ret.spans.size() << std::endl)

			// for(auto& s : this->spans){
			// 	ASSERT(s.attrs)
			// }
			// LOG("ret.spans.size() = " << ret.spans.size() << std::endl)
			// for(auto& s : ret.spans){
			// 	ASSERT(s.attrs);
			// }

			return ret;
		}
	}

	ASSERT(false)
	return ret;
}

void code_edit::scroll_to(r4::vector2<size_t> pos_glyphs){
	// vertical
	size_t top = this->list->get_pos_index();
	if(top >= pos_glyphs.y()){
		this->list->scroll_by(
				-morda::real(top - pos_glyphs.y()) * this->font_info.glyph_dims.y()
				- this->list->get_pos_offset()
			);
	}else{
		ASSERT(!this->list->children().empty())
		size_t bottom = top + this->list->children().size() - 1;
		if(bottom <= pos_glyphs.y()){
			morda::real bottom_offset = this->list->children().back()->rect().y2() - this->list->rect().d.y();
			this->list->scroll_by(
					morda::real(pos_glyphs.y() - bottom) * this->font_info.glyph_dims.y()
					+ bottom_offset
				);
		}
	}

	// horizontal
	morda::real pos_x = pos_glyphs.x() * this->font_info.glyph_dims.x();

	morda::real left = this->scroll_area->get_scroll_pos().x();
	if(left > pos_x){
		this->scroll_area->set_scroll_pos({pos_x, 0});
	}else{
		pos_x -= this->scroll_area->rect().d.x() - this->font_info.glyph_dims.x();
		if(left < pos_x){
			this->scroll_area->set_scroll_pos({pos_x, 0});
		}
	}
}

void code_edit::cursor::move_right_by(size_t dx)noexcept{
	auto p = this->get_pos_chars();
	p.x() += dx;
	auto line_size = this->owner.lines[p.y()].str.size();

	ASSERT(!this->owner.lines.empty())
	for(; p.x() > line_size;){
		if(p.y() < this->owner.lines.size() - 1){
			p.x() -= line_size + 1;
			++p.y();
			line_size = this->owner.lines[p.y()].str.size();
		}else{
			p.x() = line_size;
			break;
		}
	}
	this->set_pos_chars(p);
}

void code_edit::cursor::move_left_by(size_t dx)noexcept{
	auto p = this->get_pos_chars();
	for(; dx > p.x();){
		if(p.y() == 0){
			p.x() = 0;
			this->set_pos_chars(p);
			return;
		}else{
			dx -= p.x() + 1;
			--p.y();
			p.x() = this->owner.lines[p.y()].str.size();
		}
	}
	// LOG("p = " << p << " dx = " << dx << std::endl)
	p.x() -= dx;

	this->set_pos_chars(p);
}

void code_edit::cursor::move_up_by(size_t dy)noexcept{
	if(this->pos.y() < dy){
		this->pos.y() = 0;
	}else{
		this->pos.y() -= dy;
	}

	this->update_selection();

	this->owner.scroll_to(this->get_pos_glyphs());

	this->owner.start_cursor_blinking();
}

void code_edit::cursor::move_down_by(size_t dy)noexcept{
	size_t max_y = this->owner.lines.size() - 1;
	if(max_y - this->pos.y() < dy){
		this->pos.y() = max_y;
	}else{
		this->pos.y() += dy;
	}

	this->update_selection();

	this->owner.scroll_to(this->get_pos_glyphs());

	this->owner.start_cursor_blinking();
}

size_t code_edit::num_lines_on_page()const noexcept{
	using std::floor;
	return size_t(floor(this->list->rect().d.y() / this->font_info.glyph_dims.y()));
}

size_t code_edit::calc_word_length_forward(const cursor& c)const noexcept{
	size_t ret = 0;

	auto cp = c.get_pos_chars();

	if(cp.x() == this->lines[cp.y()].size()){
		++cp.y();
		if(cp.y() == this->lines.size()){
			return 0;
		}
		++ret; // for new line
		cp.x() = 0;
	}

	auto& l = this->lines[cp.y()].str;

	bool non_ws_met = false;
	for(auto i = std::next(l.begin(), cp.x()); i != l.end(); ++i, ++ret){
		if(non_ws_met){
			if(std::isspace(*i)){
				break;
			}
		}else{
			if(!std::isspace(*i)){
				non_ws_met = true;
			}
		}
	}

	return ret;
}

size_t code_edit::calc_word_length_backward(const cursor& c)const noexcept{
	size_t ret = 0;

	auto cp = c.get_pos_chars();

	if(cp.x() == 0){
		if(cp.y() == 0){
			return 0;
		}
		--cp.y();
		cp.x() = this->lines[cp.y()].size();
		++ret; // for new line
	}

	auto& l = this->lines[cp.y()].str;

	bool non_ws_met = false;

	ASSERT(cp.x() <= l.size())
	for(auto i = std::next(l.rbegin(), l.size() - cp.x()); i != l.rend(); ++i, ++ret){
		if(non_ws_met){
			if(std::isspace(*i)){
				break;
			}
		}else{
			if(!std::isspace(*i)){
				non_ws_met = true;
			}
		}
	}

	return ret;
}

void code_edit::on_character_input(const std::u32string& unicode, morda::key key){
	switch(key){
		case morda::key::enter:
			this->for_each_cursor([this](cursor& c){
				this->put_new_line(c);
			});
			break;
		case morda::key::right:
			this->for_each_cursor([this](cursor& c){
				size_t d = 1;
				if(this->modifiers.get(code_edit::modifier::word_navigation)){
					d = this->calc_word_length_forward(c);
				}
				c.move_right_by(d);
			});
			break;
		case morda::key::left:
			this->for_each_cursor([this](cursor& c){
				size_t d = 1;
				if(this->modifiers.get(code_edit::modifier::word_navigation)){
					d = this->calc_word_length_backward(c);
				}
				c.move_left_by(d);
			});
			break;
		case morda::key::up:
			this->for_each_cursor([](cursor& c){
				c.move_up_by(1);
			});
			break;
		case morda::key::down:
			this->for_each_cursor([](cursor& c){
				c.move_down_by(1);
			});
			break;
		case morda::key::page_up:
			this->for_each_cursor([n = this->num_lines_on_page()](cursor& c){
				c.move_up_by(n);
			});
			break;
		case morda::key::page_down:
			this->for_each_cursor([n = this->num_lines_on_page()](cursor& c){
				c.move_down_by(n);
			});
			break;
		case morda::key::end:
			this->for_each_cursor([this](cursor& c){
				auto p = c.get_pos_chars();
				p.x() = this->lines[p.y()].str.size();
				c.set_pos_chars(p);
			});
			break;
		case morda::key::home:
			this->for_each_cursor([this](cursor& c){
				auto p = c.get_pos_chars();
				auto& l = this->lines[p.y()].str;
				auto non_ws_pos = l.find_first_not_of(U" \t");
				if(non_ws_pos == std::remove_reference_t<decltype(l)>::npos || p.x() == non_ws_pos){
					p.x() = 0;
				}else{
					p.x() = non_ws_pos;
				}
				c.set_pos_chars(p);
			});
			break;
		case morda::key::backspace:
			this->for_each_cursor([this](cursor& c){
				this->erase_backward(c, 1);
			});
			break;
		case morda::key::deletion:
			this->for_each_cursor([this](cursor& c){
				this->erase_forward(c, 1);
			});
			break;
		case morda::key::escape:
			// do nothing
			break;
		case morda::key::a:
			// if(this->ctrlPressed){
			// 	this->selectionStartIndex = 0;
			// 	this->set_cursor_index(this->get_text().size(), true);
			// 	break;
			// }
			// fall through
		default:
			if(!unicode.empty()){
				this->for_each_cursor([this, &unicode](cursor& c){
					this->insert(c, unicode);
				});
			}
			break;
	}	
}

void code_edit::notify_text_change(){
	this->on_text_change();
	this->lines_provider->notify_data_set_change();
}

void code_edit::set_line_spans(decltype(line::spans)&& spans, size_t line_index){
	if(line_index >= this->lines.size()){
		throw std::out_of_range("code_edit::set_line_spans(): given line index is out of range");
	}
	this->lines[line_index].spans = std::move(spans);
}
