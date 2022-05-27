#pragma once

#include "page.hpp"

namespace cod{

class file_page : public page{
	friend class plugin_manager;

	std::string file_name;
public:
	file_page(std::shared_ptr<morda::context> context);

	std::string_view get_name()const override{
		return this->file_name;
	}
	
	void on_tear_out()noexcept override;
};

}
