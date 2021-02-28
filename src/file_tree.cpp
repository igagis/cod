#include "file_tree.hpp"

#include <utki/tree.hpp>

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

namespace{
struct file_tree_provider : public morda::tree_view::provider{
	struct file_entry{
		bool is_directory;
		std::string name;
		// TODO: type
	};

	utki::tree<file_entry> file_tree;

	size_t count(const std::vector<size_t>& index)const noexcept override{
		return 0;
	}

	std::shared_ptr<morda::widget> get_widget(const std::vector<size_t>& index, bool is_collapsed)override{
		return nullptr;
	}
};
}

file_tree::file_tree(std::shared_ptr<morda::context> c, const treeml::forest& desc) :
		morda::widget(std::move(c), desc),
		morda::column(this->context, layout)
{
	auto& tv = this->get_widget_as<morda::tree_view>("tree_view");

	this->provider = std::make_shared<file_tree_provider>();

	tv.set_provider(this->provider);
}
