#include "text_editor_page.hpp"

#include "synhi/regex_highlighter.hpp"

#include <papki/fs_file.hpp>

using namespace cod;

text_editor_page::text_editor_page(
        std::shared_ptr<morda::context> context,
        std::string&& file_name
    ) :
		morda::widget(std::move(context), treeml::forest()),
		editor_page(this->context, std::move(file_name)),
		code_edit(this->context, treeml::forest())
{
	// TODO: for now we set XML syntax highlighter for each code edit page,
	//       later need to implement proper system
	this->text_change_handler = [
			this,
			hl = std::make_shared<synhi::regex_highlighter>(
					std::make_shared<synhi::regex_highlighter_model>(
							treeml::read(papki::fs_file("highlight/xml.tml"))
						)
				)
		]
	(morda::text_widget& w)
	{
		hl->reset();
		const auto& lines = this->get_lines();
		for(auto i = lines.begin(); i != lines.end(); ++i){
			this->set_line_spans(
				hl->highlight(i->str),
				i
			);
		}
	};
}
