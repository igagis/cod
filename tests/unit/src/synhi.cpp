#include <tst/set.hpp>
#include <tst/check.hpp>

#include <utki/unicode.hpp>
#include <utki/string.hpp>
#include <utki/linq.hpp>

#include <papki/fs_file.hpp>

#include <cod/synhi/regex_highlighter.hpp>

using namespace std::string_literals;

namespace{
struct line{
    std::u32string str;
    std::vector<synhi::line_span> spans;
};
}

namespace{
std::string to_markup(utki::span<const line> lines){
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

    for(auto l = lines.begin(); l != lines.end(); ++l){
        if(l != lines.begin()){
            ss << "\n";
        }
        
        std::u32string_view v(l->str);
        for(const auto& s : l->spans){
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
            {"<tag><tag1 /></tag>", "(0)<(1)tag(0)><(1)tag1(2) (0)/></(1)tag(0)>"},
            {"<tag><!--<tag1 />--></tag>", "(0)<(1)tag(0)>(2)<!--<tag1 />-->(0)</(1)tag(0)>"},
            {"<tag attr=\"val\"/>", "(0)<(1)tag(2) (3)attr(4)=(5)\"(6)val(5)\"(0)/>"},
            {"<tag attr='val'/>", "(0)<(1)tag(2) (3)attr(4)=(5)'(6)val(5)'(0)/>"},
            {
   R"qwertyuiop(<tag>
                    <tag1
                        attr1="qwe" />
                </tag>)qwertyuiop",
   R"qwertyuiop((0)<(1)tag(0)>
(2)                    (0)<(1)tag1
(2)                        (3)attr1(4)=(5)"(6)qwe(5)"(2) (0)/>
(2)                (0)</(1)tag(0)>)qwertyuiop"}
        },
        [model = std::make_shared<synhi::regex_highlighter_model>(
                treeml::read(papki::fs_file("../../highlight/xml.3ml"))
            )]
        (const auto& p)
        {
            synhi::regex_highlighter highlighter(model);

            auto in = utki::to_utf32(p.first);

            auto lines = utki::linq(utki::split(in, U'\n')).select([&](const auto& p){
                return line{
                    str: p,
                    spans: highlighter.highlight(p)
                };
            }).get();

            tst::check_eq(
                    to_markup(lines),
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
