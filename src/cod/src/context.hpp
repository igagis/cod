#pragma once

#include "gui.hpp"
#include "file_opener.hpp"

#include <cod/plugin_manager.hpp>

namespace cod{

class context{
    friend class application;
    
    context();
public:
    cod::gui gui;
    cod::file_opener file_opener;
};

}
