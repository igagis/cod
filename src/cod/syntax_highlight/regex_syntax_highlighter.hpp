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

#include <srell.hpp>

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
        srell::u32regex regex;

        matcher(std::u32string_view regex_str) :
                regex(regex_str.data(), regex_str.size(), srell::regex_constants::optimize)
        {}

        enum class operation{
            nothing,
            push,
            pop
        };

        operation operation_ = operation::nothing;

        // plain pointer to avoid circular references
        state* state_to_push = nullptr;

        std::shared_ptr<attributes> style;

        struct parse_result{
            std::shared_ptr<matcher> matcher_;
            std::string style;
            std::string state_to_push;
        };
        static parse_result parse(const treeml::forest& spec);
    };

    struct state{
        std::vector<std::shared_ptr<matcher>> matchers;
        std::shared_ptr<attributes> style;

        struct parse_result{
            std::shared_ptr<state> state_;
            std::vector<std::string> matchers;
            std::string style;
        };
        static parse_result parse(const treeml::forest& spec);
    };

    // this struct is only used when parsing highlighter rules
    struct parsing_context{
        std::map<std::string, std::shared_ptr<attributes>> styles;
        std::map<std::string, matcher::parse_result> matchers;
        std::map<std::string, state::parse_result> states;

        void parse_styles(const treeml::forest& styles);
        void parse_matchers(const treeml::forest& desc);
        void parse_states(const treeml::forest& desc);

        std::shared_ptr<attributes> get_style(const std::string& name);
        std::shared_ptr<state> get_state(const std::string& name);
        std::shared_ptr<matcher> get_matcher(const std::string& name);
    };

    // highlighter state
    std::shared_ptr<state> initial_state;
    std::vector<std::reference_wrapper<state>> state_stack;
};

}
