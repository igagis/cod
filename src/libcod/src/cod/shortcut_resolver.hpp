#pragma once

#include <unordered_map>

#include <utki/flags.hpp>
#include <papki/file.hpp>

#include <morda/util/key.hpp>

namespace cod{

class shortcut_resolver{

public:
	struct shortcut{
		utki::flags<morda::key_modifier> modifiers;
		morda::key key = morda::key::unknown;
	};

private:
	mutable std::unordered_map<std::string, shortcut> shortcuts;

public:
	void load(const papki::file& f);

	const shortcut& get(const std::string& name)const;
};

}
