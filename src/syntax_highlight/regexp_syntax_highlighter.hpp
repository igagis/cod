#pragma once

#include "syntax_highlighter.hpp"

namespace cod{

class regex_syntax_highlighter : public syntax_highlighter{
public:
    std::vector<line_span> highlight(std::u32string_view str)override;
};

}
