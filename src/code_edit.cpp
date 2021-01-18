#include "code_edit.hpp"

code_edit::code_edit(std::shared_ptr<morda::context> c, const puu::forest& desc) :
		widget(std::move(c), desc),
		character_input_widget(this->context)
{
	
}

void code_edit::on_character_input(const std::u32string& unicode, morda::key key){

}
