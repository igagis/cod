#include "context.hpp"

using namespace cod;

context::context(command_line_args&& cla) :
        base_dir(std::move(cla.base_dir)),
        file_opener(this->gui.get_tiling_area()),
        plugins(cla.plugins)
{

}
