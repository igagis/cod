#pragma once

#include <morda/widgets/container.hpp>
#include <morda/widgets/group/linear_container.hpp>

namespace cod{

class tiling_area :
        virtual public morda::widget,
        private morda::container
{
    std::shared_ptr<morda::linear_container> content;
public:
    tiling_area(std::shared_ptr<morda::context> c, const treeml::forest& desc);

    void push_back(std::shared_ptr<widget> w);

    // TODO: override layout and measure
};

}
