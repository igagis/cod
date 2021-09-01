#pragma once

#include <morda/widgets/container.hpp>

namespace cod{

class tiling_area :
        virtual public morda::widget,
        private morda::container
{
public:
    tiling_area(std::shared_ptr<morda::context> c, const treeml::forest& desc);
};

}
