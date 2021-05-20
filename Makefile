# Makefile

include config.mk

all: prepare compile

prepare:
	mkdir -p ${BUILD_DIR}

compile:
	${CC} ${FLAGS} ${LIB} ${O_RELEASE}

run:
	./${BUILD_DIR}/${PROG}
