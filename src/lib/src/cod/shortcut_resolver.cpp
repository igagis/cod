#include "shortcut_resolver.hpp"

#include <treeml/tree.hpp>

using namespace cod;

using namespace std::string_literals;

shortcut_resolver::shortcut_resolver(const papki::file& f){
	this->load(f);
}

namespace{
shortcut_resolver::shortcut parse_shortcut(const treeml::tree& sc){
	shortcut_resolver::shortcut ret;
	ret.name = sc.value.to_string();

	for(const auto& k : sc.children){
		const auto& name = k.value.to_string();
		auto key = morda::to_key(name);
		if(key == morda::key::unknown){
			throw std::invalid_argument("unknown key name: "s + name);
		}

		auto mod = morda::to_key_modifier(key);
		if(mod != morda::key_modifier::unknown){
			if(ret.combo.modifiers.get(mod)){
				throw std::invalid_argument("key modifier '"s + name + "' specified twice");
			}
			ret.combo.modifiers.set(mod);
		}else{
			if(ret.combo.key != morda::key::unknown){
				throw std::invalid_argument(
						"two non-modifier keys ("s +
								std::string(morda::to_string(ret.combo.key)) + ", " +
								name +
								") specified in same shortcut");
			}
			ret.combo.key = key;
		}
	}

	return ret;
}
}

void shortcut_resolver::load(const papki::file& f){
	auto dom = treeml::read(f);

	for(const auto& s : dom){
		try{
			auto sc = parse_shortcut(s);
			this->shortcuts[sc.name] = std::move(sc);
		}catch(const std::invalid_argument& e){
			std::stringstream ss;
			ss << e.what() << std::endl;
			ss << "  while parsing shortcut: " << s.value.to_string() << std::endl;
			ss << "  from file: " << f.path();
			throw std::invalid_argument(ss.str());
		}
	}
}

const shortcut_resolver::shortcut& shortcut_resolver::get(std::string_view name)const{
	auto i = this->shortcuts.find(name);

	// if shortcut does not exist, then create an empty one
	if(i == this->shortcuts.end()){
		shortcut sc;
		sc.name = name;
		auto j = this->shortcuts.insert(
				std::make_pair(
						std::string_view(sc.name),
						std::move(sc)
					)
			);
		ASSERT(j.second)
		i = j.first;
	}

	return i->second;
}
