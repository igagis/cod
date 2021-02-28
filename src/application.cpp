#include "application.hpp"

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

	this->gui.context->loader.mount_res_pack(*this->get_res_file("res/"));
	
	auto c = this->gui.context->inflater.inflate(
			*this->get_res_file("res/main.gui")
		);

	c->get_widget_as<morda::text_widget>("code_edit").set_text(
R"qwertyuiop(Hello world!
second line
third line
very very long line lorem ipsum dolor sit amet consecteteur blah blag
ef
ef
qw
ef
wqef
we
fw
ef
we 
fwe 
fwe 
fw e
fwe we
f w
ef 
we
f we
f we
f
we 
fwe

fwqe
fwe

fwqe
	f
	wqe
	f
	wqf


wef wqe)qwertyuiop");
	
	this->gui.set_root(std::move(c));
}
