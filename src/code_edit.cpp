#include "code_edit.hpp"

#include <algorithm>

#include <utki/string.hpp>
#include <utki/linq.hpp>

#include <morda/widgets/label/text.hpp>

namespace{
uint16_t cursor_blink_period_ms = 500;
morda::real cursor_thickness_dp = 2.0f;
}

code_edit::code_edit(std::shared_ptr<morda::context> c, const puu::forest& desc) :
		widget(std::move(c), desc),
		character_input_widget(this->context),
		text_widget(this->context, desc),
		pile(this->context, puu::forest()),
		lines_provider(std::make_shared<provider>(*this))
{
	this->set_font(this->context->loader.load<morda::res::font>("fnt_monospace"));
	this->on_font_change();

	this->push_back_inflate(puu::read(R"qwertyuiop(
			@list{
				id{lines}
				layout{dx{fill} dy{fill}}
			}
		)qwertyuiop"));

	this->get_widget_as<morda::list_widget>("lines").set_provider(this->lines_provider);
}

void code_edit::set_text(std::u32string&& text){
	this->lines = utki::linq(utki::split(text, U'\n')).select([this](auto&& s){
			unsigned front_size = s.size() / 2;
			decltype(line::spans) spans = {{
				line_span{length: front_size, attrs: this->text_style},
				line_span{length: size_t(s.size()) - front_size, attrs: std::make_shared<attributes>(attributes{style: morda::res::font::style::italic, color: 0xff0000ff})}
			}};
			return line{
					str: std::move(s),
					spans: std::move(spans)
				};
		}).get();
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

void code_edit::line_widget::render(const morda::matrix4& matrix)const{	
	using std::round;

	const auto& str = this->owner.lines[this->line_num].str;

	unsigned cur_char_pos = 0;
	for(const auto& s : this->owner.lines[this->line_num].spans){
		const auto& font = this->owner.get_font().get(s.attrs->style);

		morda::matrix4 matr(matrix);
		matr.translate(
				cur_char_pos * this->owner.font_info.glyph_dims.x(),
				this->owner.font_info.baseline
			);
		font.render(
				matr,
				morda::color_to_vec4f(s.attrs->color),
				std::u32string_view(str.c_str() + cur_char_pos, s.length)
			);
		cur_char_pos += s.length;
	}
}

morda::vector2 code_edit::line_widget::measure(const morda::vector2& quotum)const noexcept{
	morda::vector2 ret = this->owner.font_info.glyph_dims;
	ret.x() *= this->owner.lines[this->line_num].str.size();

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
		this->cursors.clear();
	}
}

void code_edit::start_cursor_blinking(){
	this->context->updater->stop(*this);
	this->cursor_blink_visible = true;
	this->context->updater->start(
			utki::make_shared_from(*static_cast<updateable*>(this)),
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

void code_edit::insert(cursor& c, const std::u32string& str){
	auto strs = utki::split(str, U'\n');
	ASSERT(!strs.empty())

	auto cp = c.get_effective_pos();

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
	// auto cp = c.get_effective_pos();

	// TODO:
}

void code_edit::erase_backward(cursor& c, size_t num){
	// TODO:
}

void code_edit::render(const morda::matrix4& matrix)const{
	this->pile::render(matrix);

	this->render_cursors(matrix);
}

void code_edit::render_cursors(const morda::matrix4& matrix)const{
	if(!this->is_focused()) return;
	if(!this->cursor_blink_visible) return;

	for(auto& c : this->cursors){
		morda::matrix4 matr(matrix);

		morda::vector2 pos = c.get_effective_pos().to<morda::real>().comp_mul(this->font_info.glyph_dims);
		matr.translate(pos);
		matr.scale(morda::vector2(cursor_thickness_dp * this->context->units.dots_per_dp, this->font_info.glyph_dims.y()));

		auto& r = *this->context->renderer;
		r.shader->color_pos->render(matr, *r.pos_quad_01_vao, 0xffffffff);
	}
}

bool code_edit::on_mouse_button(const morda::mouse_button_event& event){
	if(event.button != morda::mouse_button::left){
		return false;
	}

	// this->leftMouseButtonDown = e.is_down;
	
	if(event.is_down){
		// this->set_cursor_index(this->posToIndex(e.pos.x()));
		if(!this->is_focused()){
			this->cursors.push_back(cursor(*this, 0));
			this->focus();
		}
	}
	
	return true;
}

r4::vector2<size_t> code_edit::cursor::get_effective_pos()const noexcept{
	ASSERT(!this->owner.lines.empty())
	if(this->pos.y() >= this->owner.lines.size()){
		return 0;
	}
	auto cur_line_size = this->owner.lines[this->pos.y()].str.size();
	if(this->pos.x() > cur_line_size){
		return {cur_line_size, this->pos.y()};
	}
	return this->pos;
}

bool code_edit::on_key(bool is_down, morda::key key){
	switch(key){
		case morda::key::left_control:
		case morda::key::right_control:
			// this->ctrlPressed = isDown;
			break;
		case morda::key::left_shift:
		case morda::key::right_shift:
			// this->shiftPressed = isDown;
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

void code_edit::cursor::move_right_by(size_t dx)noexcept{
	auto p = this->get_effective_pos();
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
	this->set_pos(p);
}

void code_edit::cursor::move_left_by(size_t dx)noexcept{
	auto p = this->get_effective_pos();
	for(; dx > p.x();){
		if(p.y() == 0){
			p.x() = 0;
			this->set_pos(p);
			return;
		}else{
			dx -= p.x() + 1;
			--p.y();
			p.x() = this->owner.lines[p.y()].str.size();
		}
	}
	p.x() -= dx;

	this->set_pos(p);
}

void code_edit::cursor::move_up_by(size_t dy)noexcept{
	auto p = this->pos;
	if(p.y() < dy){
		p.y() = 0;
	}else{
		p.y() -= dy;
	}
	this->set_pos(p);
}

void code_edit::cursor::move_down_by(size_t dy)noexcept{
	auto p = this->pos;
	p.y() += dy;
	using std::min;
	p.y() = min(this->owner.lines.size() - 1, p.y());
	this->set_pos(p);
}

void code_edit::on_character_input(const std::u32string& unicode, morda::key key){
	switch(key){
		case morda::key::enter:

			break;
		case morda::key::right:
			this->for_each_cursor([](cursor& c){
				c.move_right_by(1);
			});
			break;
		case morda::key::left:
			this->for_each_cursor([](cursor& c){
				c.move_left_by(1);
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
		case morda::key::end:
			this->for_each_cursor([this](cursor& c){
				auto p = c.get_effective_pos();
				p.x() = this->lines[p.y()].str.size();
				c.set_pos(p);
			});
			break;
		case morda::key::home:
			this->for_each_cursor([](cursor& c){
				auto p = c.get_effective_pos();
				p.x() = 0;
				c.set_pos(p);
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
