include prorab.mk

$(eval $(call prorab-config, ../../../config))

this_name := text_editor

this_soname := $(shell cat $(d)../../lib/soname.txt)

this_lib_prefix :=

this_no_install := true

this_cxxflags += -I../../lib/src

this_srcs = $(call prorab-src-dir, src)

$(eval $(prorab-build-lib))

this_license_file := ../../../LICENSE
this_src_dir := src
$(eval $(prorab-license))
