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

#include "plugin.hpp"

#include <papki/fs_file.hpp>

#include <cod/text_editor_page.hpp>

using namespace cod;

std::shared_ptr<page> text_editor_plugin::open_file(const std::shared_ptr<morda::context>& context, std::string_view file_name){
    auto page = std::make_shared<text_editor_page>(context);
    page->set_text(
			utki::to_utf32(
					utki::make_string(
							papki::fs_file(file_name).load()
						)
				)
		);

    return page;
}
