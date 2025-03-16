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

#include <ruis/widget/label/gap.hpp>
#include <ruis/widget/label/text.hpp>

using namespace std::string_literals;
using namespace std::string_view_literals;
using namespace ruis::length_literals;
using namespace cod;

namespace m {
using namespace ruis::make;
} // namespace m

tabbed_book_tile::tabbed_book_tile(
	utki::shared_ref<ruis::context> context,
	all_parameters params
) :
	ruis::widget(
		std::move(context), //
		std::move(params.layout_params),
		std::move(params.widget_params)
	),
	tile(this->context),
	tabbed_book(this->context, {})
{}

namespace {

utki::shared_ref<ruis::tab> make_tab(const utki::shared_ref<ruis::context>& c)
{
	// clang-format off
	return m::tab(c,
		{
			.container_params{
				.layout = ruis::layout::row
			}
		},
		{
			m::gap(c,
				{
					.widget_params{
						.id = "placeholder"s
					}
				}
			),
			m::push_button(c,
				{
					.widget_params{
						.id = "close_button"s
					}
				},
				{
					m::image(c,
						{
							.layout_params{
								.dims = {8_pp, 8_pp}
							},
							.image_params{
								.img = c.get().loader().load<ruis::res::image>("ruis_img_close"sv)
							}
						}
					)
				}
			)
		}
	);
	// clang-format on
}
} // namespace

void tabbed_book_tile::add(const utki::shared_ref<page>& p)
{
	auto tab = make_tab(this->context);

	tab.get().get_widget("placeholder").replace_by(p.get().create_tab_content());

	tab.get().get_widget_as<ruis::push_button>("close_button").click_handler =
		[tabbed_book_wp = utki::make_weak_from(*this), tab_wp = utki::make_weak(tab)](ruis::push_button& btn) {
			auto tb = tabbed_book_wp.lock();
			ASSERT(tb)

			auto t = tab_wp.lock();
			ASSERT(t)

			btn.context.get().post_to_ui_thread([tb, t] {
				tb->tear_out(*t);
			});
		};

	this->ruis::tabbed_book::add(tab, p);
}
