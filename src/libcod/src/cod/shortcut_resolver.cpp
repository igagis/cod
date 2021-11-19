#include "shortcut_resolver.hpp"

#include <treeml/tree.hpp>

using namespace cod;

using namespace std::string_literals;

shortcut_resolver::shortcut_resolver(const papki::file& f){
	this->load(f);
}

namespace{
shortcut_resolver::shortcut parse_shortcut(const treeml::forest& keys){
	shortcut_resolver::shortcut ret;

	for(const auto& k : keys){
		const auto& name = k.value.to_string();
		auto key = morda::to_key(name);
		if(key == morda::key::unknown){
			throw std::invalid_argument("unknown key name: "s + name);
		}

		auto mod = morda::to_key_modifier(key);
		if(mod != morda::key_modifier::unknown){
			if(ret.modifiers.get(mod)){
				throw std::invalid_argument("key modifier '"s + name + "' specified twice");
			}
			ret.modifiers.set(mod);
		}else{
			if(ret.key != morda::key::unknown){
				throw std::invalid_argument(
						"two non-modifier keys ("s +
								std::string(morda::to_string(ret.key)) + ", " +
								name +
								") specified in same shortcut");
			}
			ret.key = key;
		}
	}

	return ret;
}
}

void shortcut_resolver::load(const papki::file& f){
	auto dom = treeml::read(f);

	for(const auto& s : dom){
		this->shortcuts[s.value.to_string()] = parse_shortcut(s.children);
	}
}

const shortcut_resolver::shortcut& shortcut_resolver::get(const std::string& name)const{
	return this->shortcuts[name];
}
