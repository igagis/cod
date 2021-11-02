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

#include "file_opener.hpp"

#include "context.hpp"

using namespace cod;

void file_opener::open(const std::string& file_name){
	{
		auto i = this->open_files.find(file_name);
		if(i != this->open_files.end()){
			if(auto p = i->second.lock()){
				p->activate();
			}
			return;
		}
	}

	auto& ctx = context::inst();

	auto page = ctx.plugins.open_file(file_name);
	ASSERT(page)

	ctx.gui.open_editor(page);

	this->open_files.insert(std::make_pair(file_name, std::move(page)));
	ASSERT(iter.second)

	// on tear out the editor_page will remove itself from open_files list
}
