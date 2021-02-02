#pragma once

#include <vector>

#include <morda/widgets/character_input_widget.hpp>
#include <morda/widgets/base/text_widget.hpp>
#include <morda/widgets/group/list.hpp>
#include <morda/widgets/group/pile.hpp>

#include <morda/updateable.hpp>

class code_edit :
		public morda::character_input_widget,
		public morda::text_widget,
		private morda::pile,
		private morda::updateable
{
	struct{
		morda::vector2 glyph_dims;
		morda::real baseline;
	} font_info;

	void on_font_change()override{
		const auto& font = this->get_font().get();

		this->font_info.glyph_dims.set(font.get_advance(' '), font.get_height());
		this->font_info.baseline = round((font.get_height() + font.get_ascender() - font.get_descender()) / 2);
	}

	struct{
		unsigned tab_size = 4;
	} settings;

	struct attributes{
		morda::res::font::style style = morda::res::font::style::normal;
		bool underlined = false;
		bool stroked = false;
		uint32_t color = 0xffffffff;
	};

	struct line_span{
		size_t length;
		std::shared_ptr<attributes> attrs;
	};
	struct line{
		std::u32string str;
		std::vector<line_span> spans;

		void extend_line_span(size_t at_char_index, size_t by_length);
	};

	std::vector<line> lines;

	class line_widget : public morda::widget{
		code_edit& owner;
		size_t line_num;
	public:
		line_widget(std::shared_ptr<morda::context> c, code_edit& owner, size_t line_num) :
				widget(std::move(c), puu::forest()),
				owner(owner),
				line_num(line_num)
		{}

		void render(const morda::matrix4& matrix)const override;

		morda::vector2 measure(const morda::vector2& quotum)const noexcept override;
	};

	struct provider : public morda::list_widget::provider{
		code_edit& owner;

		provider(code_edit& owner) : owner(owner){}

		size_t count()const noexcept override{
			return this->owner.lines.size();
		}

		std::shared_ptr<morda::widget> get_widget(size_t index)override;
	};

	std::shared_ptr<provider> lines_provider;

	class cursor{
		code_edit& owner;
		r4::vector2<size_t> pos = 0;
	public:
		cursor(code_edit& owner, r4::vector2<size_t> pos) :
				owner(owner),
				pos(pos)
		{}

		r4::vector2<size_t> get_effective_pos()const noexcept;

		void set_pos(r4::vector2<size_t> pos)noexcept{
			this->pos = pos;
			this->owner.start_cursor_blinking();
		}
	};

	bool cursor_blink_visible = true;

	std::vector<cursor> cursors;

	void update(uint32_t dt)override;
	void on_focus_change()override;
	void start_cursor_blinking();

	bool on_mouse_button(const morda::mouse_button_event& event)override;

	void render(const morda::matrix4& matrix)const override;

	void render_cursors(const morda::matrix4& matrix)const;

	bool on_key(bool is_down, morda::key key)override;

	void on_character_input(const std::u32string& unicode, morda::key key)override;

	std::shared_ptr<attributes> text_style = std::make_shared<attributes>(attributes{color: 0xffb0b0b0});

	void notify_text_change();
public:
	code_edit(std::shared_ptr<morda::context> c, const puu::forest& desc);
	
	code_edit(const code_edit&) = delete;
	code_edit& operator=(const code_edit&) = delete;

	using morda::text_widget::set_text;

	void set_text(std::u32string&& text)override;
	std::u32string get_text()const override;
};
