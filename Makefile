SUBDIRS := FS-BOF Exit-BOF TK-BOF PS-BOF Postex-BOF

.PHONY: all $(SUBDIRS) clean docker-build

all: $(SUBDIRS)
$(SUBDIRS):
	@echo "=====>>  $@"
	@$(MAKE) --no-print-directory -C $@
	@echo ""

clean:
	@for dir in $(SUBDIRS); do \
		$(MAKE) --no-print-directory -C $$dir clean; \
	done

docker-build:
	docker compose build --no-cache && docker compose run bof-build
