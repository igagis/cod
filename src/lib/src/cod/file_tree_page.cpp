/*
cod - text editor

Copyright (C) 2021-2025  Ivan Gagis <igagis@gmail.com>

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

#include "file_tree_page.hpp"

#include <papki/fs_file.hpp>
#include <ruis/widget/group/scroll_area.hpp>
#include <ruis/widget/label/rectangle.hpp>
#include <ruis/widget/label/text.hpp>
#include <ruis/widget/proxy/click_proxy.hpp>
#include <ruis/widget/proxy/mouse_proxy.hpp>
#include <ruis/widget/proxy/resize_proxy.hpp>
#include <ruis/widget/slider/scroll_bar.hpp>
#include <utki/linq.hpp>
#include <utki/tree.hpp>

#include "context.hpp"

using namespace std::string_literals;
using namespace std::string_view_literals;

using namespace cod;

auto file_tree_page::file_tree_model::read_files(utki::span<const size_t> index) const -> decltype(cache)
{
#ifdef DEBUG
	for (auto& i : index) {
		LOG([&](auto& o) {
			o << " " << i;
		})
	}
	LOG([&](auto& o) {
		o << std::endl;
	})
#endif

	auto dir_name = utki::cat(
		cod::context::inst().base_dir, //
		this->get_path(index)
	);

	utki::log_debug([&](auto& o) {
		o << "dir_name = " << dir_name << std::endl;
	});

	return utki::linq(papki::fs_file(dir_name).list_dir())
		.order_by([](const auto& v) -> const auto& {
			return v;
		})
		.select([](auto e) {
			bool is_dir = papki::is_dir(e);
			return typename decltype(this->cache)::value_type( //
				file_entry{
					.is_directory = is_dir, //
					.name = is_dir ? e.substr(0, e.size() - 1) : std::move(e)
				}
			);
		})
		.get();
}

file_tree_page::file_tree_model::file_tree_model() :
	cache(read_files(utki::make_span<size_t>(nullptr, 0)))
{
	std::cout << "cache.size() = " << this->cache.size() << std::endl;
}

std::string file_tree_page::file_tree_model::get_path(utki::span<const size_t> index) const
{
	if (index.empty()) {
		return {};
	}

	auto tr = utki::make_traversal(this->cache);

	auto iter = tr.make_iterator(index);

	std::stringstream ss;

	// TODO: refactor using ranges
	for (size_t i = 0; i != iter.depth(); ++i) {
		if (i != 0) {
			ss << "/";
		}
		ss << iter.at_level(i).value.name;
	}

	if (iter->value.is_directory) {
		ss << "/";
	}

	return ss.str();
}

file_tree_page::file_tree_provider::file_tree_provider(
	utki::shared_ref<ruis::context> context,
	utki::shared_ref<file_tree_model> model,
	file_tree_page& owner
) :
	provider(std::move(context)),
	owner(owner),
	model(std::move(model))
{}

size_t file_tree_page::file_tree_model::count(utki::span<const size_t> index) const noexcept
{
	decltype(this->cache)* cur_file_list = &this->cache;
	for (auto i = index.begin(); i != index.end(); ++i) {
		ASSERT(*i < cur_file_list->size())
		auto& f = (*cur_file_list)[*i];
		if (!f.value.is_directory) {
			ASSERT(i == std::prev(index.end()))
			return 0;
		}
		if (!f.value.children_read) {
			f.children = this->read_files(utki::make_span(index.data(), std::distance(index.begin(), i) + 1));
			f.value.children_read = true;
		}
		cur_file_list = &f.children;
	}

	return cur_file_list->size();
}

const file_tree_page::file_tree_model::file_entry& file_tree_page::file_tree_model::get(utki::span<const size_t> index
) const noexcept
{
	auto tr = utki::make_traversal(this->cache);
	utki::assert(tr.is_valid(index));
	return tr[index].value;
}

size_t file_tree_page::file_tree_provider::count(utki::span<const size_t> index) const noexcept
{
	return this->model.get().count(index);
}

utki::shared_ref<ruis::widget> file_tree_page::file_tree_provider::get_widget(utki::span<const size_t> index)
{
	const auto& file_entry = this->model.get().get(index);

	namespace m = ruis::make;

	auto& c = this->context;

	// clang-format off
	auto w = m::pile(c,
		{},
		{
			m::click_proxy(c,
				{
					.layout_params = {
						.dims = {ruis::dim::fill, ruis::dim::fill}
					},
					.widget_params = {
						.id = "cp"s
					}
				}
			),
			m::rectangle(c,
				{
					.layout_params = {
						.dims = {ruis::dim::fill, ruis::dim::fill}
					},
					.widget_params = {
						.id = "bg"s,
						.visible = false
					},
					.color_params = {
						.color = 0xffff8080 // NOLINT(cppcoreguidelines-avoid-magic-numbers, "TODO: fix")
					}
				}
			),
			m::text(c,
				{
					.widget_params = {
						.id = "tx"s
					}
				},
				{}
			)
		}
	);
	// clang-format on

	w.get().get_widget_as<ruis::text>("tx").set_text(file_entry.name);

	if (utki::deep_equals(utki::make_span(this->owner.cursor_index), index)) {
		auto& bg = w.get().get_widget_as<ruis::rectangle>("bg");
		bg.set_visible(true);
	}

	auto& cp = w.get().get_widget_as<ruis::click_proxy>("cp");

	cp.click_handler = [this, index = utki::make_vector(index)](ruis::click_proxy& cp) {
		if (this->owner.cursor_index == index) {
			return;
		}
		this->owner.cursor_index = index;
		this->notify_item_changed();
		this->owner.notify_file_select(this->owner.model.get().get_path(index));
	};

	return w;
}

void file_tree_page::notify_file_select(std::string file_path)
{
	if (this->file_select_handler) {
		this->file_select_handler(std::move(file_path));
	}
}

namespace {
std::vector<utki::shared_ref<ruis::widget>> make_page_widgets(
	const utki::shared_ref<ruis::context>& c, //
	utki::shared_ref<ruis::tree_view::provider> p
)
{
	namespace m = ruis::make;

	// clang-format off
	return {
		m::row(c,
			{
				.layout_params{
					.dims = {ruis::dim::fill, ruis::dim::fill},
					.weight = 1
				}
			},
			{
				m::scroll_area(c,
					{
						.layout_params{
							.dims = {ruis::dim::fill, ruis::dim::fill},
							.weight = 1
						},
						.widget_params{
							.id = "scroll_area"s,
							.clip = true
						}
					},
					{
						m::tree_view(c,
							{
								.layout_params{
									.dims = {ruis::dim::min, ruis::dim::fill}
								},
								.widget_params{
									.id = "tree_view"s
								},
								.tree_view_params{
									.provider = std::move(p)
								}
							}
						)
					}
				),
				m::scroll_bar(c,
					{
						.layout_params{
							.dims = {ruis::dim::min, ruis::dim::max}
						},
						.widget_params = {
							.id = "vertical_scroll"s
						},
						.oriented_params = {
							.vertical = true
						}
					}
				)
			}
		),
		m::scroll_bar(c,
			{
				.layout_params{
					.dims = {ruis::dim::max, ruis::dim::min}
				},
				.widget_params = {
					.id = "horizontal_scroll"s
				},
				.oriented_params = {
					.vertical = false
				}
			}
		)
	};
	// clang-format on
}
} // namespace

file_tree_page::file_tree_page(utki::shared_ref<ruis::context> context) :
	file_tree_page(
		context, //
		utki::make_shared<file_tree_model>()
	)
{}

file_tree_page::file_tree_page(
	utki::shared_ref<ruis::context> context, //
	utki::shared_ref<file_tree_model> model
) :
	ruis::widget(std::move(context), {}, {}),
	page(this->context),
	// clang-format off
	ruis::container(
		this->context,
		{
			.container_params{
				.layout = ruis::layout::column
			}
		},
		make_page_widgets(
			this->context, //
			utki::make_shared<file_tree_provider>(
				this->context,
				model,
				*this
			)
		)
	),
	// clang-format on
	model(std::move(model))
{
	auto& tv = this->get_widget_as<ruis::tree_view>("tree_view"sv);
	auto& sa = this->get_widget_as<ruis::scroll_area>("scroll_area"sv);

	auto& vs = this->get_widget_as<ruis::scroll_bar>("vertical_scroll"sv);
	auto& hs = this->get_widget_as<ruis::scroll_bar>("horizontal_scroll"sv);

	tv.scroll_change_handler = //
		[vs = utki::make_weak_from(vs)] //
		(ruis::tree_view & tv) //
	{
		auto f = tv.get_scroll_factor();
		auto b = tv.get_scroll_band();
		if (auto sb = vs.lock()) {
			sb->set_fraction(
				f, //
				false
			);
			sb->set_band_fraction(b);
		}
	};

	sa.scroll_change_handler = [hs = utki::make_weak_from(hs)](ruis::scroll_area& sa) {
		if (auto sb = hs.lock()) {
			sb->set_fraction(
				sa.get_scroll_factor().x(), //
				false
			);
			sb->set_band_fraction(sa.get_visible_area_fraction().x());
		}
	};

	vs.fraction_change_handler = [tv = utki::make_weak_from(tv)](ruis::fraction_widget& fw) {
		if (auto w = tv.lock()) {
			w->set_scroll_factor(fw.get_fraction());
		}
	};

	hs.fraction_change_handler = [sa = utki::make_weak_from(sa)](ruis::fraction_widget& fw) {
		if (auto w = sa.lock()) {
			w->set_scroll_factor({fw.get_fraction(), 0});
		}
	};
}

utki::shared_ref<ruis::widget> file_tree_page::create_tab_content()
{
	return ruis::make::text(
		this->context, //
		{},
		U"file tree"s
	);
}
