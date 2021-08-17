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

#include "syntax_highlighter.hpp"

#include <treeml/tree.hpp>

namespace cod{

class regex_syntax_highlighter : public syntax_highlighter{
public:
    regex_syntax_highlighter(const treeml::forest& spec);

    void reset()override;

    std::vector<line_span> highlight(std::u32string_view str)override;

private:
    struct state;

    struct matcher{
        enum class operation{
            nothing,
            push,
            pop
        };

        operation op;

        state* state_to_push = nullptr;

        std::shared_ptr<attributes> style;

        struct parse_result{
            std::shared_ptr<matcher> m;
            std::string style;
            std::string state_to_push;
        };
        static parse_result parse(const treeml::forest& spec);
    };

    struct state{
        std::vector<std::shared_ptr<matcher>> matchers;
        std::shared_ptr<attributes> style;

        struct parse_result{
            std::vector<std::string> matchers;
            std::string style;
        };
        static parse_result parse(const treeml::forest& spec);
    };

    struct parsing_context{
        std::map<std::string, std::shared_ptr<attributes>> styles;
        std::map<std::string, matcher::parse_result> matchers;

        void parse_styles(const treeml::forest& styles);
    };
};

}
