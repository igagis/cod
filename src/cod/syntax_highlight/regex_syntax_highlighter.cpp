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

#include "regex_syntax_highlighter.hpp"

#include <utki/linq.hpp>

#include <treeml/crawler.hpp>

using namespace cod;

namespace{
std::shared_ptr<attributes> parse_style(const treeml::forest& style){
    auto ret = std::make_shared<attributes>();
    for(const auto& n : style){
        if(n.value == "color"){
            ret->color = treeml::crawler(n.children).get().value.to_uint32();
        }else if(n.value == "style"){
            const auto& v = treeml::crawler(n.children).get().value;
            if(v == "normal"){
                ret->style = morda::res::font::style::normal;
            }else if(v == "bold"){
                ret->style = morda::res::font::style::bold;
            }else if(v == "italic"){
                ret->style = morda::res::font::style::italic;
            }else if(v == "bold_italic"){
                ret->style = morda::res::font::style::bold_italic;
            }else{
                std::stringstream ss;
                ss << "unknown font style value: " << v;
                throw std::invalid_argument(ss.str());
            }
        }else if(n.value == "underline"){
            ret->underline = treeml::crawler(n.children).get().value.to_bool();
        }else if(n.value == "stroke"){
            ret->stroke = treeml::crawler(n.children).get().value.to_bool();
        }else{
            std::stringstream ss;
            ss << "unknown font style item: " << n.value;
            throw std::invalid_argument(ss.str());
        }
    }
    return ret;
}
}

void regex_syntax_highlighter::parsing_context::parse_styles(const treeml::forest& styles){
    for(const auto& s : styles){
        if(this->styles.find(s.value.to_string()) != this->styles.end()){
            std::stringstream ss;
            ss << "style with name '" << s.value.to_string() << "' already exists";
            throw std::invalid_argument(ss.str());
        }

        this->styles.insert(std::make_pair(s.value.to_string(), parse_style(s.children)));
    }
}

void regex_syntax_highlighter::parsing_context::parse_rules(const treeml::forest& desc){
    for(const auto& m : desc){
        if(this->rules.find(m.value.to_string()) != this->rules.end()){
            std::stringstream ss;
            ss << "rule with name '" << m.value.to_string() << "' already exists";
            throw std::invalid_argument(ss.str());
        }

        this->rules.insert(std::make_pair(m.value.to_string(), rule::parse(m.children)));
    }
}

void regex_syntax_highlighter::parsing_context::parse_states(const treeml::forest& desc){
    for(const auto& s : desc){
        if(this->states.find(s.value.to_string()) != this->states.end()){
            std::stringstream ss;
            ss << "state with name '" << s.value.to_string() << "' already exists";
            throw std::invalid_argument(ss.str());
        }

        this->states.insert(std::make_pair(s.value.to_string(), state::parse(s.children)));
    }
}

std::shared_ptr<attributes>
regex_syntax_highlighter::parsing_context::get_style(const std::string& name)
{
    auto i = this->styles.find(name);
    if(i == this->styles.end()){
        std::stringstream ss;
        ss << "style '" << name << "' not found";
        throw std::invalid_argument(ss.str());
    }
    ASSERT(i->second)
    return i->second;
}

std::shared_ptr<regex_syntax_highlighter::state>
regex_syntax_highlighter::parsing_context::get_state(const std::string& name)
{
    auto i = this->states.find(name);
    if(i == this->states.end()){
        std::stringstream ss;
        ss << "state not found: " << name;
        throw std::invalid_argument(ss.str());
    }
    ASSERT(i->second.state_)
    return i->second.state_;
}

std::shared_ptr<regex_syntax_highlighter::rule>
regex_syntax_highlighter::parsing_context::get_rule(const std::string& name)
{
    auto i = this->rules.find(name);
    if(i == this->rules.end()){
        std::stringstream ss;
        ss << "rule '" << name << "' not found";
        throw std::invalid_argument(ss.str());
    }
    return i->second.rule_;
}

regex_syntax_highlighter::rule::match_result regex_syntax_highlighter::regex_rule::match(std::u32string_view str){
    srell::match_results<decltype(str)::const_iterator> m;

    if(!srell::regex_search(str.begin(), str.end(), m, this->regex)){
        return match_result{.begin = str.size()};
    }

    ASSERT(!m.empty())
    return match_result{
        .begin = size_t(std::distance(str.cbegin(), m[0].first)),
        .end = size_t(std::distance(str.cbegin(), m[0].second))
    };
}

regex_syntax_highlighter::rule::parse_result regex_syntax_highlighter::rule::parse(const treeml::forest& desc){
    parse_result ret;

    std::u32string regex;
    operation operation_ = operation::nothing;

    for(const auto& n : desc){
        if(n.value == "style"){
            ret.style = treeml::crawler(n.children).get().value.to_string();
        }else if(n.value == "regex"){
            regex = utki::to_utf32(treeml::crawler(n.children).get().value.to_string());
        }else if(n.value == "push"){
            ret.state_to_push = treeml::crawler(n.children).get().value.to_string();
            operation_ = operation::push;
        }else if(n.value == "pop"){
            operation_ = operation::pop;
        }else{
            std::stringstream ss;
            ss << "unknown rule keyword: " << n.value;
            throw std::invalid_argument(ss.str());
        }
    }

    ret.rule_ = std::make_shared<regex_rule>(regex);
    ret.rule_->operation_ = operation_;

    return ret;
}

regex_syntax_highlighter::state::parse_result regex_syntax_highlighter::state::parse(const treeml::forest& desc){
    parse_result ret;

    ret.state_ = std::make_shared<state>();

    for(const auto& n : desc){
        if(n.value == "style"){
            ret.style = treeml::crawler(n.children).get().value.to_string();
        }else if(n.value == "rules"){
            ret.rules = utki::linq(n.children).select([](const auto& c){return c.value.to_string();}).get();
        }else{
            std::stringstream ss;
            ss << "unknown state keyword: " << n.value;
            throw std::invalid_argument(ss.str());
        }
    }

    return ret;
}

regex_syntax_highlighter::regex_syntax_highlighter(const treeml::forest& spec){
    parsing_context c;

    for(const auto& n : spec){
        if(n.value == "styles"){
            c.parse_styles(n.children);
        }else if(n.value == "rules"){
            c.parse_rules(n.children);
        }else if(n.value == "states"){
            c.parse_states(n.children);
        }else{
            std::stringstream ss;
            ss << "unknown keyword: " << n.value;
            throw std::invalid_argument(ss.str());
        }
    }

    // set state -> rules and state -> styles references
    for(const auto& n : c.states){
        ASSERT(n.second.state_)
        auto& state_ = *n.second.state_;
        const auto& parsed = n.second;
        state_.rules = utki::linq(parsed.rules).select([&](const auto& m){
            return c.get_rule(m);
        }).get();

        state_.style = c.get_style(parsed.style);
    }

    // set rule -> style and rule -> state references
    for(const auto& n : c.rules){
        ASSERT(n.second.rule_)

        auto& rule_ = *n.second.rule_;
        const auto& parsed = n.second;

        // rule can have no style, then it inherits style from pushed state
        if(!parsed.style.empty()){
            rule_.style = c.get_style(parsed.style);
        }

        if(rule_.operation_ == rule::operation::push){
            rule_.state_to_push = c.get_state(parsed.state_to_push).get();
        }
    }

    this->initial_state = c.get_state("initial");
    this->reset();
}

void regex_syntax_highlighter::reset(){
    this->state_stack.clear();
    ASSERT(this->initial_state);
    this->state_stack.push_back(*this->initial_state);
}

std::vector<line_span> regex_syntax_highlighter::highlight(std::u32string_view str){
    std::vector<line_span> ret;


    // TODO:


    return ret;
}
