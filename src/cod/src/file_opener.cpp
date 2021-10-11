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

#include <papki/fs_file.hpp>

#include <morda/widgets/group/tabbed_book.hpp>
#include <morda/widgets/label/text.hpp>

#include "editor_page.hpp"

using namespace cod;

namespace{
std::shared_ptr<morda::tab> inflate_tab(const std::shared_ptr<morda::tabbed_book>& tb, const std::string& name){
	auto t = tb->context->inflater.inflate_as<morda::tab>(R"(
		@tab{
			@row{
				@text{
					id{text}
					text{cube}
				}
				@push_button{
					id{close_button}
					@image{
						layout{
							dx { 8dp }
							dy { 8dp }
						}
						image{morda_img_close}
					}
				}
			}
		}
	)");
	t->get_widget_as<morda::text>("text").set_text(name);

	auto& close_btn = t->get_widget_as<morda::push_button>("close_button");
	
	close_btn.click_handler = [
			tabbed_book_wp = utki::make_weak(tb),
			tab_wp = utki::make_weak(t)
		](morda::push_button& btn)
	{
		auto tb = tabbed_book_wp.lock();
		ASSERT(tb)

		auto t = tab_wp.lock();
		ASSERT(t)

		tb->tear_out(*t);
	};
	return t;
}
}

void file_opener::open(std::string_view file_name){
    auto tb = this->base_tiling_area->try_get_widget_as<morda::tabbed_book>("tabbed_book");
    ASSERT(tb)

    papki::fs_file f(file_name);
    auto t = f.load();

    auto p = std::make_shared<editor_page>(tb->context, treeml::forest());
    p->set_text(utki::to_utf32(std::string(reinterpret_cast<char*>(t.data()), t.size())));

    tb->add(
            inflate_tab(tb, f.not_dir()),
            p
        );
}
