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

#include "shortcut_resolver.hpp"

#include <tml/tree.hpp>

using namespace cod;

using namespace std::string_literals;

shortcut_resolver::shortcut_resolver(const papki::file& f)
{
	this->load(f);
}

namespace {
shortcut_resolver::shortcut parse_shortcut(const tml::tree& sc)
{
	shortcut_resolver::shortcut ret;
	ret.name = sc.value.string;

	for (const auto& k : sc.children) {
		const auto& name = k.value.string;
		auto key = ruis::to_key(name);
		if (key == ruis::key::unknown) {
			throw std::invalid_argument("unknown key name: "s + name);
		}

		auto mod = ruis::to_key_modifier(key);
		if (mod != ruis::key_modifier::unknown) {
			if (ret.combo.modifiers.get(mod)) {
				throw std::invalid_argument("key modifier '"s + name + "' specified twice");
			}
			ret.combo.modifiers.set(mod);
		} else {
			if (ret.combo.key != ruis::key::unknown) {
				throw std::invalid_argument(
					"two non-modifier keys ("s + std::string(ruis::to_string(ret.combo.key)) + ", " + name +
					") specified in same shortcut"
				);
			}
			ret.combo.key = key;
		}
	}

	return ret;
}
} // namespace

void shortcut_resolver::load(const papki::file& f)
{
	auto dom = tml::read(f);

	for (const auto& s : dom) {
		try {
			auto sc = parse_shortcut(s);
			this->shortcuts[sc.name] = std::move(sc);
		} catch (const std::invalid_argument& e) {
			std::stringstream ss;
			ss << e.what() << std::endl;
			ss << "  while parsing shortcut: " << s.value.string << std::endl;
			ss << "  from file: " << f.path();
			throw std::invalid_argument(ss.str());
		}
	}
}

const shortcut_resolver::shortcut& shortcut_resolver::get(std::string_view name) const
{
	auto i = this->shortcuts.find(name);

	// if shortcut does not exist, then create an empty one
	if (i == this->shortcuts.end()) {
		shortcut sc;
		sc.name = name;
		auto j = this->shortcuts.insert(std::make_pair(std::string_view(sc.name), std::move(sc)));
		ASSERT(j.second)
		i = j.first;
	}

	return i->second;
}
