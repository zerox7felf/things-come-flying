# Makefile

include config.mk

all: prepare compile

prepare:
	mkdir -p ${BUILD_DIR}

compile:
	${CC} ${FLAGS} ${O_DEBUG}

run:
	./${BUILD_DIR}/${PROG}
