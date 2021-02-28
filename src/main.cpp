#include <mordavokne/application.hpp>

#include <clargs/parser.hpp>

#include "code_edit.hpp"
#include "file_tree.hpp"

struct command_line_arguments{
	std::string base_dir;
};

class application : public mordavokne::application{
public:
	const command_line_arguments cla;

	application(command_line_arguments&& cla) :
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
};

std::unique_ptr<mordavokne::application> mordavokne::create_application(int argc, const char** argv){
	command_line_arguments cla;

	clargs::parser p;

	bool help = false;

	p.add("help", "display help information", [&help](){help = true;});

	auto fa = p.parse(argc, argv);

	if(help){
		std::cout << p.description() << std::endl;
		return nullptr;
	}

	if(fa.size() > 1){
		throw std::invalid_argument("error: more than one directory given");
	}

	if(fa.size() == 1){
		cla.base_dir = std::move(fa.front());
	}

	return std::make_unique<::application>(std::move(cla));
}
