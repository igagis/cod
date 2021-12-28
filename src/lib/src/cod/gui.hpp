#pragma once

#include <mordavokne/application.hpp>

#include "tiling_area.hpp"
#include "editor_page.hpp"

namespace cod{

class context;

class gui{
    friend class context;
    friend class file_opener;

    std::shared_ptr<tiling_area> editors_tiling_area;

    gui(mordavokne::application& app);

    void open_editor(std::shared_ptr<editor_page> page);
public:
    const std::shared_ptr<morda::context> morda_context;
};

}
