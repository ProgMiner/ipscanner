# MIT License
#
# Copyright (c) 2018 Eridan Domoratskiy
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

CC = gcc
CFLAGS = -std=c99 -Wall -O2
LDFLAGS =

BUILDPATH = build
SOURCES = linux.c main.c options.c util.c win32.c
HEADERS = bool.h global.h main.h options.h platform.h util.h
TARGET = ipscanner

OBJECTS = $(SOURCES:%.c=$(BUILDPATH)/%.o)

ifeq ($(OS), Windows_NT)
    LDFLAGS += -lws2_32
endif

.PHONY: all build clean run

all: build

clean:
ifneq ($(OS), Windows_NT)
	rm -rf $(BUILDPATH)
else
	del /F /S /Q $(BUILDPATH)
endif

run:
	"$(BUILDPATH)/$(TARGET)" $(ARGS)

build: $(TARGET)

%.c:

$(BUILDPATH)/%.o: %.c $(HEADERS)
ifneq ($(OS), Windows_NT)
	mkdir -p $(dir $@)
else
	if not exist "$(subst /,\,$(dir $@))" mkdir $(subst /,\,$(dir $@))
endif
	$(CC) -c -o $@ $< $(CFLAGS)

$(TARGET): $(OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)
