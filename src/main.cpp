#include <clargs/parser.hpp>

#include "application.hpp"

mordavokne::application_factory app_fac([](auto args) -> std::unique_ptr<mordavokne::application>{
	cod::command_line_arguments cla;

	clargs::parser p;

	bool help = false;

	p.add("help", "display help information", [&help](){help = true;});

	auto fa = p.parse(args);

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
});
