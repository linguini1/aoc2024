SUBDIRS = $(patsubst %/,%,$(wildcard */))
CLEAN_DIRS = $(addsuffix -clean,$(SUBDIRS))
COMMONDIR = common
DAYS = $(filter-out $(COMMONDIR),$(SUBDIRS))
RUN_DAYS = $(addsuffix -run,$(DAYS))

# Allow us to call Makefiles in the subdirectories

.PHONY: $(SUBDIRS)

all: $(SUBDIRS)
	@echo "All built!"

# Any day can be made

$(SUBDIRS):
	$(MAKE) -C $@

run: $(RUN_DAYS)

%-run:
	@$(MAKE) --silent -C $(patsubst %-run,%,$@) run

%-clean:
	$(MAKE) -C $(patsubst %-clean,%,$@) clean

clean: $(CLEAN_DIRS)
	@echo "All clean!"
