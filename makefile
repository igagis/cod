include prorab.mk

this_name := cod

this_srcs += $(call prorab-src-dir,src)

$(eval $(call prorab-config, config))

ifeq ($(debug),true)
    this_cxxflags += -DDEBUG
endif

this_ldlibs += -lmordavokne-opengl2 -lmorda-opengl2-ren -lGLEW -lGL -lstdc++ -lmorda -lpuu -lutki -rdynamic -lm

$(eval $(prorab-build-app))
