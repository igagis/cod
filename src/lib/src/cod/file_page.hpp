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

namespace cod{

class file_page : public morda::page{ // TODO: make private inheritance
	friend class plugin_manager;

	std::string file_name;
public:
	file_page(std::shared_ptr<morda::context> context);

	const std::string& get_file_name()const{
		return this->file_name;
	}
	
	void on_tear_out()noexcept override;
};

}
