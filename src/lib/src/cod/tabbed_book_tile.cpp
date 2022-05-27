#include "tabbed_book_tile.hpp"

#include <morda/widgets/label/text.hpp>

using namespace cod;

tabbed_book_tile::tabbed_book_tile(std::shared_ptr<morda::context> c, const treeml::forest& desc) :
		morda::widget(std::move(c), desc),
		tile(this->context, desc),
		tabbed_book(this->context, desc)
{}

namespace{

const treeml::forest tab_desc = treeml::read(R"(
		@tab{
			@row{
				@text{
					id{text}
					text{cube}
				}
				@push_button{
					id{close_button}
					@image{
						layout{
							dx { 8dp }
							dy { 8dp }
						}
						image{morda_img_close}
					}
				}
			}
		}
	)");

}

void tabbed_book_tile::add(std::shared_ptr<page> p){
	auto tab = this->context->inflater.inflate_as<morda::tab>(tab_desc);

	tab->get_widget_as<morda::text>("text").set_text(papki::not_dir(p->get_name()));
	
	tab->get_widget_as<morda::push_button>("close_button").click_handler = [
			tabbed_book_wp = utki::make_weak_from(*this),
			tab_wp = utki::make_weak(tab)
		](morda::push_button& btn)
	{
		auto tb = tabbed_book_wp.lock();
		ASSERT(tb)

		auto t = tab_wp.lock();
		ASSERT(t)

		btn.context->run_from_ui_thread([tb, t]{
			tb->tear_out(*t);
		});
	};

    this->morda::tabbed_book::add(tab, p);
}
