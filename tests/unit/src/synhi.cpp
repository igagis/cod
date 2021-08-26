#include <tst/set.hpp>
#include <tst/check.hpp>

#include <utki/unicode.hpp>
#include <utki/string.hpp>

#include <papki/fs_file.hpp>

#include <cod/synhi/regex_highlighter.hpp>

using namespace std::string_literals;

namespace{
std::string to_markup(const std::u32string& str, utki::span<synhi::line_span> spans){
    std::stringstream ss;

    size_t cur_free_style_num = 0;
    std::map<const synhi::font_style*, std::string> style_names;

    auto get_style_name = [&](const std::shared_ptr<const synhi::font_style>& s) -> const std::string&{
        auto i = style_names.find(s.get());
        if(i == style_names.end()){
            std::stringstream ss;
            ss << "(" << cur_free_style_num << ")";
            auto res = style_names.insert(std::make_pair(s.get(), ss.str()));
            ++cur_free_style_num;
            return res.first->second;
        }
        return i->second;
    };

    auto lines = utki::split(std::u32string_view(str), U'\n');
    for(auto l = lines.begin(); l != lines.end(); ++l){
        if(l != lines.begin()){
            ss << "\n";
        }
        
        std::u32string_view v(*l);
        for(const auto& s : spans){
            tst::check_le(s.length, v.size(), SL);

            ss << get_style_name(s.style);
            ss << utki::to_utf8(v.substr(0, s.length));
            v = v.substr(s.length);
        }
    }

    return ss.str();
}
}

namespace{
tst::set set("regex_highlighter", [](tst::suite& suite){
    suite.add(
        "parse",
        [](){
            synhi::regex_highlighter sh(
                    std::make_shared<synhi::regex_highlighter_model>(
                            treeml::read(papki::fs_file("../../highlight/xml.3ml"))
                        )
                );
        }
    );

    suite.add<std::pair<std::string, std::string>>(
        "xml",
        {
            {"<", "(0)<"},
            {"<tag>bla bla</tag>", "(0)<(1)tag(0)>(2)bla bla(0)</(1)tag(0)>"},
            {"<tag/>", "(0)<(1)tag(0)/>"},
            {"<tag><tag1 /></tag>", "(0)<(1)tag(0)><(1)tag1(0) /></(1)tag(0)>"}
        },
        [model = std::make_shared<synhi::regex_highlighter_model>(
                treeml::read(papki::fs_file("../../highlight/xml.3ml"))
            )]
        (const auto& p)
        {
            synhi::regex_highlighter highlighter(model);

            auto in = utki::to_utf32(p.first);

            auto res = highlighter.highlight(in);

            tst::check_eq(
                    to_markup(in, res),
                    p.second,
                    SL
                );
        }
    );

    suite.add(
        "crash_1",
        [](){
            auto dir = "data/synhi/crash_1/"s;
            synhi::regex_highlighter highlighter(
                    std::make_shared<synhi::regex_highlighter_model>(
                            treeml::read(papki::fs_file(dir + "xml.3ml"))
                        )
                );

            auto text = utki::to_utf32(utki::make_string(papki::fs_file(dir + "LICENSE_GPL3").load()));

            auto lines = utki::split(
                    std::u32string_view(text),
                    U'\n'
                );
            
            for(auto i = lines.begin(); i != lines.end(); ++i){
                highlighter.highlight(*i);
            }
        }
    );
});
}
