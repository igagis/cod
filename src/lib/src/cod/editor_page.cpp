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

#include "context.hpp"
#include "file_opener.hpp"

using namespace cod;

editor_page::editor_page(std::shared_ptr<morda::context> context) :
		morda::widget(std::move(context), treeml::forest()),
		morda::page(this->context, treeml::forest())
{}

void editor_page::on_tear_out()noexcept{

	// remove the page from list of open files

	auto& ctx = context::inst();

	auto i = ctx.file_opener.open_files.find(file_name);
	if(i != ctx.file_opener.open_files.end()){
		ctx.file_opener.open_files.erase(i);
	}
}
