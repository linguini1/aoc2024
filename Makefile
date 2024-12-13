COMMON = common
SUBDIRS = $(patsubst %/,%,$(wildcard */))
DAYS = $(filter-out $(COMMON),$(SUBDIRS))
CLEAN_DAYS = $(addsuffix -clean,$(DAYS))

# Allow us to make days

.PHONY: $(DAYS)

all: $(DAYS)
	@echo "All built!"

# Any day can be made

$(DAYS):
	$(MAKE) -C $@ DAY=$@

%-clean: %
	$(MAKE) -C $< clean DAY=$<

clean: $(CLEAN_DAYS)
	@echo "All clean!"
