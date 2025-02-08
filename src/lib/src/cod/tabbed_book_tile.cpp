/*
cod - text editor

Copyright (C) 2021-2024  Ivan Gagis <igagis@gmail.com>

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

#include "tabbed_book_tile.hpp"

#include <ruis/widget/label/text.hpp>

using namespace cod;

tabbed_book_tile::tabbed_book_tile(const utki::shared_ref<ruis::context>& c, const tml::forest& desc) :
	ruis::widget(std::move(c), desc),
	tile(this->context, desc),
	tabbed_book(this->context, desc)
{}

namespace {

const tml::forest tab_desc = tml::read(R"(
		@tab{
			@row{
				// @text{
				// 	id{text}
				// 	text{cube}
				// }
				@widget{
					id{placeholder}
				}
				@push_button{
					id{close_button}
					@image{
						lp{
							dx { 8pp }
							dy { 8pp }
						}
						image{ruis_img_close}
					}
				}
			}
		}
	)");

// TODO: ruis::tab is not yet modernized
// utki::shared_ref<ruis::tab> make_tab(utki::shared_ref<ruis::context> c, std::vector<utki::shared_ref<ruis::widget>>
// contents){
// 	// clang-format off
// 	return ruis::tab(c,
// 	);
// 	// clang-format on
// }

} // namespace

void tabbed_book_tile::add(const utki::shared_ref<page>& p)
{
	auto tab = this->context.get().inflater.inflate_as<ruis::tab>(tab_desc);

	tab.get().get_widget("placeholder").replace_by(p.get().create_tab_content());

	tab.get().get_widget_as<ruis::push_button>("close_button").click_handler =
		[tabbed_book_wp = utki::make_weak_from(*this), tab_wp = utki::make_weak(tab)](ruis::push_button& btn) {
			auto tb = tabbed_book_wp.lock();
			ASSERT(tb)

			auto t = tab_wp.lock();
			ASSERT(t)

			btn.context.get().run_from_ui_thread([tb, t] {
				tb->tear_out(*t);
			});
		};

	this->ruis::tabbed_book::add(tab, p);
}
