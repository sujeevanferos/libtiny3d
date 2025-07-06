# Compiler and flags
CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -g -Iinclude -O2
LDFLAGS = -lm # Link math library for functions like cosf, sinf, fabsf, etc.
AR = ar
ARFLAGS = rcs

# Directories
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
DEMO_DIR = demo
TEST_DIR = tests

# Source files and object files for the library
LIB_SRCS = $(wildcard $(SRC_DIR)/*.c)
LIB_OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(LIB_SRCS))
LIB_TARGET = $(BUILD_DIR)/libtiny3d.a

# Demo program
DEMO_MAIN_SRC = $(DEMO_DIR)/main.c
DEMO_MAIN_OBJ = $(BUILD_DIR)/main_demo.o # Specific name for demo's main object
DEMO_TARGET = $(BUILD_DIR)/demo

# Default target: build library and demo
all: $(LIB_TARGET) $(DEMO_TARGET)

# Rule to build the static library
$(LIB_TARGET): $(LIB_OBJS)
	@mkdir -p $(@D) # Ensure directory for the target exists
	$(AR) $(ARFLAGS) $@ $^
	@echo "Successfully built library: $@"

# Rule to build the demo program
# It depends on its own object file and the library
$(DEMO_TARGET): $(DEMO_MAIN_OBJ) $(LIB_TARGET)
	@mkdir -p $(@D) # Ensure directory for the target exists
	$(CC) $(CFLAGS) $(DEMO_MAIN_OBJ) -L$(BUILD_DIR) -ltiny3d $(LDFLAGS) -o $@
	@echo "Successfully built demo: $@"

# Rule to compile library source files into object files
# $< is the first prerequisite (the .c file)
# $@ is the target (the .o file)
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(INCLUDE_DIR)/canvas.h $(INCLUDE_DIR)/math3d.h $(INCLUDE_DIR)/renderer.h $(INCLUDE_DIR)/lighting.h $(INCLUDE_DIR)/animation.h $(INCLUDE_DIR)/obj_loader.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to compile the demo's main source file into an object file
$(DEMO_MAIN_OBJ): $(DEMO_MAIN_SRC) $(INCLUDE_DIR)/canvas.h $(INCLUDE_DIR)/renderer.h $(INCLUDE_DIR)/animation.h $(INCLUDE_DIR)/obj_loader.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $(DEMO_MAIN_SRC) -o $(DEMO_MAIN_OBJ)

# Create build directory if it doesn't exist (Order-only prerequisite)
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Test program for math library
TEST_MATH_SRC = $(TEST_DIR)/test_math.c
TEST_MATH_OBJ = $(BUILD_DIR)/test_math.o
TEST_MATH_TARGET = $(BUILD_DIR)/test_math

# Rule to build the math test program
$(TEST_MATH_TARGET): $(TEST_MATH_OBJ) $(LIB_TARGET)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(TEST_MATH_OBJ) -L$(BUILD_DIR) -ltiny3d $(LDFLAGS) -o $@
	@echo "Successfully built math test: $@"

# Rule to compile test_math.c into an object file
$(TEST_MATH_OBJ): $(TEST_MATH_SRC) $(INCLUDE_DIR)/math3d.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $(TEST_MATH_SRC) -o $(TEST_MATH_OBJ)


TEST_PIPELINE_SRC = $(TEST_DIR)/test_pipeline.c
TEST_PIPELINE_OBJ = $(BUILD_DIR)/test_pipeline.o
TEST_PIPELINE_TARGET = $(BUILD_DIR)/test_pipeline

# Rule to build the pipeline test program
$(TEST_PIPELINE_TARGET): $(TEST_PIPELINE_OBJ) $(LIB_TARGET)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(TEST_PIPELINE_OBJ) -L$(BUILD_DIR) -ltiny3d $(LDFLAGS) -o $@
	@echo "Successfully built pipeline test: $@"

# Rule to compile test_pipeline.c into an object file
$(TEST_PIPELINE_OBJ): $(TEST_PIPELINE_SRC) $(INCLUDE_DIR)/renderer.h | $(BUILD_DIR) # Depends on renderer.h
	$(CC) $(CFLAGS) -c $(TEST_PIPELINE_SRC) -o $(TEST_PIPELINE_OBJ)


# Phony targets
.PHONY: all clean run_demo run_test_math run_test_pipeline tests

# Target to build all tests
tests: $(TEST_MATH_TARGET) $(TEST_PIPELINE_TARGET)
TEST_TASK1_CLOCK_SRC = $(TEST_DIR)/test_task1_clock.c
TEST_TASK1_CLOCK_OBJ = $(BUILD_DIR)/test_task1_clock.o
TEST_TASK1_CLOCK_TARGET = $(BUILD_DIR)/test_task1_clock

# Rule to build the Task 1 clock test program
$(TEST_TASK1_CLOCK_TARGET): $(TEST_TASK1_CLOCK_OBJ) $(LIB_TARGET)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(TEST_TASK1_CLOCK_OBJ) -L$(BUILD_DIR) -ltiny3d $(LDFLAGS) -o $@
	@echo "Successfully built Task 1 clock test: $@"

# Rule to compile test_task1_clock.c into an object file
$(TEST_TASK1_CLOCK_OBJ): $(TEST_TASK1_CLOCK_SRC) $(INCLUDE_DIR)/canvas.h $(INCLUDE_DIR)/math3d.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $(TEST_TASK1_CLOCK_SRC) -o $(TEST_TASK1_CLOCK_OBJ)


# Phony targets
.PHONY: all clean run_demo run_test_math run_test_pipeline run_test_task1_clock tests

# Target to build all tests
tests: $(TEST_MATH_TARGET) $(TEST_PIPELINE_TARGET) $(TEST_TASK1_CLOCK_TARGET)
	@echo "All tests built."

# Target to run the demo
run_demo: $(DEMO_TARGET)
	./$(DEMO_TARGET)
	@echo "Demo executed. Check for demo_output.pgm"

# Target to run the math test
run_test_math: $(TEST_MATH_TARGET)
	./$(TEST_MATH_TARGET)
	@echo "Math test executed."

# Target to run the pipeline test
run_test_pipeline: $(TEST_PIPELINE_TARGET)
	./$(TEST_PIPELINE_TARGET)
	@echo "Pipeline test executed."

# Target to run the Task 1 clock test
run_test_task1_clock: $(TEST_TASK1_CLOCK_TARGET)
	./$(TEST_TASK1_CLOCK_TARGET)
	@echo "Task 1 clock test executed. Check for build/task1_clock_output.pgm"

# Target to clean build artifacts
clean:
	rm -rf $(BUILD_DIR)
	rm -f demo_output.pgm # Output of demo main.c
	rm -f demo_output.pgm # Old demo output name, frames are in build/
	rm -f build/task1_clock_output.pgm
	rm -f build/task2_math_cube_output.pgm
	@echo "Cleaned build artifacts."
