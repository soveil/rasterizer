CC := g++
CCFLAGS := -Wall -Wextra

RELEASE_FLAGS = -O3
DEBUG_FLAGS = -g -O0
PROFILE_FLAGS = -g -pg

RELEASE_DIR = build/release
DEBUG_DIR = build/debug
PROFILE_DIR = build/profile

SRCS := $(wildcard src/*.cc src/types/*.cc)

RELEASE_OBJS = $(patsubst %.cc, $(RELEASE_DIR)/%.o, $(notdir $(SRCS)))
DEBUG_OBJS = $(patsubst %.cc, $(DEBUG_DIR)/%.o,   $(notdir $(SRCS)))
PROFILE_OBJS = $(patsubst %.cc, $(PROFILE_DIR)/%.o,   $(notdir $(SRCS)))

EXE := rd_view

vpath %.cc src src/types

.PHONY: all release debug profile clean

all: debug
release: $(RELEASE_DIR)/$(EXE)
debug: $(DEBUG_DIR)/$(EXE)
profile: $(PROFILE_DIR)/$(EXE)

$(RELEASE_DIR)/$(EXE): $(RELEASE_OBJS) lib/libcs631.a | $(RELEASE_DIR)
	$(CC) $(CCFLAGS) $(RELEASE_FLAGS) -o $@ $^ -lm -lX11

$(DEBUG_DIR)/$(EXE): $(DEBUG_OBJS) lib/libcs631.a | $(DEBUG_DIR)
	$(CC) $(CCFLAGS) $(DEBUG_FLAGS) -o $@ $^ -lm -lX11

$(PROFILE_DIR)/$(EXE): $(PROFILE_OBJS) lib/libcs631.a | $(PROFILE_DIR)
	$(CC) $(CCFLAGS) $(PROFILE_FLAGS) -o $@ $^ -lm -lX11

$(RELEASE_DIR)/%.o: %.cc | $(RELEASE_DIR)
	$(CC) $(CCFLAGS) $(RELEASE_FLAGS) -MMD -MP -c $< -o $@

$(DEBUG_DIR)/%.o: %.cc | $(DEBUG_DIR)
	$(CC) $(CCFLAGS) $(DEBUG_FLAGS) -MMD -MP -c $< -o $@

$(PROFILE_DIR)/%.o: %.cc | $(PROFILE_DIR)
	$(CC) $(CCFLAGS) $(PROFILE_FLAGS) -MMD -MP -c $< -o $@

$(DEBUG_DIR) $(RELEASE_DIR) $(PROFILE_DIR):
	mkdir -p $@

-include $(RELEASE_OBJS:.o=.d)
-include $(DEBUG_OBJS:.o=.d)
-include $(PROFILE_OBJS:.o=.d)

clean:
	rm -rf build/
