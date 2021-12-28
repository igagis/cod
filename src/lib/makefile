include prorab.mk

$(eval $(call prorab-config, ../../config))

this_name := cod

this_soname := $(shell cat $(d)soname.txt)

this_srcs := $(call prorab-src-dir, src)

this_cxxflags += -I../3rd_party

$(eval $(prorab-build-lib))

this_license_file := ../../LICENSE
this_src_dir := src
$(eval $(prorab-license))
