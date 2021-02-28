#include "file_tree.hpp"

using namespace cod;

namespace{
const treeml::forest layout = treeml::read(R"qwertyuiop(
	@row{
		layout{
			dx{fill} dy{0}
			weight{1}
		}
		@tree_view{
			id{tree_view}
			clip{true}
			layout{
				dx{0} dy{fill}
				weight{1}
			}
		}
		@vertical_scroll_bar{
			id{vertical_scroll}

			layout{
				dx{min} dy{max}
			}
		}
	}
	@horizontal_scroll_bar{
		id{treeview_vertical_slider}
		layout{
			dx{max} dy{min}
		}
	}
)qwertyuiop");
}

file_tree::file_tree(std::shared_ptr<morda::context> c, const treeml::forest& desc) :
		morda::widget(std::move(c), desc),
		morda::column(this->context, layout)
{

}
