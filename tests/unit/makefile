include prorab.mk
include prorab-test.mk

$(eval $(call prorab-config, ../../config))

this_name := unit_tests

this_srcs := $(call prorab-src-dir, .)

this_srcs += ../../src/cod/syntax_highlight/regex_syntax_highlighter.cpp

this_cxxflags += -I../../src

this_ldlibs += -ltst -lpapki -ltreeml

$(eval $(prorab-build-app))

this_test_cmd := $(prorab_this_name)
this_test_deps := $(prorab_this_name)
this_test_ld_path := $(prorab_space)
$(eval $(prorab-test))
