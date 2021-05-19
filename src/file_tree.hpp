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
	std::shared_ptr<morda::tree_view::provider> provider;
public:
	file_tree(std::shared_ptr<morda::context> c, const treeml::forest& desc);

	std::function<void(utki::span<const size_t>)> file_select_handler;
};

}
