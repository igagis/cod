#pragma once

#include <morda/widgets/CharInputWidget.hpp>

class CodeEdit : public morda::CharInputWidget{
public:
	CodeEdit(const stob::Node* chain);
	
	CodeEdit(const CodeEdit&) = delete;
	CodeEdit& operator=(const CodeEdit&) = delete;
	
	
	void onCharacterInput(const std::u32string& unicode, morda::Key_e key)override;
private:

};
