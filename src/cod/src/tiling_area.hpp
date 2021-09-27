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

#include <morda/widgets/container.hpp>
#include <morda/widgets/base/oriented_widget.hpp>

namespace cod{

class tiling_area :
        virtual public morda::widget,
        public morda::oriented_widget,
        private morda::container
{
    const morda::real min_tile_size;
    const morda::real dragger_size;

    std::shared_ptr<morda::container> content;
public:
    tiling_area(std::shared_ptr<morda::context> c, const treeml::forest& desc);

    void push_back(std::shared_ptr<widget> w);

    void lay_out()override;

    morda::vector2 measure(const morda::vector2& quotum)const override;

private:
    
};

}
