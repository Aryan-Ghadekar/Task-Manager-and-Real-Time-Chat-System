CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -pthread
SRCDIR = src
INCDIR = include
OBJDIR = obj
DATADIR = data

# Source files
SERVER_SOURCES = $(SRCDIR)/server.cpp $(SRCDIR)/TaskManager.cpp $(SRCDIR)/ChatManager.cpp $(SRCDIR)/Task.cpp $(SRCDIR)/Chat.cpp $(SRCDIR)/User.cpp $(SRCDIR)/NetworkUtils.cpp
CLIENT_SOURCES = $(SRCDIR)/client.cpp $(SRCDIR)/Task.cpp $(SRCDIR)/Chat.cpp $(SRCDIR)/User.cpp $(SRCDIR)/NetworkUtils.cpp

# Object files
SERVER_OBJECTS = $(SERVER_SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)
CLIENT_OBJECTS = $(CLIENT_SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

# Executables
SERVER_TARGET = server
CLIENT_TARGET = client

.PHONY: all clean setup server client

all: setup $(SERVER_TARGET) $(CLIENT_TARGET)

setup:
	@mkdir -p $(OBJDIR)
	@mkdir -p $(DATADIR)
	@touch $(DATADIR)/tasks.db
	@touch $(DATADIR)/chatlog.txt

$(SERVER_TARGET): $(SERVER_OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(CLIENT_TARGET): $(CLIENT_OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -I$(INCDIR) -c $< -o $@

server: $(SERVER_TARGET)

client: $(CLIENT_TARGET)

clean:
	rm -rf $(OBJDIR)
	rm -f $(SERVER_TARGET) $(CLIENT_TARGET)

run-server: $(SERVER_TARGET)
	./$(SERVER_TARGET)

run-client: $(CLIENT_TARGET)
	./$(CLIENT_TARGET)

# Development targets
debug: CXXFLAGS += -g -DDEBUG
debug: all

release: CXXFLAGS += -O2 -DNDEBUG
release: all