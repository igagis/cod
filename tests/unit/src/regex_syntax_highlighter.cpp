#include <tst/set.hpp>
#include <tst/check.hpp>

#include <utki/unicode.hpp>

#include <papki/fs_file.hpp>

#include <cod/syntax_highlight/regex_syntax_highlighter.hpp>

namespace{
std::string to_markup(const std::u32string& str, utki::span<cod::line_span> spans){
    std::stringstream ss;

    size_t cur_free_style_num = 0;
    std::map<cod::attributes*, std::string> style_names;

    auto get_style_name = [&](const std::shared_ptr<cod::attributes>& s) -> const std::string&{
        auto i = style_names.find(s.get());
        if(i == style_names.end()){
            std::stringstream ss;
            ss << "(" << cur_free_style_num << ")";
            auto res = style_names.insert(std::make_pair(s.get(), ss.str()));
            return res.first->second;
        }
        return i->second;
    };

    std::u32string_view v(str);
    for(const auto& s : spans){
        tst::check_le(s.length, v.size(), SL);

        ss << get_style_name(s.attrs);
        ss << utki::to_utf8(v.substr(0, s.length));
    }

    return ss.str();
}
}

namespace{
tst::set set("regex_syntax_highlighter", [](tst::suite& suite){
    suite.add(
        "parse",
        [](){
            cod::regex_syntax_highlighter sh(treeml::read(papki::fs_file("../../highlight/xml.3ml")));
        }
    );

    suite.add<std::pair<std::string, std::string>>(
        "correctness",
        {
            {"<", "(0)<"}
        },
        [](const auto& p){
            cod::regex_syntax_highlighter highlighter(treeml::read(papki::fs_file("../../highlight/xml.3ml")));

            auto in = utki::to_utf32(p.first);

            auto res = highlighter.highlight(in);
            tst::check_eq(
                    to_markup(in, res),
                    p.second,
                    SL
                );
        }
    );
});
}
