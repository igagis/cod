#pragma once

#include <morda/widgets/widget.hpp>
#include <morda/paint/frame_vao.hpp>

namespace cod{

class tile : virtual public morda::widget{
	mutable morda::frame_vao selection_vao;
public:
	tile(std::shared_ptr<morda::context> c, const treeml::forest& desc);

	void render(const morda::matrix4& matrix)const override;

	bool on_key(const morda::key_event& e)override;
};

}
