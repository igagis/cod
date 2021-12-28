#pragma once

#include <unordered_map>

#include <utki/flags.hpp>
#include <papki/file.hpp>

#include <morda/util/key.hpp>

namespace cod{

class shortcut_resolver{
public:
	struct shortcut{
		std::string name;
		utki::flags<morda::key_modifier> modifiers;
		morda::key key = morda::key::unknown;
	};

private:
	mutable std::unordered_map<std::string_view, shortcut> shortcuts;

public:
	shortcut_resolver(const papki::file& f);

	void load(const papki::file& f);

	const shortcut& get(std::string_view name)const;
};

}
