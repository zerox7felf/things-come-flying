# Makefile

include config.mk

all: prepare compile

prepare:
	mkdir -p ${BUILD_DIR}

compile:
	${CC} ${FLAGS} ${LIB} ${O_DEBUG}

run:
	./${BUILD_DIR}/${PROG}
