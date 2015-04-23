CC = g++

SRC_FOLDER = ./src/
INCLUDE_FOLDER = ./include/
OBJ_FOLDER = ./build/
DOC_FOLDER = ./doc

CC_FLAGS = -Wno-error -I$(INCLUDE_FOLDER) -std=c++11 -g -Wno-write-strings \
 					 -pedantic -Wall

SUPPORT_FILES = $(addprefix $(SRC_FOLDER), socket.cpp serverSocket.cpp \
																					 httpInterface.cpp clientSocket.cpp \
																					 tokenBucket.cpp)
SUPPORT_OBJS = $(addprefix $(OBJ_FOLDER), socket.o serverSocket.o \
																					httpInterface.o clientSocket.o \
																					tokenBucket.o)

.PHONY: all
.PHONY: doc
.PHONY: clean

all: client server

doc: $(SRC_FOLDER) $(INCLUDE_FOLDER) doxconfig
	@echo "Generating documentation..."
	@doxygen doxconfig
	@echo "Done!"

support: $(SUPPORT_FILES)
	@echo "Building support files..."
	@$(CC) -c $^ $(CC_FLAGS)
	@mv *.o $(OBJ_FOLDER)
	@echo "Done!"

client: $(SRC_FOLDER)clientMain.cpp support
	@echo "Building client code..."
	@$(CC) -o client $(SRC_FOLDER)clientMain.cpp $(SUPPORT_OBJS) $(CC_FLAGS)
	@echo "Done!"

server: $(SRC_FOLDER)serverMain.cpp support
	@echo "Building server code..."
	@$(CC) -o server $(SRC_FOLDER)serverMain.cpp $(SUPPORT_OBJS) $(CC_FLAGS)
	@echo "Done!"

clean:
	@echo "Cleaning files..."
	@rm client $(OBJ_FOLDER)*.o $(TARGET)
	@rm -rf $(DOC_FOLDER)
	@echo "Done!"
