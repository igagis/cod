include prorab.mk
include prorab-test.mk

$(eval $(call prorab-config, ../../config))

this_name := unit_tests

this_srcs := $(call prorab-src-dir, src)

this_srcs += ../../src/cod/synhi/regex_highlighter.cpp

this_cxxflags += -I../../src -I../../src/3rd_party

this_ldlibs += -ltst -lpapki -ltreeml -lutki

$(eval $(prorab-build-app))

this_test_cmd := $(prorab_this_name)
this_test_deps := $(prorab_this_name)
this_test_ld_path := $(prorab_space)
$(eval $(prorab-test))
