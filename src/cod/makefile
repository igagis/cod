include prorab.mk
include prorab-test.mk
include prorab-license.mk

this_name := cod

this_srcs += $(call prorab-src-dir,src)

$(eval $(call prorab-config, ../../config))

this_cxxflags += -I../3rd_party

this_ldlibs += -lmordavokne-opengl -lmorda -ltreeml -lpapki -lclargs -lutki -rdynamic -lm

$(eval $(prorab-build-app))

this_run_name := $(this_name)
this_test_cmd := cd ../.. && $(prorab_this_name)
this_test_deps := $(prorab_this_name)
this_test_ld_path := $(prorab_space)
$(eval $(prorab-run))

this_license_file := ../../LICENSE
this_src_dir := src
$(eval $(prorab-license))
