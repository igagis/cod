#pragma once

#include "command_line_args.hpp"

#include "gui.hpp"
#include "file_opener.hpp"

#include <cod/plugin_manager.hpp>

namespace cod{

class context{
    friend class application;
    
    context(command_line_args&& cla);
public:
    std::string base_dir;

    cod::gui gui;
    cod::file_opener file_opener;

    // this goes as last member to be sure the all the other members are initialized
	// before loading plugins
	plugin_manager plugins;
};

}
