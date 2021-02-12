#include "code_edit.hpp"

#include <algorithm>

#include <utki/string.hpp>
#include <utki/linq.hpp>

#include <morda/widgets/label/text.hpp>
#include <morda/widgets/base/fraction_widget.hpp>

namespace{
uint16_t cursor_blink_period_ms = 500;
morda::real cursor_thickness_dp = 2.0f;
}

code_edit::code_edit(std::shared_ptr<morda::context> c, const puu::forest& desc) :
		widget(std::move(c), desc),
		character_input_widget(this->context),
		text_widget(this->context, desc),
		column(this->context, puu::forest()),
		lines_provider(std::make_shared<provider>(*this))
{
	this->set_font(this->context->loader.load<morda::res::font>("fnt_monospace"));
	this->on_font_change();

	this->push_back_inflate(puu::read(R"qwertyuiop(
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

	this->scroll_area = utki::make_shared_from(this->get_widget_as<morda::scroll_area>("scroll_area"));

	this->list = utki::make_shared_from(this->get_widget_as<morda::list_widget>("lines"));
	this->list->set_provider(this->lines_provider);

	this->get_widget_as<morda::fraction_widget>("vertical_scroll").fraction_change_handler =
			[lw = utki::make_weak(this->list)](morda::fraction_widget& fw){
				if(auto w = lw.lock()){
					w->set_scroll_factor(fw.fraction());
				}
			};
	
	this->get_widget_as<morda::fraction_widget>("horizontal_scroll").fraction_change_handler =
			[lw = utki::make_weak(this->scroll_area)](morda::fraction_widget& fw){
				if(auto w = lw.lock()){
					w->set_scroll_factor(fw.fraction());
				}
			};
}

void code_edit::set_text(std::u32string&& text){
	this->lines = utki::linq(utki::split(text, U'\n')).select([this](auto&& s){
			unsigned front_size = s.size() / 2;
			decltype(line::spans) spans = {{
				line_span{length: front_size, attrs: this->text_style},
				line_span{
						length: size_t(s.size()) - front_size,
						attrs: std::make_shared<attributes>(attributes{
								style: morda::res::font::style::italic,
								color: 0xff0000ff
							})
					}
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
	// find cursors
	auto cursors = this->owner.find_cursors(this->line_num);

	const auto& l = this->owner.lines[this->line_num];
	const auto& str = l.str;

	// render text
	size_t cur_char_pos = 0;
	size_t cur_char_index = 0;
	for(const auto& s : l.spans){
		const auto& font = this->owner.get_font().get(s.attrs->style);

		morda::matrix4 matr(matrix);
		matr.translate(
				cur_char_pos * this->owner.font_info.glyph_dims.x(),
				this->owner.font_info.baseline
			);
		auto res = font.render(
				matr,
				morda::color_to_vec4f(s.attrs->color),
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
			morda::matrix4 matr(matrix);

			auto pos = morda::real(c->get_pos_glyphs().x()) * this->owner.font_info.glyph_dims.x();
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

std::vector<const code_edit::cursor*> code_edit::find_cursors(size_t line_num){
	std::vector<const cursor*> ret;

	for(auto& c : this->cursors){
		if(c.get_line_num() == line_num){
			ret.push_back(&c);
		}
	}

	return ret;
}

void code_edit::insert(cursor& c, const std::u32string& str){
	auto strs = utki::split(str, U'\n');
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

bool code_edit::on_mouse_button(const morda::mouse_button_event& event){
	if(this->base_container::on_mouse_button(event)){
		return true;
	}

	if(event.button != morda::mouse_button::left){
		return false;
	}
	
	if(event.is_down){
		this->cursors.clear();

		auto corrected_pos = event.pos +
				morda::vector2{
					this->scroll_area->get_scroll_pos().x(),
					this->list->get_pos_offset()
				};

		// LOG("corrected_pos = " << corrected_pos << std::endl)

		using std::floor;
		auto char_pos = floor(corrected_pos.comp_div(this->font_info.glyph_dims)).to<size_t>();
		char_pos.y() += this->list->get_pos_index();

		this->cursors.push_back(cursor(*this, char_pos));

		this->focus();
		this->start_cursor_blinking();
	}
	
	return true;
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

r4::vector2<size_t> code_edit::cursor::get_pos_chars()const noexcept{
	size_t line_num = this->get_line_num();

	return {
		glyph_pos_to_char_pos(
				this->pos.x(),
				this->owner.lines[line_num].str,
				this->owner.settings.tab_size
			),
		this->pos.y()
	};
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

	this->owner.scroll_to(this->pos);

	this->owner.start_cursor_blinking();
}

r4::vector2<size_t> code_edit::cursor::get_pos_glyphs()const noexcept{
	ASSERT(!this->owner.lines.empty())
	auto p = this->get_pos_chars();

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
			spans: {line_span{ attrs: this->spans.back().attrs }}
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
			ret.spans.push_back(line_span(*i));
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
	}

	ASSERT(!this->list->children().empty())
	size_t bottom = top + this->list->children().size();
	if(bottom <= pos_glyphs.y()){
		morda::real bottom_offset = this->list->rect().d.y() - this->list->children().back()->rect().y2();
		this->list->scroll_by(
				morda::real(pos_glyphs.y() - bottom) * this->font_info.glyph_dims.y()
				+ bottom_offset
			);
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

	this->owner.scroll_to(this->get_pos_glyphs());

	this->owner.start_cursor_blinking();
}

size_t code_edit::num_lines_on_page()const noexcept{
	using std::floor;
	return size_t(floor(this->list->rect().d.y() / this->font_info.glyph_dims.y()));
}

void code_edit::on_character_input(const std::u32string& unicode, morda::key key){
	switch(key){
		case morda::key::enter:
			this->for_each_cursor([this](cursor& c){
				this->put_new_line(c);
			});
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
