#pragma once

#include <utki/span.hpp>

namespace cod{

class plugin_manager{

    void load(std::string_view file_name);
public:
    plugin_manager(utki::span<const std::string> plugins);
};

}
