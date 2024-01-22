PREFIX    = /usr/local

CC      = cc
LD      = $(CC)

CFLAGS  = -std=c99 -pedantic -Wall -fPIC -Os
LDFLAGS = -lxcb-cursor -lxcb-randr -lxcb
