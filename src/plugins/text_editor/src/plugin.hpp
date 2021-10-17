#pragma once

#include <cod/plugin.hpp>

namespace cod{

class text_editor_plugin : public cod::plugin{
public:
    std::shared_ptr<morda::page> open_file(std::string_view file_name)override;
};

}
