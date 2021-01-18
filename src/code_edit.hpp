#pragma once

#include <morda/widgets/character_input_widget.hpp>
#include <morda/widgets/base/text_widget.hpp>
class code_edit :
		public morda::character_input_widget,
		public morda::text_widget
{
public:
	code_edit(std::shared_ptr<morda::context> c, const puu::forest& desc);
	
	code_edit(const code_edit&) = delete;
	code_edit& operator=(const code_edit&) = delete;
	
	void on_character_input(const std::u32string& unicode, morda::key key)override;
private:

};
