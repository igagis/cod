#include "tiling_area.hpp"

using namespace cod;

tiling_area::tiling_area(std::shared_ptr<morda::context> c, const treeml::forest& desc) :
        morda::widget(std::move(c), desc),
        morda::container(this->context, treeml::forest())
{
    this->content = std::make_shared<morda::linear_container>(this->context, treeml::forest(), false);
    this->push_back(this->content);
}

void tiling_area::push_back(std::shared_ptr<widget> w){
    this->content->push_back(std::move(w));
}
