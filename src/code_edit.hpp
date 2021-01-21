#pragma once

#include <vector>

#include <morda/widgets/character_input_widget.hpp>
#include <morda/widgets/base/text_widget.hpp>

class code_edit :
		public morda::character_input_widget,
		public morda::text_widget
{
	struct attributes{

	};
	struct line{
		std::u32string str;
		std::vector<std::pair<std::u32string_view, attributes>> spans;
	};

	std::vector<line> lines;

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
