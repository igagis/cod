#include <mordavokne/application.hpp>

class Application : public mordavokne::application{
public:
	Application() :
			application(
					"cod",
					[](){
						return mordavokne::window_params(r4::vector2<unsigned>(320, 480));
					}()
				)
	{
		this->gui.initStandardWidgets(*this->get_res_file());
		
//		morda::inst().resMan.mountResPack(*this->getResFile("res/"));
		
		auto c = this->gui.context->inflater.inflate(
				*this->get_res_file("res/main.gui")
			);
		
		this->gui.set_root(std::move(c));
	}
};

std::unique_ptr<mordavokne::application> mordavokne::create_application(int argc, const char** argv){
	return std::make_unique<::Application>();
}
