#pragma once

#include <mordavokne/application.hpp>

#include "tiling_area.hpp"
#include "editor_page.hpp"

namespace cod{

class context;

class gui{
    friend class context;

    std::shared_ptr<tiling_area> editors_tiling_area;

    gui(mordavokne::application& app);
public:
    const std::shared_ptr<morda::context> morda_context;

    // TODO: remove this method
    std::shared_ptr<tiling_area> get_tiling_area(){
        return this->editors_tiling_area;
    }

    void open_editor(std::shared_ptr<editor_page> page);
};

}
