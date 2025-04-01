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

#pragma once

#include <ruis/res/font.hpp>

namespace synhi {

struct font_style {
	ruis::res::font::style style = ruis::res::font::style::normal;
	bool underline = false;
	bool stroke = false;
	constexpr static auto default_color = 0xffffffff;
	ruis::color color = default_color;
};

struct line_span {
	size_t length = 0;
	std::shared_ptr<const font_style> style;
};

class highlighter
{
public:
	virtual std::vector<line_span> highlight(std::u32string_view str) = 0;

	virtual void reset() = 0;

	highlighter() = default;

	highlighter(const highlighter&) = delete;
	highlighter& operator=(const highlighter&) = delete;

	highlighter(highlighter&&) = delete;
	highlighter& operator=(highlighter&&) = delete;

	virtual ~highlighter() = default;
};

} // namespace synhi
