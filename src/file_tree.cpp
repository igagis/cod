#include "file_tree.hpp"

file_tree::file_tree(std::shared_ptr<morda::context> c, const treeml::forest& desc) :
		morda::widget(std::move(c), desc)
{}
