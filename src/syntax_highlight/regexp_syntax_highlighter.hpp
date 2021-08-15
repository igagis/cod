#pragma once

#include "syntax_highlighter.hpp"

#include <treeml/tree.hpp>

namespace cod{

class regex_syntax_highlighter : public syntax_highlighter{
public:
    regex_syntax_highlighter(const treeml::forest& spec);

    void reset()override;

    std::vector<line_span> highlight(std::u32string_view str)override;
};

}
