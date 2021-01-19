#include "code_edit.hpp"

code_edit::code_edit(std::shared_ptr<morda::context> c, const puu::forest& desc) :
		widget(std::move(c), desc),
		character_input_widget(this->context),
		text_widget(this->context, desc)
{
	
}

void code_edit::on_character_input(const std::u32string& unicode, morda::key key){

}

void code_edit::set_text(std::u32string&& text){
	// TODO:
}

std::u32string code_edit::get_text()const{
	std::u32string ret;

	for(auto& l : this->lines){
		ret.append(l);
	}

	return ret;
}
