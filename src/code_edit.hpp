#pragma once

#include <morda/widgets/character_input_widget.hpp>

class CodeEdit : public morda::character_input_widget{
public:
	CodeEdit(std::shared_ptr<morda::context> c, const puu::forest& desc);
	
	CodeEdit(const CodeEdit&) = delete;
	CodeEdit& operator=(const CodeEdit&) = delete;
	
	
	void on_character_input(const std::u32string& unicode, morda::key key)override;
private:

};
