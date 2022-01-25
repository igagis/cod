#pragma once

#include <morda/widgets/widget.hpp>

namespace cod{

class tile : virtual public morda::widget{
public:
	tile(std::shared_ptr<morda::context> c, const treeml::forest& desc);
};

}
