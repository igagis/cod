/*
cod - text editor

Copyright (C) 2021  Ivan Gagis <igagis@gmail.com>

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

#include <morda/widgets/group/book.hpp>

#include "page.hpp"
#include "code_edit.hpp"

namespace cod{

class text_editor_page :
		public file_page,
		private code_edit
{
public:
	text_editor_page(std::shared_ptr<morda::context> context);

	void set_text(std::u32string&& text)override{
		this->code_edit::set_text(std::move(text));
	}

	void on_show()override{
		this->focus();
	}

	void on_hide()noexcept override{
		this->unfocus();
	}
};

}
