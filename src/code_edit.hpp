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
		std::shared_ptr<morda::res::font> regular;
		std::shared_ptr<morda::res::font> bold;
		std::shared_ptr<morda::res::font> italic;
		std::shared_ptr<morda::res::font> bold_italic;
	} font;

	struct{
		unsigned tab_size = 4;
	} settings;

	struct attributes{
		bool bold;
		bool italic;
		bool underlined;
		bool stroked;
		uint32_t color;
	};
	struct line{
		std::u32string str;
		std::vector<std::pair<std::u32string_view, attributes>> spans;
	};

	std::vector<line> lines;

	class line_widget : virtual public morda::widget{
		std::weak_ptr<code_edit> ce;
	public:
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
