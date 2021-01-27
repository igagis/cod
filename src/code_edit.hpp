#pragma once

#include <vector>

#include <morda/widgets/character_input_widget.hpp>
#include <morda/widgets/base/text_widget.hpp>
#include <morda/widgets/group/list.hpp>
#include <morda/widgets/group/pile.hpp>

class code_edit :
		public morda::character_input_widget,
		public morda::text_widget,
		private morda::pile
{
	struct{
		morda::real advance;
		morda::real baseline;
	} font_info;

	void on_font_change()override{
		const auto& font = this->get_font().get();
		this->font_info.advance = font.get_advance(' ');
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
	struct line{
		std::u32string str;
		std::vector<std::pair<std::u32string_view, attributes>> spans;
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

public:
	code_edit(std::shared_ptr<morda::context> c, const puu::forest& desc);
	
	code_edit(const code_edit&) = delete;
	code_edit& operator=(const code_edit&) = delete;
	
	void on_character_input(const std::u32string& unicode, morda::key key)override;

	using morda::text_widget::set_text;

	void set_text(std::u32string&& text)override;
	std::u32string get_text()const override;
private:
};
