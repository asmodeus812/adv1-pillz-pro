DOCKER := $(shell { command -v podman || command -v docker; })
TIMESTAMP := $(shell date -u +"%Y%m%d%H%M")
COMMIT := $(shell git rev-parse --short HEAD 2>/dev/null)
ifeq ($(shell uname),Darwin)
SELINUX1 :=
SELINUX2 :=
else
SELINUX1 := :z
SELINUX2 := ,z
endif

.PHONY: all clean_firmware clean_image clean

all:
	$(shell bin/get_version.sh >> /dev/null)
	$(DOCKER) build --tag pillzmod_pro --file Dockerfile .
	$(DOCKER) run --rm -it --name pillzmod_pro \
		-v $(PWD)/firmware:/app/firmware$(SELINUX1) \
		-v $(PWD)/config:/app/config:ro$(SELINUX2) \
		-e TIMESTAMP=$(TIMESTAMP) \
		-e COMMIT=$(COMMIT) \
		pillzmod_pro

clean_firmware:
	rm -f firmware/*.uf2

clean_image:
	$(DOCKER) image rm pillzmod_pro docker.io/zmkfirmware/zmk-build-arm:stable

clean: clean_firmware clean_image
