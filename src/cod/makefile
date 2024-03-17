include prorab.mk
include prorab-test.mk
include prorab-license.mk
include prorab-clang-format.mk

this_name := cod

this_srcs += $(call prorab-src-dir,src)

$(eval $(call prorab-config, ../../config))

this_cxxflags += -I ../lib/src

this_libcod := ../lib/out/$(c)/libcod.so

this_ldflags += -L $(dir $(this_libcod))
this_ldlibs += -lcod -lruisapp-opengl -lruis -ltml -lpapki -lclargs -lutki -rdynamic -lm

$(eval $(prorab-build-app))

$(eval $(prorab-clang-format))

$(eval $(call prorab-depend, $(prorab_this_name), $(this_libcod) ../plugins/text_editor/out/$(c)/text_editor.so))

this_run_name := $(this_name)
this_test_cmd := cd ../.. && $(prorab_this_name) --plugin=src/plugins/text_editor/out/$(c)/text_editor.so.$(shell cat $(d)../lib/soname.txt)
this_test_deps := $(prorab_this_name)
this_test_ld_path := src/cod/$(dir $(this_libcod))
$(eval $(prorab-run))

this_license_file := ../../LICENSE
this_src_dir := src
$(eval $(prorab-license))

$(eval $(call prorab-include, ../plugins/text_editor/makefile))
