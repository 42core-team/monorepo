.PHONY: sync-base check-base

SYNC_BASE_ARGS :=

ifneq ($(strip $(LANGS)),)
SYNC_BASE_ARGS += $(foreach lang,$(LANGS),--lang $(lang))
endif

ifneq ($(strip $(MODES)),)
SYNC_BASE_ARGS += $(foreach mode,$(MODES),--mode $(mode))
endif

ifeq ($(VERBOSE),1)
SYNC_BASE_ARGS += --verbose
endif

sync-base:
	bash ./sync_base_resources.sh $(SYNC_BASE_ARGS)

check-base:
	bash ./sync_base_resources.sh --check --verbose $(SYNC_BASE_ARGS)
