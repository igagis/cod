/*
cod - text editor

Copyright (C) 2021-2025  Ivan Gagis <igagis@gmail.com>

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

#pragma once

#include <vector>

#include <r4/segment2.hpp>
#include <ruis/updateable.hpp>
#include <ruis/widget/base/text_widget.hpp>
#include <ruis/widget/group/list.hpp>
#include <ruis/widget/group/scroll_area.hpp>
#include <ruis/widget/input/character_input_widget.hpp>
#include <utki/flags.hpp>

#include "synhi/highlighter.hpp"

namespace cod {

class code_edit :
	public ruis::character_input_widget,
	public ruis::text_widget,
	private ruis::container,
	private ruis::updateable
{
	using base_container = ruis::container;

	utki::shared_ref<ruis::list> list;
	utki::shared_ref<ruis::scroll_area> scroll_area;

	struct {
		ruis::vector2 glyph_dims;
		ruis::real baseline;
	} font_info;

	void on_font_change() override;

	struct {
		unsigned tab_size = 4;
	} settings;

	enum class modifier {
		selection,
		word_navigation,

		enum_size
	};

	utki::flags<modifier> modifiers = false;

	struct line {
		std::u32string str;
		std::vector<synhi::line_span> spans;

		size_t size() const noexcept
		{
			return this->str.size();
		}

		void extend_span(size_t at_char_index, size_t by_length);
		void erase_spans(size_t at_char_index, size_t by_length);

		void erase(size_t pos, size_t num)
		{
			this->str.erase(pos, num);
			this->erase_spans(pos, num);
		}

		void append(line l);

		line cut_tail(size_t pos);
	};

	std::vector<line> lines;

	class line_widget : public ruis::widget
	{
		code_edit& owner;
		size_t line_num;

	public:
		line_widget(
			utki::shared_ref<ruis::context> context, //
			code_edit& owner,
			size_t line_num
		) :
			widget(std::move(context), {}, {}),
			owner(owner),
			line_num(line_num)
		{}

		void render(const ruis::matrix4& matrix) const override;

		ruis::vector2 measure(const ruis::vector2& quotum) const noexcept override;
	};

	struct provider : public ruis::list_provider {
		code_edit& owner;

		provider(code_edit& owner) :
			list_provider(owner.context),
			owner(owner)
		{}

		size_t count() const noexcept override
		{
			return this->owner.lines.size();
		}

		utki::shared_ref<ruis::widget> get_widget(size_t index) override;
	};

	std::shared_ptr<provider> lines_provider;

	class cursor
	{
		code_edit& owner;
		r4::vector2<size_t> pos = 0;
		r4::vector2<size_t> sel_pos_glyphs = 0;

		void update_selection();

	public:
		cursor(code_edit& owner, r4::vector2<size_t> pos) :
			owner(owner),
			pos(pos)
		{
			this->pos = this->get_pos_glyphs();
			// TODO: refactor get_pos_glyphs() etc. to take pos arguemnt isntead of using pos member variable
			// NOLINTNEXTLINE(cppcoreguidelines-prefer-member-initializer)
			this->sel_pos_glyphs = this->pos; // no selection initially
		}

		size_t get_line_num() const noexcept;

		struct selection {
			// the segment points are sorted left to right and top to bottom
			r4::segment2<size_t> segment;

			// tells if the cursor is at the beginning (false) of the selection or at the end (true)
			bool is_left_to_right;

			r4::vector2<size_t> get_cursor_pos_glyphs() const noexcept
			{
				if (this->is_left_to_right) {
					return this->segment.p2;
				} else {
					return this->segment.p1;
				}
			}
		};

		selection get_selection_glyphs() const noexcept;

		// get position in characters
		r4::vector2<size_t> get_pos_chars() const noexcept;

		// get position in glyphs
		r4::vector2<size_t> get_pos_glyphs() const noexcept;

		void move_right_by(size_t dx) noexcept;
		void move_left_by(size_t dx) noexcept;
		void move_up_by(size_t dy) noexcept;
		void move_down_by(size_t dy) noexcept;

		void set_pos_chars(r4::vector2<size_t> pos) noexcept;

		void set_pos_glyphs(r4::vector2<size_t> pos) noexcept;

		void drop_selection() noexcept
		{
			this->sel_pos_glyphs = this->get_pos_glyphs(); // make selection pos same as cursor pos
		}

		bool is_selection_empty() const noexcept
		{
			auto s = this->get_selection_glyphs();
			return s.segment.p1 == s.segment.p2;
		}
	};

	bool cursor_blink_visible = true;

	std::vector<cursor> cursors;

	void for_each_cursor(const std::function<void(cursor&)>& func);

	// find cursors intersecting line_num by its selection
	std::vector<std::tuple<const cursor*, cursor::selection>> find_cursors(size_t line_num);

	void insert(cursor& c, std::u32string_view str);
	void erase_forward(cursor& c, size_t num);
	void erase_backward(cursor& c, size_t num);
	void put_new_line(cursor& c);

	size_t calc_word_length_forward(const cursor& c) const noexcept;
	size_t calc_word_length_backward(const cursor& c) const noexcept;

	bool text_changed = false;

	void update(uint32_t dt) override;
	void on_focus_change() override;
	void start_cursor_blinking();

	r4::vector2<size_t> mouse_pos_to_glyph_pos(const ruis::vector2& mouse_pos) const noexcept;

	bool mouse_selection = false;

	constexpr static auto default_text_color = 0xffb0b0b0;

	std::shared_ptr<synhi::font_style> text_style =
		std::make_shared<synhi::font_style>(synhi::font_style{.color = default_text_color});

	void notify_text_change();

	size_t num_lines_on_page() const noexcept;

	void scroll_to(r4::vector2<size_t> pos_glyphs);

public:
	struct all_parameters {
		ruis::layout_parameters layout_params;
		ruis::widget::parameters widget_params;
		ruis::text_widget::parameters text_params;
	};

	code_edit(
		utki::shared_ref<ruis::context> context, //
		all_parameters params
	);

	code_edit(const code_edit&) = delete;
	code_edit& operator=(const code_edit&) = delete;

	code_edit(code_edit&&) = delete;
	code_edit& operator=(code_edit&&) = delete;

	~code_edit() override = default;

	void render(const ruis::matrix4& matrix) const override;
	bool on_mouse_button(const ruis::mouse_button_event& event) override;
	bool on_mouse_move(const ruis::mouse_move_event& event) override;
	bool on_key(const ruis::key_event& e) override;
	void on_character_input(const ruis::character_input_event& e) override;

	using ruis::text_widget::set_text;

	void set_text(std::u32string text) override;
	std::u32string get_text() const override;

	void set_line_spans(decltype(line::spans)&& spans, size_t line_index);

	void set_line_spans(decltype(line::spans)&& spans, decltype(lines)::const_iterator i)
	{
		this->set_line_spans(std::move(spans), std::distance(this->lines.cbegin(), i));
	}

	const decltype(lines)& get_lines() const
	{
		return this->lines;
	}
};

} // namespace cod
