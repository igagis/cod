#pragma once

#include <morda/widgets/widget.hpp>
#include <morda/widgets/group/column.hpp>
#include <morda/widgets/group/tree_view.hpp>
#include <morda/context.hpp>

namespace cod{

class file_tree :
		virtual public morda::widget,
		private morda::column
{
	class file_tree_provider : public morda::tree_view::provider{
		file_tree& owner;

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
		file_tree_provider(file_tree& owner);
		size_t count(utki::span<const size_t> index)const noexcept override;
		std::shared_ptr<morda::widget> get_widget(utki::span<const size_t> index, bool is_collapsed)override;
	};

	std::shared_ptr<file_tree_provider> provider;

	std::vector<size_t> cursor_index;

public:
	file_tree(std::shared_ptr<morda::context> c, const treeml::forest& desc);

	std::function<void(utki::span<const size_t>)> file_select_handler;
};

}
