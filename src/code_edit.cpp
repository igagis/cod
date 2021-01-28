#include "code_edit.hpp"

#include <algorithm>

#include <utki/string.hpp>
#include <utki/linq.hpp>

#include <morda/widgets/label/text.hpp>

code_edit::code_edit(std::shared_ptr<morda::context> c, const puu::forest& desc) :
		widget(std::move(c), desc),
		character_input_widget(this->context),
		text_widget(this->context, desc),
		pile(this->context, puu::forest()),
		lines_provider(std::make_shared<provider>(*this))
{
	this->set_font(this->context->loader.load<morda::res::font>("fnt_monospace"));
	this->on_font_change();

	this->push_back_inflate(puu::read(R"qwertyuiop(
			@list{
				id{lines}
				layout{dx{fill} dy{fill}}
			}
		)qwertyuiop"));

	this->get_widget_as<morda::list_widget>("lines").set_provider(this->lines_provider);
}

void code_edit::on_character_input(const std::u32string& unicode, morda::key key){
	// TODO:
}

void code_edit::set_text(std::u32string&& text){
	this->lines = utki::linq(utki::split(text, U'\n')).select([](auto&& s){
			size_t front_size = s.size() / 2;
			decltype(line::spans) spans = {{
				std::make_pair(front_size, attributes{color: 0xff00ff00}),
				std::make_pair(s.size() - front_size, attributes{style: morda::res::font::style::italic, color: 0xff0000ff})
			}};
			return line{
					str: std::move(s),
					spans: std::move(spans)
				};
		}).get();
}

std::u32string code_edit::get_text()const{
	std::u32string ret;

	for(auto& l : this->lines){
		ret.append(l.str);
	}

	return ret;
}

std::shared_ptr<morda::widget> code_edit::provider::get_widget(size_t index){
	auto w = std::make_shared<code_edit::line_widget>(this->owner.context, this->owner, index);
	return w;
}

void code_edit::line_widget::render(const morda::matrix4& matrix)const{	
	using std::round;

	const auto& str = this->owner.lines[this->line_num].str;

	unsigned cur_char_pos = 0;
	for(const auto& s : this->owner.lines[this->line_num].spans){
		const auto& font = this->owner.get_font().get(s.second.style);

		morda::matrix4 matr(matrix);
		matr.translate(
				cur_char_pos * this->owner.font_info.advance,
				this->owner.font_info.baseline
			);
		font.render(
				matr,
				morda::color_to_vec4f(s.second.color),
				std::u32string_view(str.c_str() + cur_char_pos, s.first)
			);
		cur_char_pos += s.first;
	}
}

morda::vector2 code_edit::line_widget::measure(const morda::vector2& quotum)const noexcept{
	const auto& font = this->owner.get_font().get();
	morda::vector2 ret(this->owner.font_info.advance, font.get_height());

	for(unsigned i = 0; i != ret.size(); ++i){
		if(quotum[i] >= 0){
			ret[i] = quotum[i];
		}
	}

	return ret;
}
