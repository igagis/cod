#include "tiling_area.hpp"

using namespace cod;

tiling_area::tiling_area(std::shared_ptr<morda::context> c, const treeml::forest& desc) :
        morda::widget(std::move(c), desc),
        morda::container(this->context, treeml::forest())
{}
