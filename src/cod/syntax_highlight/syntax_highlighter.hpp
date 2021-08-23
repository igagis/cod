/*
cod - text editor

Copyright (C) 2021  Ivan Gagis <igagis@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/* ================ LICENSE END ================ */

#pragma once

#include <morda/res/font.hpp>

namespace cod{

struct font_style{
    morda::res::font::style style = morda::res::font::style::normal;
    bool underline = false;
    bool stroke = false;
    uint32_t color = 0xffffffff;
};

struct line_span{
    size_t length = 0;
    std::shared_ptr<const font_style> attrs;
};

class syntax_highlighter{
public:
    virtual std::vector<line_span> highlight(std::u32string_view str) = 0;

    virtual void reset() = 0;

    virtual ~syntax_highlighter(){}
};

}
