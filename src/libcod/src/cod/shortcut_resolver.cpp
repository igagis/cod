#include "shortcut_resolver.hpp"

using namespace cod;

void shortcut_resolver::load(const papki::file& f){
	// TODO:
}

const shortcut_resolver::shortcut& shortcut_resolver::get(const std::string& name)const{
	return this->shortcuts[name];
}
