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
	
    app.gui.context->inflater.register_widget<file_tree_page>("file_tree"); // TODO: remove
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

	// TODO: remove
	// c.get_widget_as<morda::tabbed_book>("left_panel");

    c.get_widget_as<file_tree_page>("file_tree")
            .file_select_handler = [](std::string file_name)
    {
        // std::cout << "file = " << file_name << '\n';

        if(papki::is_dir(file_name)){
            return;
        }

        context::inst().file_opener.open(file_name);
    };

	this->editors_tabbed_book = c.try_get_widget_as<tabbed_book_tile>("tabbed_book");
}

void gui::open_editor(std::shared_ptr<file_page> page){
	this->editors_tabbed_book->add(page);
}
