#include <mordavokne/application.hpp>
#include "code_edit.hpp"

class Application : public mordavokne::application{
public:
	Application() :
			application(
					"cod",
					[](){
						return mordavokne::window_params(r4::vector2<unsigned>(320, 480));
					}()
				)
	{
		this->gui.initStandardWidgets(*this->get_res_file());
		
		this->gui.context->inflater.register_widget<code_edit>("code_edit");

		this->gui.context->loader.mount_res_pack(*this->get_res_file("res/"));
		
		auto c = this->gui.context->inflater.inflate(
				*this->get_res_file("res/main.gui")
			);

		c->get_widget_as<morda::text_widget>("code_edit").set_text(
R"qwertyuiop(Hello world!
second line
\tthird line
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
};

std::unique_ptr<mordavokne::application> mordavokne::create_application(int argc, const char** argv){
	return std::make_unique<::Application>();
}
