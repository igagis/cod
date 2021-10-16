#include "plugin_manager.hpp"

using namespace cod;

namespace{
const unsigned soname =
#include "../../libcodplugin/soname.txt"
    ;
}

plugin_manager::plugin_manager(utki::span<const std::string> plugins){
    // TODO:
}
