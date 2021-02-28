#pragma once

#include <morda/widgets/widget.hpp>
#include <morda/context.hpp>

class file_tree : virtual public morda::widget{

public:
	file_tree(std::shared_ptr<morda::context> c, const treeml::forest& desc);

};
