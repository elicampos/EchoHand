# Define the C++ compiler
CXX = g++

# Define other compiler options
# Include paths: ./include for our headers, ./lib/raylib/include for Raylib
INCLUDES = -I ./include -I ./lib/raylib/include

# Library paths: ./lib/raylib/lib
LIBPATHS = -L ./lib/raylib/lib

# Libraries to link
LIBS = -lraylib -lopengl32 -lgdi32 -lwinmm -lws2_32 -lwsock32

# Source files
SRC = src/main.cpp src/glove.cpp src/serial.cpp src/socket.cpp

# Define the target executable name
TARGET = bin/opengloves_sim.exe

# Rule to build the final executable
$(TARGET): $(SRC)
	$(CXX) $(SRC) -o $(TARGET) $(INCLUDES) $(LIBPATHS) $(LIBS)
	
# Rule to run the program
run: $(TARGET) 
	./$(TARGET)

# Rule to clean up generated files
clean:
	if exist bin\opengloves_sim.exe del bin\opengloves_sim.exe