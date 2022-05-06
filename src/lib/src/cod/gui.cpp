#include "gui.hpp"

#include <morda/widgets/group/tabbed_book.hpp>
#include <morda/widgets/label/text.hpp>

#include "context.hpp"
#include "file_tree.hpp"
#include "tabbed_book_tile.hpp"

using namespace cod;

namespace{

const treeml::forest tab_desc = treeml::read(R"(
		@tab{
			@row{
				@text{
					id{text}
					text{cube}
				}
				@push_button{
					id{close_button}
					@image{
						layout{
							dx { 8dp }
							dy { 8dp }
						}
						image{morda_img_close}
					}
				}
			}
		}
	)");

}

gui::gui(mordavokne::application& app) :
        morda_context(app.gui.context)
{
    app.gui.initStandardWidgets(*app.get_res_file());
	
    app.gui.context->inflater.register_widget<file_tree>("file_tree");
    app.gui.context->inflater.register_widget<tiling_area>("tiling_area");
	app.gui.context->inflater.register_widget<tabbed_book_tile>("tabbed_book_tile");

    app.gui.context->loader.mount_res_pack(
            *app.get_res_file("res/")
        );

    app.gui.set_root(
            app.gui.context->inflater.inflate(
                    *app.get_res_file("res/main.gui")
                )
        );

    ASSERT(app.gui.get_root())
    auto& c = *app.gui.get_root();

    c.get_widget_as<file_tree>("file_tree")
            .file_select_handler = [](std::string file_name)
    {
        // std::cout << "file = " << file_name << '\n';

        if(papki::is_dir(file_name)){
            return;
        }

        context::inst().file_opener.open(file_name);
    };

	this->editors_tabbed_book = c.try_get_widget_as<morda::tabbed_book>("tabbed_book");
}

void gui::open_editor(std::shared_ptr<editor_page> page){

	auto tab = this->editors_tabbed_book->context->inflater.inflate_as<morda::tab>(tab_desc);

	tab->get_widget_as<morda::text>("text").set_text(papki::not_dir(page->get_file_name()));
	
	tab->get_widget_as<morda::push_button>("close_button").click_handler = [
			tabbed_book_wp = utki::make_weak(this->editors_tabbed_book),
			tab_wp = utki::make_weak(tab)
		](morda::push_button& btn)
	{
		auto tb = tabbed_book_wp.lock();
		ASSERT(tb)

		auto t = tab_wp.lock();
		ASSERT(t)

		btn.context->run_from_ui_thread([tb, t]{
			tb->tear_out(*t);
		});
	};

    this->editors_tabbed_book->add(tab, page);
}
