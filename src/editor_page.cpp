/*
cod text editor

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

using namespace cod;

editor_page::editor_page(std::shared_ptr<morda::context> context, const treeml::forest& desc) :
		morda::widget(std::move(context), desc),
		morda::page(this->context, desc),
		code_edit(this->context, desc)
{
// 	this->set_text(
// R"qwertyuiop(Hello world!
// second line
// third line
// very very long line lorem ipsum dolor sit amet consecteteur blah blag
// ef
// ef
// qw
// ef
// wqef
// we
// fw
// ef
// we 
// fwe 
// fwe 
// fw e
// fwe we
// f w
// ef 
// we
// f we
// f we
// f
// we 
// fwe

// fwqe
// fwe

// fwqe
// 	f
// 	wqe
// 	f
// 	wqf


// wef wqe)qwertyuiop");
}
