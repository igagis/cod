#include "context.hpp"

using namespace cod;

context::context(command_line_args&& cla, mordavokne::application& app) :
        base_dir(std::move(cla.base_dir)),
        res_file(app.get_res_file()),
        gui(app),
        plugins(cla.plugins)
{

}
