#pragma once

#include <morda/widgets/group/book.hpp>

#include "code_edit.hpp"

namespace cod{

class editor_page :
		public morda::page,
		private code_edit
{
public:
	editor_page(std::shared_ptr<morda::context> context, const treeml::forest& desc);

	void set_text(std::u32string&& text){
		this->code_edit::set_text(std::move(text));
	}
};

}
