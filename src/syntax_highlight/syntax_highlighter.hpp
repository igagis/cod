#pragma once

#include <morda/res/font.hpp>

namespace cod{

struct attributes{
    morda::res::font::style style = morda::res::font::style::normal;
    bool underline = false;
    bool stroke = false;
    uint32_t color = 0xffffffff;
};

struct line_span{
    size_t length = 0;
    std::shared_ptr<attributes> attrs;
};

class syntax_highlighter{
public:
    virtual std::vector<line_span> highlight(std::u32string_view str) = 0;

    virtual ~syntax_highlighter(){}
};

}
