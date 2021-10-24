#pragma once

#include "tiling_area.hpp"

namespace cod{

class gui{
    friend class context;

    std::shared_ptr<tiling_area> editors_tiling_area;

    gui();
public:

    // TODO: remove this method
    std::shared_ptr<tiling_area> get_tiling_area(){
        return this->editors_tiling_area;
    }

    // void open_editor(std::shared_ptr<morda::page> page, )
};

}
