LITMUS_REMOTE ?= https://github.com/litmus-tests/litmus-tests-riscv.git
LITMUS_COMMIT ?=
DEFAULT_LITMUS_ROOT := $(CURDIR)/.cache/litmus-tests-riscv
LITMUS_ROOT ?= $(DEFAULT_LITMUS_ROOT)
OUT ?= $(CURDIR)/generated
BUILD_DIR ?= $(CURDIR)/build
SCRIPT := $(CURDIR)/scripts/litmus_to_am.py
ARCH ?= riscv64-xs-dual
AM_HOME ?= $(abspath $(CURDIR)/../..)
NAME := litmus-dual

PYTHON ?= python3
GEN_ARGS ?=
MAX_TESTS ?=
INPUT ?=
CATEGORY ?=

APP_DIR := $(OUT)
SRCS := $(shell if [ -d "$(OUT)/src" ]; then cd "$(OUT)" && find -L ./src -name '*.c' | sed 's#^\./##' | sort; fi)
VPATH += $(OUT)
INC_DIR += $(OUT)/include
DST_DIR := $(BUILD_DIR)/$(ARCH)
BINARY := $(BUILD_DIR)/$(NAME)-$(ARCH)
INTERNAL_APP_GOALS := $(filter _build-app _run-app,$(MAKECMDGOALS))
BUILD_VARS := ARCH="$(ARCH)" AM_HOME="$(AM_HOME)" \
	$(if $(LINUX_GNU_TOOLCHAIN),LINUX_GNU_TOOLCHAIN="$(LINUX_GNU_TOOLCHAIN)") \
	$(if $(MARCH),MARCH="$(MARCH)")

.PHONY: all fetch update generate generate-tui categories build run clean clean-generated clean-cache clean-build _build-app _run-app

all: fetch
	@rm -f "$(OUT)/.litmus-generated.stamp"
	$(PYTHON) "$(SCRIPT)" \
		--repo-root "$(LITMUS_ROOT)" \
		--output-dir "$(OUT)" \
		--clean \
		--interactive \
		$(if $(MAX_TESTS),--max-tests "$(MAX_TESTS)") \
		$(GEN_ARGS)
	@if [ -f "$(OUT)/.litmus-generated.stamp" ]; then \
		$(MAKE) build $(BUILD_VARS); \
	fi

fetch:
	@if [ "$(LITMUS_ROOT)" != "$(DEFAULT_LITMUS_ROOT)" ]; then \
		test -d "$(LITMUS_ROOT)/.git" || { echo "missing LITMUS_ROOT git repo: $(LITMUS_ROOT)" >&2; exit 1; }; \
		echo "using existing litmus repository: $(LITMUS_ROOT)"; \
	elif [ ! -d "$(LITMUS_ROOT)/.git" ]; then \
		mkdir -p "$(dir $(LITMUS_ROOT))"; \
		git clone "$(LITMUS_REMOTE)" "$(LITMUS_ROOT)"; \
	fi
	@if [ "$(LITMUS_ROOT)" = "$(DEFAULT_LITMUS_ROOT)" ]; then \
		if [ -n "$(LITMUS_COMMIT)" ]; then \
			git -C "$(LITMUS_ROOT)" fetch --tags origin; \
			git -C "$(LITMUS_ROOT)" checkout "$(LITMUS_COMMIT)" || { \
				echo "failed to checkout LITMUS_COMMIT=$(LITMUS_COMMIT)" >&2; \
				echo "leave LITMUS_COMMIT empty to use the upstream default branch" >&2; \
				exit 1; \
			}; \
		fi; \
	fi

update:
	@if [ "$(LITMUS_ROOT)" != "$(DEFAULT_LITMUS_ROOT)" ]; then \
		test -d "$(LITMUS_ROOT)/.git" || { echo "missing LITMUS_ROOT git repo: $(LITMUS_ROOT)" >&2; exit 1; }; \
		echo "not updating external LITMUS_ROOT: $(LITMUS_ROOT)"; \
	elif [ ! -d "$(LITMUS_ROOT)/.git" ]; then \
		mkdir -p "$(dir $(LITMUS_ROOT))"; \
		git clone "$(LITMUS_REMOTE)" "$(LITMUS_ROOT)"; \
	else \
		git -C "$(LITMUS_ROOT)" fetch --tags origin; \
		git -C "$(LITMUS_ROOT)" checkout master; \
		git -C "$(LITMUS_ROOT)" pull --ff-only origin master; \
	fi

categories: fetch
	$(PYTHON) "$(SCRIPT)" --repo-root "$(LITMUS_ROOT)" --list-categories

generate: fetch
	$(PYTHON) "$(SCRIPT)" \
		--repo-root "$(LITMUS_ROOT)" \
		--output-dir "$(OUT)" \
		--clean \
		$(if $(INPUT),--input "$(INPUT)") \
		$(if $(CATEGORY),--category "$(CATEGORY)") \
		$(if $(MAX_TESTS),--max-tests "$(MAX_TESTS)") \
		$(GEN_ARGS)

generate-tui: fetch
	$(PYTHON) "$(SCRIPT)" \
		--repo-root "$(LITMUS_ROOT)" \
		--output-dir "$(OUT)" \
		--clean \
		--interactive \
		$(if $(MAX_TESTS),--max-tests "$(MAX_TESTS)") \
		$(GEN_ARGS)

build: fetch
	@if [ ! -f "$(OUT)/.litmus-generated.stamp" ]; then \
		$(PYTHON) "$(SCRIPT)" \
			--repo-root "$(LITMUS_ROOT)" \
			--output-dir "$(OUT)" \
			--clean \
			$(if $(INPUT),--input "$(INPUT)") \
			$(if $(CATEGORY),--category "$(CATEGORY)") \
			$(if $(MAX_TESTS),--max-tests "$(MAX_TESTS)") \
			$(GEN_ARGS); \
	fi
	$(MAKE) _build-app $(BUILD_VARS)

ifeq ($(INTERNAL_APP_GOALS),)
run: build
	$(MAKE) _run-app $(BUILD_VARS)
endif

clean: clean-build clean-generated clean-cache

clean-generated:
	rm -rf "$(OUT)"

clean-cache:
	rm -rf "$(CURDIR)/.cache"

clean-build:
	rm -rf "$(BUILD_DIR)"

_build-app: image

_run-app: run

ifneq ($(INTERNAL_APP_GOALS),)
include $(AM_HOME)/Makefile.app
endif
