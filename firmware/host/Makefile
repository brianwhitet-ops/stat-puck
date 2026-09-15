CC ?= gcc
CFLAGS ?= -std=c11 -Wall -Wextra -Werror -O2
CPPFLAGS = -I../include -I. -DSLAB_HOST=1
LDFLAGS = -lz

CORE = \
	../src/slab_fb.c \
	../src/slab_font.c \
	../src/slab_font_draw.c \
	../src/slab_ui.c \
	../src/slab_frames.c \
	../src/slab_frames_1bit.c \
	../src/slab_state.c \
	../src/slab_input.c \
	../src/slab_persist.c \
	../src/slab_ble.c \
	../src/slab_epd.c

GOLD = ../../design/instinct-firmware-goldens-pr1

.PHONY: all bench test visual clean

all: bench test

bench: bench_render
test: test_state test_device_1bit

bench_render: bench_main.c png_write.c $(CORE)
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ bench_main.c png_write.c $(CORE) $(LDFLAGS)

test_state: ../tests/test_state.c $(CORE)
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ ../tests/test_state.c $(CORE) $(LDFLAGS)

# Non-host / device compile: no RGBA, no zlib, packed 1-bit frames only.
DEVICE_CORE = \
	../src/slab_fb.c \
	../src/slab_ui.c \
	../src/slab_frames_1bit.c \
	../src/slab_state.c \
	../src/slab_input.c \
	../src/slab_persist.c \
	../src/slab_ble.c \
	../src/slab_epd.c

test_device_1bit: ../tests/test_device_1bit.c $(DEVICE_CORE)
	$(CC) $(CFLAGS) -I../include -I. -DSLAB_HOST=0 -o $@ \
		../tests/test_device_1bit.c $(DEVICE_CORE)

visual: clean bench_render test_state test_device_1bit
	python3 ../tests/verify_sha256.py $(GOLD)/SHA256SUMS.txt
	./test_state
	./test_device_1bit
	./bench_render ../testdata/out
	python3 ../tests/compare_goldens.py ../testdata/out $(GOLD)

clean:
	rm -f bench_render test_state test_device_1bit
