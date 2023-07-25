# The output directory to store the built executable
OUTPUT_DIR = bin/

# The output target
TARGET = $(OUTPUT_DIR)test_prospect

# The operating system
ifndef $(OPERATING_SYSTEM)
	OPERATING_SYSTEM := $(shell uname -s)
	ifneq ($(OPERATING_SYSTEM), QNX)
		OPERATING_SYSTEM := $(shell uname -o)
	endif
endif

SRC = main.cpp \
      test_prospect.cpp

CPU_ARCHITECTURE := $(shell uname -m | sed "s/i.86/x86-32/" | sed "s/x86_64/x86-64/")
LINK_FLAGS += -L../../../../lib/$(CPU_ARCHITECTURE)
	  
# Operating system specific properties
ifneq ($(OPERATING_SYSTEM), QNX)
	ifneq ($(OPERATING_SYSTEM), Cygwin)
		CFLAGS += -pthread
	endif
	
	LINK_FLAGS += -lrt -lpthread
endif

ifdef DEBUG
	CFLAGS += -g -O0
else
	CFLAGS += -O3
endif

CFLAGS += -Wall -Wextra -I../../../../inc/star -I../../inc -I../common

OBJS = $(addsuffix .o, $(basename $(SRC)))

all: $(OUTPUT_DIR) $(TARGET)

$(OUTPUT_DIR):
	mkdir -p $(OUTPUT_DIR)

.cpp.o:
	$(CXX) $(CFLAGS) -c $< -o $@

$(TARGET) : $(OBJS)
	$(CXX) $(CFLAGS) $^ -lstar-api -lstar_conf_api_router -lstar_conf_api_mk2 -lstar_conf_api_brick_mk2 -lstar_conf_api_pxi $(LINK_FLAGS) -o $@

clean:
	$(RM) $(OBJS) $(TARGET) $(RTEMS_INIT_C)
