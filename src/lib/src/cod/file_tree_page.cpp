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

#include "file_tree_page.hpp"

#include <papki/fs_file.hpp>
#include <ruis/widgets/group/tree_view.hpp>
#include <ruis/widgets/label/color.hpp>
#include <ruis/widgets/label/text.hpp>
#include <ruis/widgets/proxy/click_proxy.hpp>
#include <ruis/widgets/proxy/mouse_proxy.hpp>
#include <ruis/widgets/proxy/resize_proxy.hpp>
#include <ruis/widgets/slider/scroll_bar.hpp>
#include <utki/linq.hpp>
#include <utki/tree.hpp>

#include "context.hpp"

using namespace std::string_literals;

using namespace cod;

namespace {
const tml::forest layout = tml::read(R"qwertyuiop(
	layout{column}
	@row{
		lp{
			dx{fill} dy{fill}
			weight{1}
		}
		@tree_view{
			id{tree_view}
			clip{true}
			lp{
				dx{fill} dy{fill}
				weight{1}
			}
		}
		@vertical_scroll_bar{
			id{vertical_scroll}

			lp{
				dx{min} dy{max}
			}
		}
	}
	@horizontal_scroll_bar{
		id{horizontal_scroll}
		lp{
			dx{max} dy{min}
		}
	}
)qwertyuiop");
} // namespace

std::string file_tree_page::file_tree_provider::make_path(
	utki::span<const size_t> index,
	const file_entry_forest_type& fef
)
{
	auto cur_file_list = &fef;
	std::string dir_name;
	for (const auto& i : index) {
		ASSERT(i < cur_file_list->size())
		auto& f = (*cur_file_list)[i];
		ASSERT(f.value.is_directory)
		dir_name.append(f.value.name);
		if (f.value.is_directory) {
			dir_name.append("/");
		} else {
			// It should be the last index.
			// Since we are using range based for loop, at least check that
			// it is equal to the last index of the span.
			ASSERT(i == *index.rbegin())
		}
		cur_file_list = &f.children;
	}
	return dir_name;
}

auto file_tree_page::file_tree_provider::read_files(utki::span<const size_t> index) const -> decltype(cache)
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

	auto dir_name = utki::cat( //
		cod::context::inst().base_dir,
		make_path(index, this->cache)
	);

	LOG([&](auto& o) {
		o << "dir_name = " << dir_name << std::endl;
	})

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

std::string file_tree_page::file_tree_provider::get_path(utki::span<const size_t> index) const
{
	auto tr = utki::make_traversal(this->cache);

	auto iter = tr.make_iterator(index);

	std::stringstream ss;

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

file_tree_page::file_tree_provider::file_tree_provider(file_tree_page& owner) :
	owner(owner),
	cache(read_files(utki::make_span<size_t>(nullptr, 0)))
{}

size_t file_tree_page::file_tree_provider::count(utki::span<const size_t> index) const noexcept
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

utki::shared_ref<ruis::widget> file_tree_page::file_tree_provider::get_widget(
	utki::span<const size_t> index,
	bool is_collapsed
)
{
	auto tr = utki::make_traversal(this->cache);
	ASSERT(tr.is_valid(index))
	auto& file_entry = tr[index];

	auto w = this->owner.context.get().inflater.inflate(R"(
		@pile{
			@click_proxy{
				id{cp}
				lp{
					dx{fill}
					dy{fill}
				}
			}
			@color{
				id{bg}
				lp{
					dx{fill}
					dy{fill}
				}
				color{${ruis_color_highlight}}
				visible{false}
			}
			@text{
				id{tx}
			}
		}
	)");

	w.get().get_widget_as<ruis::text>("tx").set_text(file_entry.value.name);

	if (utki::make_span(this->owner.cursor_index) == index) {
		auto bg = w.get().try_get_widget_as<ruis::color>("bg");
		ASSERT(bg)
		bg->set_visible(true);
	}

	auto& cp = w.get().get_widget_as<ruis::click_proxy>("cp");

	cp.click_handler = [this, index = utki::make_vector(index)](ruis::click_proxy& cp) {
		if (this->owner.cursor_index == index) {
			return;
		}
		this->owner.cursor_index = index;
		this->notify_item_changed();
		this->owner.notify_file_select();
	};

	return w;
}

void file_tree_page::notify_file_select()
{
	if (this->file_select_handler) {
		this->file_select_handler(this->provider->get_path(utki::make_span(this->cursor_index)));
	}
}

namespace {
std::vector<utki::shared_ref<ruis::widget>> make_page_widgets(utki::shared_ref<ruis::context> c)
{
	namespace m = ruis::make;
	using ruis::lp;

	// clang-format off
	return {
		m::row(c,
			{
				.widget_params = {
					.lp ={
						.dims = {lp::fill, lp::fill},
						.weight = 1
					}
				}
			},
			{
				m::tree_view(c,
					{
						.widget_params = {
							.id = "tree_view"s,
							.lp = {
								.dims = {lp::fill, lp::fill},
								.weight = 1
							},
							.clip = true
						}
					}
				),
				m::scroll_bar(c,
					{
						.widget_params = {
							.id = "vertical_scroll"s,
							.lp = {
								.dims = {lp::min, lp::max}
							}
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
				.widget_params = {
					.id = "horizontal_scroll"s,
					.lp = {
						.dims = {lp::max, lp::min}
					}
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

file_tree_page::file_tree_page(const utki::shared_ref<ruis::context>& c) :
	ruis::widget(c, tml::forest()),
	page(this->context),
	ruis::container(
		this->context,
		{.container_params = {.layout = ruis::layout::column}},
		make_page_widgets(this->context)
	)
{
	auto& tv = this->get_widget_as<ruis::tree_view>("tree_view");

	auto& vs = this->get_widget_as<ruis::scroll_bar>("vertical_scroll");
	auto& hs = this->get_widget_as<ruis::scroll_bar>("horizontal_scroll");

	tv.scroll_change_handler = [vs = utki::make_weak_from(vs), hs = utki::make_weak_from(hs)](ruis::tree_view& tv) {
		auto f = tv.get_scroll_factor();
		auto b = tv.get_scroll_band();
		if (auto sb = hs.lock()) {
			sb->set_fraction(f.x());
			sb->set_band_fraction(b.x());
		}
		if (auto sb = vs.lock()) {
			sb->set_fraction(f.y());
			sb->set_band_fraction(b.y());
		}
	};

	vs.fraction_change_handler = [tv = utki::make_weak_from(tv)](ruis::fraction_widget& fw) {
		if (auto w = tv.lock()) {
			w->set_vertical_scroll_factor(fw.get_fraction());
		}
	};

	hs.fraction_change_handler = [tv = utki::make_weak_from(tv)](ruis::fraction_widget& fw) {
		if (auto w = tv.lock()) {
			w->set_horizontal_scroll_factor(fw.get_fraction());
		}
	};

	this->provider = std::make_shared<file_tree_provider>(*this);

	tv.set_provider(this->provider);
}

utki::shared_ref<ruis::widget> file_tree_page::create_tab_content()
{
	auto t = utki::make_shared<ruis::text>(this->context, tml::forest());
	t.get().set_text("file tree");
	return t;
}
