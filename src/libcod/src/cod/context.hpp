#pragma once

#include <utki/singleton.hpp>

#include "command_line_args.hpp"
#include "gui.hpp"
#include "file_opener.hpp"
#include "plugin_manager.hpp"
#include "shortcut_resolver.hpp"

namespace cod{

// TODO: make intrusive_singleton
class context : public utki::singleton<context>{
    friend class application;
    
    context(command_line_args&& cla, mordavokne::application& app);
public:
    const std::string base_dir;

    const std::unique_ptr<papki::file> res_file;

    cod::gui gui;

    cod::file_opener file_opener;

	shortcut_resolver shortcuts;

    // this goes as last member to be sure the all the other members are initialized
	// before loading plugins
	plugin_manager plugins;
};

}
