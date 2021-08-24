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
#include <utki/string.hpp>

#include <treeml/crawler.hpp>

using namespace cod;

namespace{
std::shared_ptr<font_style> parse_style(const treeml::forest& style){
    auto ret = std::make_shared<font_style>();
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

namespace{
struct parsing_context{
    std::map<std::string, std::shared_ptr<font_style>> styles;
    std::map<std::string, regex_syntax_highlighter_model::rule::parse_result> rules;

    // needs to preserve order
    std::vector<std::pair<std::string, regex_syntax_highlighter_model::state::parse_result>> states;

    std::string initial_state;

    void parse_styles(const treeml::forest& styles){
        for(const auto& s : styles){
            if(this->styles.find(s.value.to_string()) != this->styles.end()){
                std::stringstream ss;
                ss << "style with name '" << s.value.to_string() << "' already exists";
                throw std::invalid_argument(ss.str());
            }

            this->styles.insert(std::make_pair(s.value.to_string(), parse_style(s.children)));
        }
    }

    void parse_rules(const treeml::forest& desc){
        for(const auto& m : desc){
            if(this->rules.find(m.value.to_string()) != this->rules.end()){
                std::stringstream ss;
                ss << "rule with name '" << m.value.to_string() << "' already exists";
                throw std::invalid_argument(ss.str());
            }

            this->rules.insert(std::make_pair(
                    m.value.to_string(),
                    regex_syntax_highlighter_model::rule::parse(m.children)
                ));
        }
    }

    void parse_states(const treeml::forest& desc){
        if(!desc.empty()){
            this->initial_state = desc.front().value.to_string();
        }
        for(const auto& s : desc){
            if(std::find_if(
                    this->states.begin(),
                    this->states.end(),
                    [&](const auto& v){return v.first == s.value.to_string();}
                ) != this->states.end())
            {
                std::stringstream ss;
                ss << "state with name '" << s.value.to_string() << "' already exists";
                throw std::invalid_argument(ss.str());
            }

            this->states.push_back(std::make_pair(
                    s.value.to_string(),
                    regex_syntax_highlighter_model::state::parse(s.children)
                ));
        }
    }

    std::shared_ptr<font_style> get_style(const std::string& name){
        auto i = this->styles.find(name);
        if(i == this->styles.end()){
            std::stringstream ss;
            ss << "style '" << name << "' not found";
            throw std::invalid_argument(ss.str());
        }
        ASSERT(i->second)
        return i->second;
    }

    std::shared_ptr<regex_syntax_highlighter_model::state> get_state(const std::string& name){
        auto i = std::find_if(this->states.begin(), this->states.end(), [&](const auto& v){return v.first == name;});
        if(i == this->states.end()){
            std::stringstream ss;
            ss << "state not found: " << name;
            throw std::invalid_argument(ss.str());
        }
        ASSERT(i->second.state_)
        return i->second.state_;
    }

    std::shared_ptr<regex_syntax_highlighter_model::rule> get_rule(const std::string& name){
        auto i = this->rules.find(name);
        if(i == this->rules.end()){
            std::stringstream ss;
            ss << "rule '" << name << "' not found";
            throw std::invalid_argument(ss.str());
        }
        return i->second.rule_;
    }
};
}

regex_syntax_highlighter_model::matcher::match_result
regex_syntax_highlighter_model::regex_matcher::match(
        std::u32string_view str,
        bool line_begin
    )const
{
    srell::match_results<decltype(str)::const_iterator> m;

    auto regex_flags = srell::regex_constants::match_default;
    if(!line_begin){
        regex_flags |= srell::regex_constants::match_not_bol;
    }

    if(!srell::regex_search(str.begin(), str.end(), m, this->regex, regex_flags)){
        return match_result{
            .begin = str.size(),
            .end = 0
        };
    }

    ASSERT(!m.empty())

    std::vector<std::u32string> capture_groups;

    ASSERT(m.size() >= 1)
    for(size_t i = 1; i != m.size(); ++i){
        if(!m[i].matched){
            capture_groups.push_back(std::u32string());
        }else{
            capture_groups.push_back(std::u32string(m[i].first, m[i].second));
        }
    }

    return match_result{
        .begin = size_t(std::distance(str.cbegin(), m[0].first)),
        .end = size_t(std::distance(str.cbegin(), m[0].second)),
        .capture_groups = std::move(capture_groups)
    };
}

regex_syntax_highlighter_model::rule::parse_result regex_syntax_highlighter_model::rule::parse(const treeml::forest& desc){
    parse_result ret;
    ret.rule_ = std::make_shared<rule>();

    for(const auto& n : desc){
        if(n.value == "style"){
            ret.style = treeml::crawler(n.children).get().value.to_string();
        }else if(n.value == "regex"){
            ret.rule_->matcher_ = std::make_shared<regex_syntax_highlighter_model::regex_matcher>(
                    utki::to_utf32(treeml::crawler(n.children).get().value.to_string())
                );
        }else if(n.value == "ppregex"){
            ret.rule_->matcher_ = std::make_shared<regex_syntax_highlighter_model::ppregex_matcher>(
                    treeml::crawler(n.children).get().value.to_string()
                );
        }else if(n.value == "push"){
            ret.operations.push_back({
                operation::type::push,
                treeml::crawler(n.children).get().value.to_string()
            });
        }else if(n.value == "pop"){
            ret.operations.push_back({
                operation::type::pop,
                std::string()
            });
        }else{
            std::stringstream ss;
            ss << "unknown rule keyword: " << n.value;
            throw std::invalid_argument(ss.str());
        }
    }

    ASSERT(ret.rule_->matcher_, [&](auto&o){o << treeml::to_string(desc);})

    return ret;
}

regex_syntax_highlighter_model::state::parse_result regex_syntax_highlighter_model::state::parse(const treeml::forest& desc){
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

regex_syntax_highlighter_model::regex_syntax_highlighter_model(const treeml::forest& spec){
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
        this->states.push_back(n.second.state_);
        auto& state_ = *n.second.state_;
        const auto& parsed = n.second;
        state_.rules = utki::linq(parsed.rules).select([&](const auto& m) -> std::shared_ptr<const rule>{
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

        for(const auto& o : parsed.operations){
            auto op = std::get<rule::operation::type>(o);
            rule_.operations.push_back({
                op,
                op == rule::operation::type::push ? c.get_state(std::get<std::string>(o)).get() : nullptr
            });
        }
    }
}

regex_syntax_highlighter::regex_syntax_highlighter(
        std::shared_ptr<const regex_syntax_highlighter_model> model
    ) :
        model(std::move(model))
{
    ASSERT(this->model)
    this->reset();
}

void regex_syntax_highlighter::reset(){
    this->state_stack.clear();
    ASSERT(this->model)
    ASSERT(!this->model->states.empty());
    this->state_stack.push_back(
            state_frame{
                .state = *this->model->states.front()
            }
        );
}

std::vector<line_span> regex_syntax_highlighter::highlight(std::u32string_view str){
    std::vector<line_span> ret;

    // start with one span of length 0
    ret.push_back(line_span{
        .length = 0,
        .attrs = this->state_stack.back().state.get().style
    });

    bool line_begin = true;
    std::u32string_view view(str);

    while(!view.empty()){
        regex_syntax_highlighter_model::matcher::match_result match{
            .begin = view.size(),
            .end = 0
        };
        const regex_syntax_highlighter_model::rule* match_rule = nullptr;

        // go through all rules of the current state to find the match closest to current
        // position in the text line
        ASSERT(!this->state_stack.empty())
        for(const auto& r : this->state_stack.back().state.get().rules){
            auto matcher = r->matcher_.get();
            ASSERT(matcher)
            if(matcher->is_preprocessed){
                auto& cache = this->state_stack.back().preprocessed_rules_cache;
                auto i = std::find_if(cache.begin(), cache.end(), [&](const auto& e){return e.first == matcher;});
                if(i == cache.end()){
                    cache.push_back(std::make_pair(
                            matcher,
                            matcher->preprocess(this->state_stack.back().capture_groups)
                        ));
                    matcher = cache.back().second.get();
                }else{
                    matcher = i->second.get();
                }
            }

            auto m = matcher->match(view, line_begin);

            if(m.begin < match.begin){
                match = m;
                match_rule = r.get();
            }

            if(match.begin == 0){
                // there can be no other match closer than 0 chars away, so exit early
                break;
            }
        }

        if(!match_rule){
            // no rule has matched, extend current span to the end of the line an exit
            ASSERT(!ret.empty())
            ret.back().length += view.size();
            break;
        }

        ASSERT(match_rule)

        if(match.begin != 0){
            // extend current span and move the current position to the beginning of the match
            ret.back().length += match.begin;
            view = view.substr(match.begin);
        }

        for(const auto& op : match_rule->operations){
            switch(op.type_){
                case regex_syntax_highlighter_model::rule::operation::type::push:
                    ASSERT(op.state_to_push)
                    this->state_stack.push_back(
                            state_frame{
                                .state = *op.state_to_push,
                                .capture_groups = std::move(match.capture_groups)
                            }
                        );
                    break;
                case regex_syntax_highlighter_model::rule::operation::type::pop:
                    // we must not pop the initial state, so check that more than one state is currently in the stack
                    if(this->state_stack.size() > 1){
                        this->state_stack.pop_back();
                    }
                    break;
                default:
                    ASSERT(false, [&](auto&o){o << "opeartion = " << unsigned(op.type_);})
                    break;
            }
        }

        ASSERT(!ret.empty())
        if(ret.back().length == 0){
            ret.pop_back();
        }

        auto size = match.size();
        view = view.substr(size);

        std::shared_ptr<const font_style> style;
        if(match_rule->style){
            style = match_rule->style;
        }else{
            style = this->state_stack.back().state.get().style;
        }

        if(ret.back().attrs != style){
            ret.push_back(line_span{
                .length = size,
                .attrs = style
            });
        }else{
            ret.back().length += size;
        }

        if(!view.empty()){
            if(ret.back().attrs != this->state_stack.back().state.get().style){
                ret.push_back(line_span{
                    .length = 0,
                    .attrs = this->state_stack.back().state.get().style
                });
            }
        }

        line_begin = false;
    }

    return ret;
}

regex_syntax_highlighter_model::ppregex_matcher::ppregex_matcher(std::string_view regex_str) :
        matcher(true) // true = preprocessed
{
    // prepare preprocessed regex model

    utki::string_parser p(regex_str);

    std::string str;
    while(true){
        auto w = p.read_chars_until('$');

        str.append(w);

        if(p.empty()){
            this->regex_tail = utki::to_utf32(str);
            break;
        }

        ASSERT(!p.empty())
        ASSERT(p.peek_char() == '$')
        p.read_char(); // skip '$'

        switch(p.peek_char()){
            case '$':
                // escaped dollar sign
                p.read_char(); // skip second '$'
                str.append("$");
                continue;
            case '{':
                // capture group reference
                p.read_char(); // skip '{'
                {
                    auto num = p.read_number<unsigned>();
                    if(p.empty() || p.read_char() != '}'){
                        throw std::invalid_argument("preprocessed regex capture group reference syntax error: missing closing '}'");
                    }
                    this->regex_parts.push_back(regex_part{
                        .str = utki::to_utf32(str),
                        .group_num = num
                    });
                    str.clear();
                }
                break;
            default:
                // append skipped '$'
                str.append("$");
                continue;
        }
    }
}

std::shared_ptr<const regex_syntax_highlighter_model::matcher>
regex_syntax_highlighter_model::ppregex_matcher::preprocess(utki::span<const std::u32string> capture_groups)const
{
    std::u32string regex_str;
    for(const auto& p : this->regex_parts){
        if(p.group_num >= capture_groups.size()){
            throw std::logic_error("preprocessed matcher references absent capture group");
        }
        regex_str.append(p.str);
        regex_str.append(capture_groups[p.group_num]);
    }

    regex_str.append(this->regex_tail);

    return std::make_shared<regex_matcher>(regex_str);
}
