#include "gui.hpp"

#include "context.hpp"

#include "file_tree.hpp"

using namespace cod;

gui::gui(mordavokne::application& app, context& owner) :
        owner(owner)
{
    app.gui.initStandardWidgets(*app.get_res_file());
	
    app.gui.context->inflater.register_widget<file_tree>("file_tree");
    app.gui.context->inflater.register_widget<tiling_area>("tiling_area");

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

    this->editors_tiling_area = c.try_get_widget_as<tiling_area>("base_tiling_area");
}

void gui::open_editor(std::shared_ptr<editor_page> page){
    
}
