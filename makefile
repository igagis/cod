include prorab.mk

this_name := cod

this_srcs += $(call prorab-src-dir,src)

$(eval $(call prorab-config, config))

ifeq ($(debug),true)
    this_cxxflags += -DDEBUG
endif

this_ldlibs += -lmordavokne-opengl2 -lmorda -ltreeml -lclargs -lutki -rdynamic -lm

$(eval $(prorab-build-app))

define this_rules
run: $(prorab_this_name)
$(.RECIPEPREFIX)$(a)$(prorab_this_name)
endef
$(eval $(this_rules))
