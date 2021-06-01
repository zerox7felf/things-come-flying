# config.mk

CC=g++

PROG=solar-system

RES_DIR=resource

BUILD_DIR=build

LIB=-lm -lGL -lGLU -lGLEW -lglfw -lpng

SRC=${wildcard src/*.cpp}

INC=include

FLAGS=${SRC} -o ${BUILD_DIR}/${PROG} -I${INC} -Wall

O_DEBUG=-O0 -g

O_RELEASE=-ffast-math -O2
