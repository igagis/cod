#pragma once

#include <mordavokne/application.hpp>

#include "tiling_area.hpp"
#include "tabbed_book_tile.hpp"
#include "file_page.hpp"

namespace cod{

class context;

class gui{
    friend class context;
    friend class file_opener;

    std::shared_ptr<tabbed_book_tile> editors_tabbed_book;

    gui(mordavokne::application& app);

    void open_editor(std::shared_ptr<file_page> page);
public:
    const std::shared_ptr<morda::context> morda_context;
};

}
