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

regex_syntax_highlighter::regex_syntax_highlighter(const treeml::forest& spec){
    parsing_context c;

    for(const auto& n : spec){
        if(n.value == "styles"){
            c.parse_styles(n.children);
        }
    }
}

void regex_syntax_highlighter::reset(){
    // TODO:
}

std::vector<line_span> regex_syntax_highlighter::highlight(std::u32string_view str){
    std::vector<line_span> ret;

    return ret;
}
