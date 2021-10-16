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

#include "application.hpp"

#include "tiling_area.hpp"

using namespace cod;

application::application(command_line_arguments&& cla) :
		mordavokne::application(
				"cod",
				[](){
					return mordavokne::window_params(r4::vector2<unsigned>(1024, 768));
				}()
			),
		cla(std::move(cla)),
		plugins(this->cla.plugins),
		file_opener([this]{
			this->gui.initStandardWidgets(*this->get_res_file());
	
			this->gui.context->inflater.register_widget<code_edit>("code_edit");
			this->gui.context->inflater.register_widget<file_tree>("file_tree");
			this->gui.context->inflater.register_widget<tiling_area>("tiling_area");

			this->gui.context->loader.mount_res_pack(
					*this->get_res_file("res/")
				);

			this->gui.set_root(
					this->gui.context->inflater.inflate(
							*this->get_res_file("res/main.gui")
						)
				);

			ASSERT(this->gui.get_root())
			auto& c = *this->gui.get_root();

			c.get_widget_as<file_tree>("file_tree")
					.file_select_handler = [](std::string file_name)
			{
				// std::cout << "file = " << file_name << '\n';

				if(papki::is_dir(file_name)){
					return;
				}

				application::inst().file_opener.open(file_name);
			};

			return c.try_get_widget_as<tiling_area>("base_tiling_area");
		}())
{}
