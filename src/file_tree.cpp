#include "file_tree.hpp"

#include <utki/tree.hpp>
#include <utki/linq.hpp>

#include <papki/fs_file.hpp>

#include <morda/widgets/label/text.hpp>

#include "application.hpp"

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
class file_tree_provider : public morda::tree_view::provider{
	std::shared_ptr<morda::context> context;

	struct file_entry{
		bool is_directory;
		std::string name;
		// TODO: type

		bool children_read = false;
	};

	mutable utki::tree<file_entry>::container_type cache;

	decltype(cache) read_files(utki::span<const size_t> index)const{
#ifdef DEBUG
		for(auto& i : index){
			LOG(" " << i)
		}
		LOG(std::endl)
#endif
		auto cur_file_list = &this->cache;
		std::string dir_name = cod::application::inst().cla.base_dir;
		for(auto i = index.begin(); i != index.end(); ++i){
			ASSERT(*i < cur_file_list->size())
			auto& f = (*cur_file_list)[*i];
			ASSERT(f.value.is_directory)
			dir_name.append(f.value.name).append("/");
			cur_file_list = &f.children;
		}

		LOG("dir_name = " << dir_name << std::endl)

		return utki::linq(papki::fs_file(dir_name).list_dir()).select([](auto&& e){
			bool is_dir = papki::is_dir(e);
			return typename decltype(this->cache)::value_type(file_entry{
				is_dir,
				is_dir ? e.substr(0, e.size() - 1) : std::move(e)
			});
		}).get();
	}

public:
	file_tree_provider(std::shared_ptr<morda::context> context) :
			context(std::move(context)),
			cache(read_files(utki::make_span<size_t>(nullptr, 0)))
	{}

	size_t count(const std::vector<size_t>& index)const noexcept override{
		decltype(this->cache)* cur_file_list = &this->cache;
		for(auto i = index.begin(); i != index.end(); ++i){
			ASSERT(*i < cur_file_list->size())
			auto& f = (*cur_file_list)[*i];
			if(!f.value.is_directory){
				ASSERT(i == --index.end())
				return 0;
			}
			if(!f.value.children_read){
				f.children = this->read_files(utki::make_span(index.data(), std::distance(index.begin(), i) + 1));
				f.value.children_read = true;
			}
			cur_file_list = &f.children;
		}

		return cur_file_list->size();
	}

	std::shared_ptr<morda::widget> get_widget(const std::vector<size_t>& index, bool is_collapsed)override{
		auto tr = utki::make_traversal(this->cache);
		ASSERT(tr.is_valid(index))
		auto& fe = tr[index];

		auto ret = std::make_shared<morda::text>(this->context, treeml::forest());
		ret->set_text(fe.value.name);
		return ret;
	}
};
}

file_tree::file_tree(std::shared_ptr<morda::context> c, const treeml::forest& desc) :
		morda::widget(std::move(c), desc),
		morda::column(this->context, layout)
{
	auto& tv = this->get_widget_as<morda::tree_view>("tree_view");

	this->provider = std::make_shared<file_tree_provider>(this->context);

	tv.set_provider(this->provider);
}
