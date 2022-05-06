#pragma once

#include <mordavokne/application.hpp>

#include <morda/widgets/group/tabbed_book.hpp>

#include "tiling_area.hpp"
#include "editor_page.hpp"

namespace cod{

class context;

class gui{
    friend class context;
    friend class file_opener;

    std::shared_ptr<morda::tabbed_book> editors_tabbed_book;

    gui(mordavokne::application& app);

    void open_editor(std::shared_ptr<editor_page> page);
public:
    const std::shared_ptr<morda::context> morda_context;
};

}
