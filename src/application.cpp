#include "application.hpp"

#include "tabbed_book.hpp"
#include "editor_page.hpp"

using namespace cod;

application::application(command_line_arguments&& cla) :
		mordavokne::application(
				"cod",
				[](){
					return mordavokne::window_params(r4::vector2<unsigned>(1024, 768));
				}()
			),
		cla(std::move(cla))
{
	this->gui.initStandardWidgets(*this->get_res_file());
	
	this->gui.context->inflater.register_widget<code_edit>("code_edit");
	this->gui.context->inflater.register_widget<file_tree>("file_tree");
	this->gui.context->inflater.register_widget<tabbed_book>("tabbed_book");

	this->gui.context->loader.mount_res_pack(*this->get_res_file("res/"));
	
	auto c = this->gui.context->inflater.inflate(
			*this->get_res_file("res/main.gui")
		);


	auto& tb = c->get_widget_as<tabbed_book>("tabbed_book");

	tb.add("test", std::make_shared<editor_page>(this->gui.context, treeml::forest()));
	
	this->gui.set_root(std::move(c));
}
