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

#include "file_tree.hpp"

#include <utki/tree.hpp>
#include <utki/linq.hpp>

#include <papki/fs_file.hpp>

#include <morda/widgets/label/text.hpp>
#include <morda/widgets/label/color.hpp>
#include <morda/widgets/slider/scroll_bar.hpp>
#include <morda/widgets/proxy/click_proxy.hpp>

#include "application.hpp"

using namespace cod;

namespace{
const treeml::forest layout = treeml::read(R"qwertyuiop(
	@row{
		layout{
			dx{fill} dy{0}
			weight{1}
		}
		@tree_view{
			id{tree_view}
			clip{true}
			layout{
				dx{0} dy{fill}
				weight{1}
			}
		}
		@vertical_scroll_bar{
			id{vertical_scroll}

			layout{
				dx{min} dy{max}
			}
		}
	}
	@horizontal_scroll_bar{
		id{horizontal_scroll}
		layout{
			dx{max} dy{min}
		}
	}
)qwertyuiop");
}

std::string file_tree::file_tree_provider::make_path(utki::span<const size_t> index, const file_entry_forest_type& fef){
	auto cur_file_list = &fef;
	std::string dir_name;
	for(auto i = index.begin(); i != index.end(); ++i){
		ASSERT(*i < cur_file_list->size())
		auto& f = (*cur_file_list)[*i];
		ASSERT(f.value.is_directory)
		dir_name.append(f.value.name);
		if(f.value.is_directory){
			dir_name.append("/");
		}else{
			ASSERT(std::next(i) == index.end())
		}
		cur_file_list = &f.children;
	}
	return dir_name;
}

auto file_tree::file_tree_provider::read_files(utki::span<const size_t> index)const -> decltype(cache){
#ifdef DEBUG
	for(auto& i : index){
		LOG([&](auto&o){o << " " << i;})
	}
	LOG([&](auto&o){o << std::endl;})
#endif

	auto dir_name = cod::application::inst().cla.base_dir + make_path(index, this->cache);

	LOG([&](auto&o){o << "dir_name = " << dir_name << std::endl;})

	return utki::linq(papki::fs_file(dir_name).list_dir())
			.order_by([](const auto& v) -> const auto&{return v;})
			.select([](auto&& e){
					bool is_dir = papki::is_dir(e);
					return typename decltype(this->cache)::value_type(file_entry{
						is_dir,
						is_dir ? e.substr(0, e.size() - 1) : std::move(e)
					});
				}).get();
}

std::string file_tree::file_tree_provider::get_path(utki::span<const size_t> index)const noexcept{
	auto tr = utki::make_traversal(this->cache);

	auto iter = tr.make_iterator(index);

	std::stringstream ss;

	for(size_t i = 0; i != iter.depth(); ++i){
		if(i != 0){
			ss << "/";
		}
		ss << iter.at_level(i).value.name;
	}

	if(iter->value.is_directory){
		ss << "/";
	}

	return ss.str();
}

file_tree::file_tree_provider::file_tree_provider(file_tree& owner) :
		owner(owner),
		cache(read_files(utki::make_span<size_t>(nullptr, 0)))
{}

size_t file_tree::file_tree_provider::count(utki::span<const size_t> index)const noexcept{
	decltype(this->cache)* cur_file_list = &this->cache;
	for(auto i = index.begin(); i != index.end(); ++i){
		ASSERT(*i < cur_file_list->size())
		auto& f = (*cur_file_list)[*i];
		if(!f.value.is_directory){
			ASSERT(i == std::prev(index.end()))
			return 0;
		}
		if(!f.value.children_read){
			f.children = this->read_files(utki::make_span(index.data(), std::distance(index.begin(), i) + 1));
			f.value.children_read = true;
		}
		cur_file_list = &f.children;
	}

	return cur_file_list->size();
}

std::shared_ptr<morda::widget> file_tree::file_tree_provider::get_widget(utki::span<const size_t> index, bool is_collapsed){
	auto tr = utki::make_traversal(this->cache);
	ASSERT(tr.is_valid(index))
	auto& file_entry = tr[index];

	auto w = this->owner.context->inflater.inflate(R"(
		@pile{
			@click_proxy{
				id{cp}
				layout{
					dx{fill}
					dy{fill}
				}
			}
			@color{
				id{bg}
				layout{
					dx{fill}
					dy{fill}
				}
				color{${morda_color_highlight}}
				visible{false}
			}
			@text{
				id{tx}
			}
		}
	)");

	w->get_widget_as<morda::text>("tx").set_text(file_entry.value.name);

	if(utki::make_span(this->owner.cursor_index) == index){
		auto bg = w->try_get_widget_as<morda::color>("bg");
		ASSERT(bg)
		bg->set_visible(true);
	}

	auto& cp = w->get_widget_as<morda::click_proxy>("cp");

	cp.click_handler = [
			this,
			index = utki::make_vector(index)
		](morda::click_proxy& cp)
	{
		if(this->owner.cursor_index == index){
			return;
		}
		this->owner.cursor_index = index;
		this->notify_item_changed();
		this->owner.notify_file_select();
	};

	return w;
}

void file_tree::notify_file_select(){
	if(this->file_select_handler){
		this->file_select_handler(this->provider->get_path(utki::make_span(this->cursor_index)));
	}
}

file_tree::file_tree(std::shared_ptr<morda::context> c, const treeml::forest& desc) :
		morda::widget(std::move(c), desc),
		morda::column(this->context, layout)
{
	auto& tv = this->get_widget_as<morda::tree_view>("tree_view");

	auto& vs = this->get_widget_as<morda::scroll_bar>("vertical_scroll");
	auto& hs = this->get_widget_as<morda::scroll_bar>("horizontal_scroll");

	tv.scroll_change_handler = [vs = utki::make_weak_from(vs), hs = utki::make_weak_from(hs)](morda::tree_view& tv){
		auto f = tv.get_scroll_factor();
		auto b = tv.get_scroll_band();
		if(auto sb = hs.lock()){
			sb->set_fraction(f.x());
			sb->set_band_fraction(b.x());
		}
		if(auto sb = vs.lock()){
			sb->set_fraction(f.y());
			sb->set_band_fraction(b.y());
		}
	};

	vs.fraction_change_handler = [tv = utki::make_weak_from(tv)](morda::fraction_widget& fw){
		if(auto w = tv.lock()){
			w->set_vertical_scroll_factor(fw.fraction());
		}
	};

	hs.fraction_change_handler = [tv = utki::make_weak_from(tv)](morda::fraction_widget& fw){
		if(auto w = tv.lock()){
			w->set_horizontal_scroll_factor(fw.fraction());
		}
	};

	this->provider = std::make_shared<file_tree_provider>(*this);

	tv.set_provider(this->provider);
}
