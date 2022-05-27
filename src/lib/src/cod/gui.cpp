#include "gui.hpp"

#include <morda/widgets/group/tabbed_book.hpp>

#include "context.hpp"
#include "file_tree_page.hpp"
#include "tabbed_book_tile.hpp"

using namespace cod;

gui::gui(mordavokne::application& app) :
        morda_context(app.gui.context)
{
    app.gui.initStandardWidgets(*app.get_res_file());

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

	{
		auto ft = std::make_shared<file_tree_page>(c.context);

		ft->file_select_handler = [](std::string file_name){
			// std::cout << "file = " << file_name << '\n';

			if(papki::is_dir(file_name)){
				return;
			}

			context::inst().file_opener.open(file_name);
		};

		c.get_widget_as<tabbed_book_tile>("left_panel").add(ft);
	}

	this->editors_tabbed_book = c.try_get_widget_as<tabbed_book_tile>("tabbed_book");
}

void gui::open_editor(std::shared_ptr<file_page> page){
	this->editors_tabbed_book->add(page);
}
