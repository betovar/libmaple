# Standard things
sp := $(sp).x
dirstack_$(sp) := $(d)
d := $(dir)
BUILDDIRS += $(BUILD_PATH)/$(d)

# Local flags
CXXFLAGS_$(d) := $(WIRISH_INCLUDES) $(LIBMAPLE_INCLUDES)
CFLAGS_$(d) := $(WIRISH_INCLUDES) $(LIBMAPLE_INCLUDES)

# Local rules and targets
sSRCS_$(d) := cr4_fft_64_stm32.s        \
              cr4_fft_256_stm32.s       \
              cr4_fft_1024_stm32.s      \

#cFILES_$(d) := $(cSRCS_$(d):%=$(d)/%)
sFILES_$(d) := $(sSRCS_$(d):%=$(d)/%)

#$(cFILES_$(d):%.c=$(BUILD_PATH)/%.o)
OBJS_$(d) := $(sFILES_$(d):%.S=$(BUILD_PATH)/%.o)
DEPS_$(d) := $(OBJS_$(d):%.o=%.d)

#$(OBJS_$(d)): TGT_CFLAGS := $(CFLAGS_$(d))
$(OBJS_$(d)): TGT_ASFLAGS :=

TGT_BIN += $(OBJS_$(d))

# Standard things
-include        $(DEPS_$(d))
d               := $(dirstack_$(sp))
sp              := $(basename $(sp))
