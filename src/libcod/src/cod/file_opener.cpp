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

#include "context.hpp"

using namespace cod;

namespace{

const treeml::forest tab_desc = treeml::read(R"(
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
}

void file_opener::open(const std::string& file_name){
	{
		auto i = this->open_files.find(file_name);
		if(i != this->open_files.end()){
			i->second->activate();
			return;
		}
	}

	auto& ctx = context::inst();

    auto& book = ctx.gui.get_tiling_area()->get_widget_as<morda::tabbed_book>("tabbed_book");

	auto page = ctx.plugins.open_file(file_name);
	ASSERT(page)

	auto tab = book.context->inflater.inflate_as<morda::tab>(tab_desc);

	auto iter = this->open_files.insert(std::make_pair(file_name, tab));
	ASSERT(iter.second)
	utki::scope_exit scope_exit([this, iter = iter.first]{
		this->open_files.erase(iter);
	});

	tab->get_widget_as<morda::text>("text").set_text(papki::not_dir(file_name));
	
	tab->get_widget_as<morda::push_button>("close_button").click_handler = [
			tabbed_book_wp = utki::make_weak_from(book),
			tab_wp = utki::make_weak(tab),
			iter = iter.first,
			this // should be fine, since file_opener instance is in application singleton
		](morda::push_button& btn)
	{
		auto tb = tabbed_book_wp.lock();
		ASSERT(tb)

		auto t = tab_wp.lock();
		ASSERT(t)

		btn.context->run_from_ui_thread([tb, t]{
			tb->tear_out(*t);
		});

		this->open_files.erase(iter);
	};

    book.add(tab, page);

	scope_exit.release();
}
