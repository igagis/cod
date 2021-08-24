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

class regex_syntax_highlighter_model{
public:
    regex_syntax_highlighter_model(const treeml::forest& spec);

    struct state;

    class matcher{
    public:
        const bool is_preprocessed;

        matcher(bool is_preprocessed = false) :
                is_preprocessed(is_preprocessed)
        {}

        virtual ~matcher(){}

        struct match_result{
            size_t begin;
            size_t end;

            std::vector<std::string> capture_groups;

            size_t size()const{
                return end - begin;
            }
        };
        virtual match_result match(std::u32string_view str, bool line_begin)const = 0;

        virtual std::shared_ptr<const matcher> preprocess(utki::span<const std::string> capture_groups)const{
            return nullptr;
        }
    };

    struct rule{
    public:
        std::shared_ptr<const matcher> matcher_;

        struct operation{
            enum class type{
                push,
                pop
            };
            type type_;

            // plain pointer to avoid circular references in case state refers a rule which pushes the same state
            state* state_to_push = nullptr;
        };

        std::vector<operation> operations;

        std::shared_ptr<const font_style> style;

        struct parse_result{
            std::shared_ptr<rule> rule_;
            std::vector<std::tuple<operation::type, std::string>> operations;
            std::string style;
            std::string state_to_push;
        };
        static parse_result parse(const treeml::forest& spec);
    };

    class regex_matcher : public matcher{
        srell::u32regex regex;

    public:
        regex_matcher(std::u32string_view regex_str) :
                regex(regex_str.data(), regex_str.size(), srell::regex_constants::optimize)
        {}
        match_result match(std::u32string_view str, bool line_begin)const override;
    };

    // preprocessed regex matcher
    class ppregex_matcher : public matcher{
        struct regex_part{
            std::u32string str;
            unsigned group_num;
        };
        std::vector<regex_part> regex_parts;
        std::u32string regex_tail;

    public:
        ppregex_matcher(std::string_view regex);

        match_result match(std::u32string_view str, bool line_begin)const override{
            // this method is not supposed to be ever called
            ASSERT(false)
            return {};
        }

        std::shared_ptr<const matcher> preprocess(utki::span<const std::string> capture_groups)const override;
    };

    struct state{
        std::vector<std::shared_ptr<const rule>> rules;
        std::shared_ptr<const font_style> style;

        struct parse_result{
            std::shared_ptr<state> state_;
            std::vector<std::string> rules;
            std::string style;
        };
        static parse_result parse(const treeml::forest& spec);
    };

    // need to keep strong pointers to all states, because rules hold only plain pointer to the state_to_push
    std::vector<std::shared_ptr<const state>> states;
};

class regex_syntax_highlighter : public syntax_highlighter{
    const std::shared_ptr<const regex_syntax_highlighter_model> model;
public:
    regex_syntax_highlighter(
            std::shared_ptr<const regex_syntax_highlighter_model> model
        );

    void reset()override;

    std::vector<line_span> highlight(std::u32string_view str)override;

private:
    struct state_frame{
        std::reference_wrapper<const regex_syntax_highlighter_model::state> state;
        std::vector<std::string> capture_groups;
        std::vector<std::pair<
                const regex_syntax_highlighter_model::matcher*,
                std::shared_ptr<const regex_syntax_highlighter_model::matcher>
            >> preprocessed_rules_cache;
    };
    std::vector<state_frame> state_stack;
};

}
