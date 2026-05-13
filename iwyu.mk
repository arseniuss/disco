# iwyu.mk - IWYU rules using make conditionals

ifneq ($(SUBDIRS),)

.PHONY: iwyu-all iwyu-clean

iwyu-all:
	@failed=0; \
	for dir in $(SUBDIRS); do \
		echo "Checking IWYU in $$dir..."; \
		$(MAKE) $(AM_MAKEFLAGS) -C $$dir iwyu-all || failed=1; \
	done; \
	exit $$failed

iwyu-clean:
	@for dir in $(SUBDIRS); do \
		$(MAKE) $(AM_MAKEFLAGS) -C $$dir iwyu-clean; \
	done

else

# No subdirectories - process local sources
iwyu_sources ?= $(SOURCES)
iwyu_sources := $(filter %.c %.cpp %.cc %.cxx %.h %.hpp, $(iwyu_sources))
iwyu_sources := $(sort $(iwyu_sources))

.PHONY: iwyu-all iwyu-clean

iwyu-all: $(addprefix iwyu-check-, $(iwyu_sources))

iwyu-clean:
	-rm -f *.iwyu.out

iwyu-check-%.h: %.h
	$(IWYU) $(IWYU_FLAGS) $(AM_IWYU_FLAGS) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) \
		$(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CFLAGS) $(CFLAGS) $<

iwyu-check-%.c: %.c
	$(IWYU) $(IWYU_FLAGS) $(AM_IWYU_FLAGS) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) \
		$(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CFLAGS) $(CFLAGS) $<

iwyu-check-%.cpp: %.cpp
	$(IWYU) $(IWYU_FLAGS) $(AM_IWYU_FLAGS) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) \
		$(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS) $<

iwyu-check-%.cc: %.cc
	$(IWYU) $(IWYU_FLAGS) $(AM_IWYU_FLAGS) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) \
		$(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS) $<

iwyu-check-%.cxx: %.cxx
	$(IWYU) $(IWYU_FLAGS) $(AM_IWYU_FLAGS) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) \
		$(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS) $<

endif
