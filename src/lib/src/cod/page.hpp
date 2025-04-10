/*
cod - text editor

Copyright (C) 2021-2025  Ivan Gagis <igagis@gmail.com>

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

#pragma once

#include <ruis/widget/button/tab.hpp>
#include <ruis/widget/group/book.hpp>

namespace cod {

class page : public ruis::page
{
public:
	page(utki::shared_ref<ruis::context> context);

	bool on_key(const ruis::key_event& e) override;

	virtual utki::shared_ref<ruis::widget> create_tab_content() = 0;

private:
	void move_right();
};

} // namespace cod
