#include "editor_page.hpp"

using namespace cod;

editor_page::editor_page(std::shared_ptr<morda::context> context, const treeml::forest& desc) :
		morda::widget(std::move(context), desc),
		morda::page(this->context, desc),
		code_edit(this->context, desc)
{
	this->set_text(
R"qwertyuiop(Hello world!
second line
third line
very very long line lorem ipsum dolor sit amet consecteteur blah blag
ef
ef
qw
ef
wqef
we
fw
ef
we 
fwe 
fwe 
fw e
fwe we
f w
ef 
we
f we
f we
f
we 
fwe

fwqe
fwe

fwqe
	f
	wqe
	f
	wqf


wef wqe)qwertyuiop");
}
