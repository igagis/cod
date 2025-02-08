/*
cod - text editor

Copyright (C) 2021-2024  Ivan Gagis <igagis@gmail.com>

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

#include "file_page.hpp"

#include <ruis/widget/label/text.hpp>

#include "context.hpp"
#include "file_opener.hpp"

using namespace cod;

file_page::file_page(const utki::shared_ref<ruis::context>& context, std::string&& file_name) :
	ruis::widget(std::move(context), tml::forest()),
	page(this->context),
	file_name(std::move(file_name))
{}

void file_page::on_tear_out() noexcept
{
	// remove the page from list of open files

	auto& ctx = context::inst();

	auto i = ctx.file_opener.open_files.find(file_name);
	if (i != ctx.file_opener.open_files.end()) {
		ctx.file_opener.open_files.erase(i);
	}
}

utki::shared_ref<ruis::widget> file_page::create_tab_content()
{
	auto t = utki::make_shared<ruis::text>(this->context, tml::forest());
	t.get().set_text(file_name);
	return t;
}
