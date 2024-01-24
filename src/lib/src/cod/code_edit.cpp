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

#include <ruis/widgets/base/fraction_band_widget.hpp>
#include <ruis/widgets/label/text.hpp>
#include <utki/linq.hpp>
#include <utki/string.hpp>

using namespace cod;

namespace {
constexpr uint16_t cursor_blink_period_ms = 500;
constexpr ruis::real cursor_thickness_pp = 2.0f;
} // namespace

// TODO: refactor to fix this lint issue
// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
code_edit::code_edit(const utki::shared_ref<ruis::context>& c, const treeml::forest& desc) :
	widget(std::move(c), desc),
	character_input_widget(this->context),
	text_widget(this->context, desc),
	container( //
		this->context,
		treeml::read(R"qwertyuiop(
			layout{column}
			@row{
				lp{dx{fill} dy{0} weight{1}}

				@scroll_area{
					id{scroll_area}
					lp{dx{0} dy{fill} weight{1}}
					clip{true}
					@list{
						id{lines}
						lp{dx{min} dy{fill}}
					}
				}
				@vertical_scroll_bar{
					id{vertical_scroll}

					lp{
						dx{min} dy{max}
					}
				}
			}
			@horizontal_scroll_bar{
				id{horizontal_scroll}

				lp{
					dx{fill} dy{min}
				}
			}
		)qwertyuiop")
	),
	list(utki::make_shared_from(this->get_widget_as<ruis::list_widget>("lines"))),
	scroll_area(utki::make_shared_from(this->get_widget_as<ruis::scroll_area>("scroll_area"))),
	lines_provider(std::make_shared<provider>(*this))
{
	this->set_font_face(this->context.get().loader.load<ruis::res::font>("fnt_monospace"));
	this->code_edit::on_font_change();

	this->list.get().set_provider(this->lines_provider);

	auto& vs = this->get_widget_as<ruis::fraction_band_widget>("vertical_scroll");

	vs.fraction_change_handler = [lw = utki::make_weak(this->list)](ruis::fraction_widget& fw) {
		if (auto w = lw.lock()) {
			w->set_scroll_factor(fw.fraction());
		}
	};

	this->list.get().scroll_change_handler = [sw = utki::make_weak_from(vs)](ruis::list_widget& lw) {
		if (auto s = sw.lock()) {
			s->set_fraction(lw.get_scroll_factor(), false);
			s->set_band_fraction(lw.get_scroll_band());
		}
	};

	auto& hs = this->get_widget_as<ruis::fraction_band_widget>("horizontal_scroll");

	hs.fraction_change_handler = [saw = utki::make_weak(this->scroll_area)](ruis::fraction_widget& fw) {
		if (auto sa = saw.lock()) {
			sa->set_scroll_factor(fw.fraction());
		}
	};

	this->scroll_area.get().scroll_change_handler = [sw = utki::make_weak_from(hs)](ruis::scroll_area& sa) {
		if (auto s = sw.lock()) {
			s->set_fraction(sa.get_scroll_factor().x(), false);
			s->set_band_fraction(sa.get_visible_area_fraction().x());
		}
	};
}

void code_edit::set_text(std::u32string text)
{
	this->lines = utki::linq(utki::split(text, U'\n'))
					  .select([this](auto s) {
						  auto size = s.size();
						  return line{
							  .str = std::move(s),
							  .spans = {synhi::line_span{.length = size, .style = this->text_style}}
						  };
					  })
					  .get();
	this->on_text_change();
}

std::u32string code_edit::get_text() const
{
	std::u32string ret;

	for (auto& l : this->lines) {
		ret.append(l.str);
	}

	return ret;
}

utki::shared_ref<ruis::widget> code_edit::provider::get_widget(size_t index)
{
	return utki::make_shared<code_edit::line_widget>(this->owner.context, this->owner, index);
}

namespace {
size_t char_pos_to_glyph_pos(size_t p, const std::u32string& str, size_t tab_size)
{
	size_t x = 0;
	for (size_t i = 0; i < str.size() && i != p; ++i) {
		ASSERT(i < str.size())
		if (str[i] == U'\t') {
			x += tab_size - x % tab_size;
		} else {
			++x;
		}
	}
	return x;
}
} // namespace

namespace {
size_t string_length_glyphs(const std::u32string& str, size_t tab_size)
{
	return char_pos_to_glyph_pos(str.size(), str, tab_size);
}
} // namespace

void code_edit::line_widget::render(const ruis::matrix4& matrix) const
{
	// find cursors
	auto cursors = this->owner.find_cursors(this->line_num);

	// render selection
	for (auto c : cursors) {
		auto& sel = std::get<cursor::selection>(c).segment;

		if (sel.p1.y() > this->line_num || this->line_num > sel.p2.y()) {
			continue;
		}

		size_t start = [&sel, this]() -> size_t {
			if (sel.p1.y() == this->line_num) {
				return sel.p1.x();
			} else {
				return 0;
			}
		}();

		size_t length = [&sel, &start, this]() {
			if (sel.p2.y() == this->line_num) {
				return sel.p2.x() - start;
			} else {
				return string_length_glyphs(this->owner.lines[this->line_num].str, this->owner.settings.tab_size) -
					start;
			}
		}();

		ruis::matrix4 matr(matrix);

		auto pos = ruis::real(start) * this->owner.font_info.glyph_dims.x();
		matr.translate(pos, 0);
		matr.scale(ruis::vector2(ruis::real(length), 1).comp_mul(this->owner.font_info.glyph_dims));

		constexpr auto selection_color = 0xff804000;

		auto& r = this->context.get().renderer.get();
		r.shader->color_pos->render(matr, r.pos_quad_01_vao.get(), selection_color);
	}

	const auto& l = this->owner.lines[this->line_num];
	const auto str = std::u32string_view(l.str);

	// render text
	size_t cur_char_pos = 0;
	size_t cur_char_index = 0;
	for (const auto& s : l.spans) {
		const auto& font = this->owner.get_font(s.style->style);

		ruis::matrix4 matr(matrix);
		matr.translate(
			ruis::real(cur_char_pos) * this->owner.font_info.glyph_dims.x(),
			this->owner.font_info.baseline
		);
		auto res = font.render(
			matr,
			ruis::color_to_vec4f(s.style->color),
			str.substr(cur_char_index, s.length),
			this->owner.settings.tab_size,
			cur_char_pos
		);
		cur_char_index += s.length;
		cur_char_pos += res.length;
	}

	// render cursors
	if (this->owner.cursor_blink_visible) {
		for (auto c : cursors) {
			auto cp = std::get<cursor::selection>(c).get_cursor_pos_glyphs();

			if (cp.y() != this->line_num) {
				continue;
			}

			ruis::matrix4 matr(matrix);

			auto pos = ruis::real(cp.x()) * this->owner.font_info.glyph_dims.x();
			matr.translate(pos, 0);
			matr.scale(ruis::vector2(
				cursor_thickness_pp * this->context.get().units.dots_per_pp,
				this->owner.font_info.glyph_dims.y()
			));

			constexpr auto cursor_color = 0xffffffff;

			auto& r = this->context.get().renderer.get();
			r.shader->color_pos->render(matr, r.pos_quad_01_vao.get(), cursor_color);
		}
	}
}

ruis::vector2 code_edit::line_widget::measure(const ruis::vector2& quotum) const noexcept
{
	ruis::vector2 ret = this->owner.font_info.glyph_dims;
	ret.x() *= ruis::real(
		this->owner.lines[this->line_num].str.size() + 1
	); // for empty strings the widget will still have size of one glyph

	for (unsigned i = 0; i != ret.size(); ++i) {
		if (quotum[i] >= 0) {
			ret[i] = quotum[i];
		}
	}

	return ret;
}

void code_edit::update(uint32_t dt)
{
	this->cursor_blink_visible = !this->cursor_blink_visible;
}

void code_edit::on_focus_change()
{
	if (this->is_focused()) {
		this->start_cursor_blinking();
	} else {
		this->context.get().updater.get().stop(*this);
	}
}

void code_edit::start_cursor_blinking()
{
	this->context.get().updater.get().stop(*this);
	this->cursor_blink_visible = true;
	this->context.get().updater.get().start(
		utki::make_weak_from(*static_cast<updateable*>(this)),
		cursor_blink_period_ms
	);
}

void code_edit::for_each_cursor(const std::function<void(cursor&)>& func)
{
	ASSERT(func)
	for (auto& c : this->cursors) {
		func(c);
		// TODO: check if cursors do not intersect
	}
	if (this->text_changed) {
		this->text_changed = false;
		this->notify_text_change();
	}
}

std::vector<std::tuple<const code_edit::cursor*, code_edit::cursor::selection>> code_edit::find_cursors(size_t line_num)
{
	std::vector<std::tuple<const cursor*, cursor::selection>> ret;

	for (auto& c : this->cursors) {
		auto s = c.get_selection_glyphs();
		if (s.segment.p1.y() <= line_num && line_num <= s.segment.p2.y()) {
			ret.emplace_back(&c, s);
		}
	}

	return ret;
}

void code_edit::insert(cursor& c, std::u32string_view str)
{
	auto strs = utki::split(str, U'\n');
	ASSERT(!strs.empty())

	auto cp = c.get_pos_chars();
	c.set_pos_chars(cp);

	if (strs.size() == 1) {
		auto& l = this->lines[cp.y()];
		l.str.insert(cp.x(), strs.front());
		l.extend_span(cp.x(), strs.front().size());
		c.move_right_by(strs.front().size());
	} else {
		// TODO: multiline insert (from clipboard)
	}

	// TODO: correct cursors

	this->text_changed = true;
}

void code_edit::erase_forward(cursor& c, size_t num)
{
	auto cp = c.get_pos_chars();

	auto& l = this->lines[cp.y()];

	ASSERT(cp.x() <= l.size())
	if (cp.x() == l.size()) {
		ASSERT(cp.y() < this->lines.size())
		if (cp.y() + 1 == this->lines.size()) {
			return;
		}
		auto i = utki::next(this->lines.begin(), cp.y() + 1);
		auto ll = std::move(*i);
		this->lines.erase(i);
		l.append(std::move(ll));
	} else {
		size_t s = [&cp, &l, &num]() {
			ASSERT(cp.x() <= l.size());
			size_t to_end = l.size() - cp.x();
			if (num > to_end) {
				return to_end;
			} else {
				return num;
			}
		}();
		l.erase(cp.x(), s);
	}

	// TODO: correct cursors

	this->text_changed = true;
}

void code_edit::erase_backward(cursor& c, size_t num)
{
	auto cp = c.get_pos_chars();

	if (cp.x() == 0) {
		if (cp.y() == 0) {
			return;
		}
		auto i = utki::next(this->lines.begin(), cp.y());
		auto ll = std::move(*i);
		this->lines.erase(i);
		--cp.y();
		auto& l = this->lines[cp.y()];
		cp.x() = l.size();
		l.append(std::move(ll));
		c.set_pos_chars(cp);
	} else {
		auto& l = this->lines[cp.y()];

		struct pos_and_size {
			size_t pos;
			size_t size;
		};

		auto ps = [&cp, &num]() -> pos_and_size {
			if (cp.x() >= num) {
				return {cp.x() - num, num};
			} else {
				return {0, cp.x()};
			}
		}();

		cp.x() -= ps.size;
		c.set_pos_chars(cp);

		l.erase(ps.pos, ps.size);
	}

	// TODO: correct cursors

	this->text_changed = true;
}

void code_edit::put_new_line(cursor& c)
{
	auto cp = c.get_pos_chars();

	ASSERT(cp.y() < this->lines.size())

	auto i = utki::next(this->lines.begin(), cp.y());

	auto nl = i->cut_tail(cp.x());
	this->lines.insert(std::next(i), std::move(nl));

	++cp.y();
	cp.x() = 0;
	c.set_pos_chars(cp);

	// TODO: correct cursors

	this->text_changed = true;
}

void code_edit::render(const ruis::matrix4& matrix) const
{
	this->base_container::render(matrix);
}

r4::vector2<size_t> code_edit::mouse_pos_to_glyph_pos(const ruis::vector2& mouse_pos) const noexcept
{
	auto corrected_mouse_pos =
		mouse_pos + ruis::vector2{this->scroll_area.get().get_scroll_pos().x(), this->list.get().get_pos_offset()};

	using std::max;
	corrected_mouse_pos = max(corrected_mouse_pos, 0); // clamp to positive values

	// LOG("corrected_pos = " << corrected_pos << std::endl)

	using std::round;
	using std::floor;
	auto glyph_pos_real = corrected_mouse_pos.comp_div(this->font_info.glyph_dims);
	glyph_pos_real.x() = round(glyph_pos_real.x());
	glyph_pos_real.y() = floor(glyph_pos_real.y());
	auto glyph_pos = glyph_pos_real.to<size_t>();
	glyph_pos.y() += this->list.get().get_pos_index();

	return glyph_pos;
}

bool code_edit::on_mouse_button(const ruis::mouse_button_event& event)
{
	if (this->base_container::on_mouse_button(event)) {
		return true;
	}

	ruis::real scroll_direction = 1;
	switch (event.button) {
		case ruis::mouse_button::left:
			if (event.is_down) {
				this->cursors.clear();

				this->cursors.emplace_back(*this, this->mouse_pos_to_glyph_pos(event.pos));
				this->mouse_selection = true;

				this->focus();
				this->start_cursor_blinking();
			} else {
				this->mouse_selection = false;
			}
			break;
		case ruis::mouse_button::wheel_up:
			scroll_direction = -1;
			[[fallthrough]];
		case ruis::mouse_button::wheel_down:
			if (event.is_down) {
				this->list.get().scroll_by(scroll_direction * this->font_info.glyph_dims.y() * 3);
			}
			break;
		case ruis::mouse_button::wheel_left:
			scroll_direction = -1;
			[[fallthrough]];
		case ruis::mouse_button::wheel_right:
			if (event.is_down) {
				this->scroll_area.get().set_scroll_pos(
					this->scroll_area.get().get_scroll_pos() +
					ruis::vector2{scroll_direction * this->font_info.glyph_dims.x() * 3, 0}
				);
			}
			break;
		default:
			return false;
	}

	return true;
}

bool code_edit::on_mouse_move(const ruis::mouse_move_event& event)
{
	if (this->base_container::on_mouse_move(event)) {
		return true;
	}

	if (this->mouse_selection) {
		ASSERT(!this->cursors.empty())
		this->cursors.front().set_pos_glyphs(this->mouse_pos_to_glyph_pos(event.pos));
		return true;
	}
	return false;
}

void code_edit::cursor::update_selection()
{
	bool selection = this->owner.mouse_selection || this->owner.modifiers.get(code_edit::modifier::selection);
	if (selection) {
		return;
	}
	this->sel_pos_glyphs = this->get_pos_glyphs();
}

namespace {
size_t glyph_pos_to_char_pos(size_t p, const std::u32string& str, size_t tab_size)
{
	size_t x = 0;
	size_t i = 0;
	for (; i != str.size(); ++i) {
		auto d = [&str, &i, &tab_size, &x]() -> size_t {
			if (str[i] == U'\t') {
				return tab_size - x % tab_size;
			} else {
				return 1;
			}
		}();

		size_t px = p - x;
		if (px <= d) {
			if (px <= d / 2)
				return i;
			else
				return i + 1;
		}

		x += d;
	}
	return i;
}
} // namespace

size_t code_edit::cursor::get_line_num() const noexcept
{
	ASSERT(!this->owner.lines.empty())
	if (this->pos.y() >= this->owner.lines.size()) {
		return this->owner.lines.size() - 1;
	}
	return this->pos.y();
}

code_edit::cursor::selection code_edit::cursor::get_selection_glyphs() const noexcept
{
	auto cp = this->get_pos_glyphs();

	if (cp.y() < this->sel_pos_glyphs.y() || (cp.y() == this->sel_pos_glyphs.y() && cp.x() < this->sel_pos_glyphs.x()))
	{
		return {
			.segment{cp, this->sel_pos_glyphs},
			.is_left_to_right = false
		};
	} else {
		return {
			.segment{this->sel_pos_glyphs, cp},
			.is_left_to_right = true
		};
	}
}

r4::vector2<size_t> code_edit::cursor::get_pos_chars() const noexcept
{
	size_t line_num = this->get_line_num();

	ASSERT(line_num < this->owner.lines.size())

	using std::min;

	ASSERT(!this->owner.lines.empty())
	return {
		glyph_pos_to_char_pos(this->pos.x(), this->owner.lines[line_num].str, this->owner.settings.tab_size),
		min(this->pos.y(), this->owner.lines.size() - 1)
	};
}

void code_edit::cursor::set_pos_chars(r4::vector2<size_t> p) noexcept
{
	ASSERT(!this->owner.lines.empty())
	ASSERT(p.y() < this->owner.lines.size())

	// LOG("p = " << p << std::endl)

	this->pos = {char_pos_to_glyph_pos(p.x(), this->owner.lines[p.y()].str, this->owner.settings.tab_size), p.y()};

	this->update_selection();

	this->owner.scroll_to(this->pos);

	this->owner.start_cursor_blinking();
}

void code_edit::cursor::set_pos_glyphs(r4::vector2<size_t> p) noexcept
{
	this->pos = p;

	this->update_selection();
}

r4::vector2<size_t> code_edit::cursor::get_pos_glyphs() const noexcept
{
	ASSERT(!this->owner.lines.empty())
	auto p = this->get_pos_chars();

	ASSERT(p.y() < this->owner.lines.size(), [&](auto& o) {
		o << "this->owner.lines.size() = " << this->owner.lines.size() << ", p.y() = " << p.y();
	})

	return {char_pos_to_glyph_pos(p.x(), this->owner.lines[p.y()].str, this->owner.settings.tab_size), p.y()};
}

bool code_edit::on_key(const ruis::key_event& e)
{
	switch (e.combo.key) {
		case ruis::key::left_control:
		case ruis::key::right_control:
			this->modifiers.set(modifier::word_navigation, e.is_down);
			break;
		case ruis::key::left_shift:
		case ruis::key::right_shift:
			this->modifiers.set(modifier::selection, e.is_down);
			break;
		default:
			break;
	}
	return false;
}

void code_edit::line::extend_span(size_t at_char_index, size_t by_length)
{
	size_t cur_span_end = 0;
	for (auto& s : this->spans) {
		cur_span_end += s.length;
		if (at_char_index < cur_span_end) {
			s.length += by_length;
			return;
		}
	}
	// extend last span if adding chars to the end of the string
	ASSERT(!this->spans.empty())
	this->spans.back().length += by_length;
}

void code_edit::line::erase_spans(size_t at_char_index, size_t by_length)
{
	size_t span_end = 0;
	for (auto i = this->spans.begin(); i != this->spans.end() && by_length != 0;) {
		span_end += i->length;
		if (at_char_index < span_end) {
			size_t to_end = span_end - at_char_index;
			ASSERT(to_end <= i->length)
			if (by_length < to_end) {
				ASSERT(i->length > by_length)
				i->length -= by_length;
				return;
			} else {
				by_length -= to_end;
				if (to_end == i->length) {
					if (this->spans.size() == 1) { // do not remove the last span
						this->spans.back().length = 0;
						break;
					}
					// remove span
					i = this->spans.erase(i);
					continue;
				} else {
					ASSERT(to_end < i->length)
					i->length -= to_end;
				}
			}
		}
		++i;
	}
}

void code_edit::line::append(line l)
{
	this->str.append(std::move(l.str));
	this->spans
		.insert(this->spans.end(), std::make_move_iterator(l.spans.begin()), std::make_move_iterator(l.spans.end()));
}

code_edit::line code_edit::line::cut_tail(size_t pos)
{
	if (pos >= this->size()) {
		return line{.spans = {synhi::line_span{.style = this->spans.back().style}}};
	}

	line ret;
	ret.str = this->str.substr(pos);
	this->str = this->str.substr(0, pos);

	size_t cur_span_end = 0;
	for (auto i = this->spans.begin(); i != this->spans.end(); ++i) {
		cur_span_end += i->length;
		if (pos < cur_span_end) {
			size_t to_end = cur_span_end - pos;
			ret.spans.emplace_back(*i);
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

void code_edit::scroll_to(r4::vector2<size_t> pos_glyphs)
{
	// vertical
	size_t top = this->list.get().get_pos_index();
	if (top >= pos_glyphs.y()) {
		this->list.get().scroll_by(
			-ruis::real(top - pos_glyphs.y()) * this->font_info.glyph_dims.y() - this->list.get().get_pos_offset()
		);
	} else {
		ASSERT(!this->list.get().children().empty())
		size_t bottom = top + this->list.get().children().size() - 1;
		if (bottom <= pos_glyphs.y()) {
			ruis::real bottom_offset =
				this->list.get().children().back().get().rect().y2() - this->list.get().rect().d.y();
			this->list.get().scroll_by(
				ruis::real(pos_glyphs.y() - bottom) * this->font_info.glyph_dims.y() + bottom_offset
			);
		}
	}

	// horizontal
	auto pos_x = ruis::real(pos_glyphs.x()) * this->font_info.glyph_dims.x();

	auto left = this->scroll_area.get().get_scroll_pos().x();
	if (left > pos_x) {
		this->scroll_area.get().set_scroll_pos({pos_x, 0});
	} else {
		pos_x -= this->scroll_area.get().rect().d.x() - this->font_info.glyph_dims.x();
		if (left < pos_x) {
			this->scroll_area.get().set_scroll_pos({pos_x, 0});
		}
	}
}

void code_edit::cursor::move_right_by(size_t dx) noexcept
{
	auto p = this->get_pos_chars();
	p.x() += dx;
	auto line_size = this->owner.lines[p.y()].str.size();

	ASSERT(!this->owner.lines.empty())
	for (; p.x() > line_size;) {
		if (p.y() < this->owner.lines.size() - 1) {
			p.x() -= line_size + 1;
			++p.y();
			line_size = this->owner.lines[p.y()].str.size();
		} else {
			p.x() = line_size;
			break;
		}
	}
	this->set_pos_chars(p);
}

void code_edit::cursor::move_left_by(size_t dx) noexcept
{
	auto p = this->get_pos_chars();
	for (; dx > p.x();) {
		if (p.y() == 0) {
			p.x() = 0;
			this->set_pos_chars(p);
			return;
		} else {
			dx -= p.x() + 1;
			--p.y();
			p.x() = this->owner.lines[p.y()].str.size();
		}
	}
	// LOG("p = " << p << " dx = " << dx << std::endl)
	p.x() -= dx;

	this->set_pos_chars(p);
}

void code_edit::cursor::move_up_by(size_t dy) noexcept
{
	if (this->pos.y() < dy) {
		this->pos.y() = 0;
	} else {
		this->pos.y() -= dy;
	}

	this->update_selection();

	this->owner.scroll_to(this->get_pos_glyphs());

	this->owner.start_cursor_blinking();
}

void code_edit::cursor::move_down_by(size_t dy) noexcept
{
	size_t max_y = this->owner.lines.size() - 1;
	if (max_y - this->pos.y() < dy) {
		this->pos.y() = max_y;
	} else {
		this->pos.y() += dy;
	}

	this->update_selection();

	this->owner.scroll_to(this->get_pos_glyphs());

	this->owner.start_cursor_blinking();
}

size_t code_edit::num_lines_on_page() const noexcept
{
	using std::floor;
	return size_t(floor(this->list.get().rect().d.y() / this->font_info.glyph_dims.y()));
}

size_t code_edit::calc_word_length_forward(const cursor& c) const noexcept
{
	size_t ret = 0;

	auto cp = c.get_pos_chars();

	if (cp.x() == this->lines[cp.y()].size()) {
		++cp.y();
		if (cp.y() == this->lines.size()) {
			return 0;
		}
		++ret; // for new line
		cp.x() = 0;
	}

	auto& l = this->lines[cp.y()].str;

	bool non_ws_met = false;
	for (auto i = utki::next(l.begin(), cp.x()); i != l.end(); ++i, ++ret) {
		if (non_ws_met) {
			// TODO: converting char32_t to int might not be the right thing to do here.
			// Consider using ICU library for checking for whitespace.
			if (std::isspace(int(*i))) {
				break;
			}
		} else {
			// TODO: converting char32_t to int might not be the right thing to do here.
			// Consider using ICU library for checking for whitespace.
			if (!std::isspace(int(*i))) {
				non_ws_met = true;
			}
		}
	}

	return ret;
}

size_t code_edit::calc_word_length_backward(const cursor& c) const noexcept
{
	size_t ret = 0;

	auto cp = c.get_pos_chars();

	if (cp.x() == 0) {
		if (cp.y() == 0) {
			return 0;
		}
		--cp.y();
		cp.x() = this->lines[cp.y()].size();
		++ret; // for new line
	}

	auto& l = this->lines[cp.y()].str;

	bool non_ws_met = false;

	ASSERT(cp.x() <= l.size())
	for (auto i = utki::next(l.rbegin(), l.size() - cp.x()); i != l.rend(); ++i, ++ret) {
		if (non_ws_met) {
			// TODO: converting char32_t to int might not be the right thing to do here.
			// Consider using ICU library for checking for whitespace.
			if (std::isspace(int(*i))) {
				break;
			}
		} else {
			// TODO: converting char32_t to int might not be the right thing to do here.
			// Consider using ICU library for checking for whitespace.
			if (!std::isspace(int(*i))) {
				non_ws_met = true;
			}
		}
	}

	return ret;
}

void code_edit::on_character_input(const ruis::character_input_event& e)
{
	switch (e.combo.key) {
		case ruis::key::enter:
			this->for_each_cursor([this](cursor& c) {
				this->put_new_line(c);
			});
			break;
		case ruis::key::arrow_right:
			this->for_each_cursor([this](cursor& c) {
				size_t d = 1;
				if (this->modifiers.get(code_edit::modifier::word_navigation)) {
					d = this->calc_word_length_forward(c);
				}
				c.move_right_by(d);
			});
			break;
		case ruis::key::arrow_left:
			this->for_each_cursor([this](cursor& c) {
				size_t d = 1;
				if (this->modifiers.get(code_edit::modifier::word_navigation)) {
					d = this->calc_word_length_backward(c);
				}
				c.move_left_by(d);
			});
			break;
		case ruis::key::arrow_up:
			this->for_each_cursor([](cursor& c) {
				c.move_up_by(1);
			});
			break;
		case ruis::key::arrow_down:
			this->for_each_cursor([](cursor& c) {
				c.move_down_by(1);
			});
			break;
		case ruis::key::page_up:
			this->for_each_cursor([n = this->num_lines_on_page()](cursor& c) {
				c.move_up_by(n);
			});
			break;
		case ruis::key::page_down:
			this->for_each_cursor([n = this->num_lines_on_page()](cursor& c) {
				c.move_down_by(n);
			});
			break;
		case ruis::key::end:
			this->for_each_cursor([this](cursor& c) {
				auto p = c.get_pos_chars();
				p.x() = this->lines[p.y()].str.size();
				c.set_pos_chars(p);
			});
			break;
		case ruis::key::home:
			this->for_each_cursor([this](cursor& c) {
				auto p = c.get_pos_chars();
				auto& l = this->lines[p.y()].str;
				auto non_ws_pos = l.find_first_not_of(U" \t");
				if (non_ws_pos == std::remove_reference_t<decltype(l)>::npos || p.x() == non_ws_pos) {
					p.x() = 0;
				} else {
					p.x() = non_ws_pos;
				}
				c.set_pos_chars(p);
			});
			break;
		case ruis::key::backspace:
			this->for_each_cursor([this](cursor& c) {
				this->erase_backward(c, 1);
			});
			break;
		case ruis::key::deletion:
			this->for_each_cursor([this](cursor& c) {
				this->erase_forward(c, 1);
			});
			break;
		case ruis::key::escape:
			// do nothing
			break;
		case ruis::key::a:
			// if(this->ctrlPressed){
			// 	this->selectionStartIndex = 0;
			// 	this->set_cursor_index(this->get_text().size(), true);
			// 	break;
			// }
			// fall through
		default:
			if (!e.string.empty()) {
				this->for_each_cursor([this, &e](cursor& c) {
					this->insert(c, e.string);
				});
			}
			break;
	}
}

void code_edit::notify_text_change()
{
	this->on_text_change();
	this->lines_provider->notify_data_set_change();
}

void code_edit::set_line_spans(decltype(line::spans)&& spans, size_t line_index)
{
	if (line_index >= this->lines.size()) {
		throw std::out_of_range("code_edit::set_line_spans(): given line index is out of range");
	}
	this->lines[line_index].spans = std::move(spans);
}

void code_edit::on_font_change()
{
	const auto& font = this->get_font();

	using std::round;

	this->font_info.glyph_dims.set(font.get_advance(' '), font.get_height());
	this->font_info.baseline = round((font.get_height() + font.get_ascender() - font.get_descender()) / 2);
}
