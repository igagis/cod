#include "code_edit.hpp"

#include <algorithm>

#include <utki/string.hpp>
#include <utki/linq.hpp>

#include <morda/widgets/label/text.hpp>

namespace{
uint16_t cursor_blink_period_ms = 500;
morda::real cursor_thickness_dp = 1.0f;
}

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
			unsigned front_size = s.size() / 2;
			decltype(line::spans) spans = {{
				line_span{length: front_size, attrs: attributes{color: 0xff00ff00}},
				line_span{length: unsigned(s.size()) - front_size, attrs: attributes{style: morda::res::font::style::italic, color: 0xff0000ff}}
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
		const auto& font = this->owner.get_font().get(s.attrs.style);

		morda::matrix4 matr(matrix);
		matr.translate(
				cur_char_pos * this->owner.font_info.glyph_dims.x(),
				this->owner.font_info.baseline
			);
		font.render(
				matr,
				morda::color_to_vec4f(s.attrs.color),
				std::u32string_view(str.c_str() + cur_char_pos, s.length)
			);
		cur_char_pos += s.length;
	}
}

morda::vector2 code_edit::line_widget::measure(const morda::vector2& quotum)const noexcept{
	morda::vector2 ret = this->owner.font_info.glyph_dims;
	ret.x() *= this->owner.lines[this->line_num].str.size();

	for(unsigned i = 0; i != ret.size(); ++i){
		if(quotum[i] >= 0){
			ret[i] = quotum[i];
		}
	}

	return ret;
}

void code_edit::update(uint32_t dt){
	this->cursor_blink_visible = !this->cursor_blink_visible;
}

void code_edit::on_focus_change(){
	if(this->is_focused()){
		this->start_cursor_blinking();
	}else{
		this->context->updater->stop(*this);
	}
}

void code_edit::start_cursor_blinking(){
	this->context->updater->stop(*this);
	this->cursor_blink_visible = true;
	this->context->updater->start(
			utki::make_shared_from(*static_cast<updateable*>(this)),
			cursor_blink_period_ms
		);
}

void code_edit::render(const morda::matrix4& matrix)const{
	this->pile::render(matrix);

	this->render_cursor(matrix);
}

void code_edit::render_cursor(const morda::matrix4& matrix)const{
	if(!this->is_focused()) return;
	if(!this->cursor_blink_visible) return;

	morda::matrix4 matr(matrix);

	morda::vector2 pos = this->cursor_pos.to<morda::real>().comp_mul(this->font_info.glyph_dims);
	matr.translate(pos);
	matr.scale(morda::vector2(cursor_thickness_dp * this->context->units.dots_per_dp, this->font_info.glyph_dims.y()));

	auto& r = *this->context->renderer;
	r.shader->color_pos->render(matr, *r.pos_quad_01_vao, 0xffffffff);
}

bool code_edit::on_mouse_button(const morda::mouse_button_event& event){
	if(event.button != morda::mouse_button::left){
		return false;
	}

	// this->leftMouseButtonDown = e.is_down;
	
	if(event.is_down){
		// this->set_cursor_index(this->posToIndex(e.pos.x()));
		this->focus();
	}
	
	return true;
}
