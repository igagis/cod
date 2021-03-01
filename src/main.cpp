#include <clargs/parser.hpp>

#include "application.hpp"

std::unique_ptr<mordavokne::application> mordavokne::create_application(int argc, const char** argv){
	cod::command_line_arguments cla;

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
	}else{
		cla.base_dir = "./";
	}

	return std::make_unique<cod::application>(std::move(cla));
}
