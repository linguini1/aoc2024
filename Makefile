SUBDIRS = $(patsubst %/,%,$(wildcard */))
CLEAN_DIRS = $(addsuffix -clean,$(SUBDIRS))

# Allow us to call Makefiles in the subdirectories

.PHONY: $(SUBDIRS)

all: $(SUBDIRS)
	@echo "All built!"

# Any day can be made

$(SUBDIRS):
	$(MAKE) -C $@

%-clean: %
	$(MAKE) -C $< clean

clean: $(CLEAN_DIRS)
	@echo "All clean!"
