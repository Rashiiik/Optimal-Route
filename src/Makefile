CC = gcc
CFLAGS = -Wall -Wextra -O2 -lm

# All source files in current directory
SOURCES = *.c
TARGET = main

# Build executable
all:
	$(CC) $(SOURCES) -o $(TARGET) $(CFLAGS)
	@echo "Build complete!"

# Clean
clean:
	rm -f $(TARGET) ../*.kml
	@echo "Clean complete"

# Run program
run: all
	cd .. && ./main

# Run with test input
test: all
	cd .. && ./main < test.txt