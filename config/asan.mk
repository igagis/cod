include $(config_dir)rel.mk

this_lint_cmd :=

this_cxxflags += -fsanitize=address
this_ldflags += -fsanitize=address
