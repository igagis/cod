#include <tst/set.hpp>
#include <tst/check.hpp>

#include <papki/fs_file.hpp>

#include <cod/syntax_highlight/regex_syntax_highlighter.hpp>

namespace{
tst::set set("regex_syntax_highlighter", [](tst::suite& suite){
    suite.add(
        "parse",
        [](){
            cod::regex_syntax_highlighter sh(treeml::read(papki::fs_file("../../highlight/xml.tml")));
        }
    );
});
}
