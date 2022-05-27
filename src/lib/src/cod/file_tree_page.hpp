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

#pragma once

#include <morda/widgets/widget.hpp>
#include <morda/widgets/group/book.hpp>
#include <morda/widgets/group/column.hpp>
#include <morda/widgets/group/tree_view.hpp>
#include <morda/context.hpp>

#include "page.hpp"

namespace cod{

class file_tree_page :
		virtual public morda::widget,
		public page,
		private morda::column
{
	class file_tree_provider : public morda::tree_view::provider{
		file_tree_page& owner;

		struct file_entry{
			bool is_directory;
			std::string name;
			// TODO: type

			bool children_read = false;
		};

		typedef utki::tree<file_entry>::container_type file_entry_forest_type;

		mutable file_entry_forest_type cache;

		decltype(cache) read_files(utki::span<const size_t> index)const;

		static std::string make_path(utki::span<const size_t> index, const file_entry_forest_type& fef);
	public:
		file_tree_provider(file_tree_page& owner);
		size_t count(utki::span<const size_t> index)const noexcept override;
		std::shared_ptr<morda::widget> get_widget(utki::span<const size_t> index, bool is_collapsed)override;

		std::string get_path(utki::span<const size_t> index)const; // TODO: make noexcept, right now linter is angry about it
	};

	std::shared_ptr<file_tree_provider> provider;

	std::vector<size_t> cursor_index;

	void notify_file_select();
public:
	file_tree_page(std::shared_ptr<morda::context> c);

	std::function<void(std::string)> file_select_handler;

	void render(const morda::matrix4& matrix)const override{
		this->morda::column::render(matrix);
		this->page::render(matrix);
	}

	std::string_view get_name()const override;
};

}
