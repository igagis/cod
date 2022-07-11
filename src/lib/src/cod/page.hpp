/*
cod - text editor

Copyright (C) 2021  Ivan Gagis <igagis@gmail.com>

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

#include <morda/widgets/group/book.hpp>
#include <morda/widgets/button/tab.hpp>

namespace cod{

class page : public morda::page{
public:
	page(std::shared_ptr<morda::context> context);

	bool on_key(const morda::key_event& e)override;

	virtual std::shared_ptr<morda::widget> create_tab_content() = 0;

private:
	void move_right();
};

}
