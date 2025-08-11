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

#pragma once

#include <ruis/context.hpp>
#include <ruis/widget/group/book.hpp>
#include <ruis/widget/group/tree_view.hpp>
#include <ruis/widget/widget.hpp>

#include "page.hpp"

namespace cod {

class file_tree_page :
	public page, //
	private ruis::container
{
	class file_tree_model
	{
	public:
		struct file_entry {
			bool is_directory;
			std::string name;
			// TODO: type

			bool children_read = false;
		};

	private:
		using file_entry_forest_type = utki::tree<file_entry>::container_type;

		mutable file_entry_forest_type cache;

		decltype(cache) read_files(utki::span<const size_t> index) const;

	public:
		file_tree_model();

		// throws std::out_of_bounds if index is out of this->cache
		std::string get_path(utki::span<const size_t> index) const;

		size_t count(utki::span<const size_t> index) const noexcept;

		const file_entry& get(utki::span<const size_t> index) const noexcept;
	};

	class file_tree_provider : public ruis::tree_view::provider
	{
		file_tree_page& owner;

		utki::shared_ref<file_tree_model> model;

	public:
		file_tree_provider(
			utki::shared_ref<ruis::context> context, //
			utki::shared_ref<file_tree_model> model,
			file_tree_page& owner
		);

		size_t count(utki::span<const size_t> index) const noexcept override;

		utki::shared_ref<ruis::widget> get_widget(utki::span<const size_t> index) override;
	};

	std::vector<size_t> cursor_index;

	utki::shared_ref<file_tree_model> model;

	void notify_file_select(std::string file_path);

	file_tree_page(
		utki::shared_ref<ruis::context> context, //
		utki::shared_ref<file_tree_model> model
	);

public:
	file_tree_page(utki::shared_ref<ruis::context> context);

	std::function<void(std::string)> file_select_handler;

	void render(const ruis::matrix4& matrix) const override
	{
		this->ruis::container::render(matrix);
		this->page::render(matrix);
	}

	utki::shared_ref<ruis::widget> create_tab_content() override;
};

} // namespace cod
