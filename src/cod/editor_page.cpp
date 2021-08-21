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

#include "editor_page.hpp"

#include "syntax_highlight/regex_syntax_highlighter.hpp"

#include <papki/fs_file.hpp>

using namespace cod;

editor_page::editor_page(std::shared_ptr<morda::context> context, const treeml::forest& desc) :
		morda::widget(std::move(context), desc),
		morda::page(this->context, desc),
		code_edit(this->context, desc)
{
	// TODO: for now we set XML syntax highlighter for each code edit page,
	//       later need to implement proper system
	this->text_change_handler = [
			this,
			hl = std::make_shared<cod::regex_syntax_highlighter>(treeml::read(papki::fs_file("highlight/xml.3ml")))
		]
	(morda::text_widget& w)
	{
		hl->reset();
		const auto& lines = this->get_lines();
		for(auto i = lines.begin(); i != lines.end(); ++i){
			this->set_line_spans(
				hl->highlight(i->str),
				i
			);
		}
	};
}
