CXX = g++
CXXFLAGS = -Wall -Wextra -g

SRCS_TEST_SERVER = test_server.cpp server.cpp
SRCS_CLIENT = client.cpp

OBJS_TEST_SERVER = $(SRCS_TEST_SERVER:.cpp=.o)
OBJS_CLIENT = $(SRCS_CLIENT:.cpp=.o)

TEST_SERVER_TARGET = ./test/test_server
CLIENT_TARGET = ./test/client

.PHONY: all clean start_server start_client

all: test_server client

test_server: $(OBJS_TEST_SERVER)
	$(CXX) $(CXXFLAGS) $^ -o $(TEST_SERVER_TARGET) -lpthread

client: $(OBJS_CLIENT)
	$(CXX) $(CXXFLAGS) $^ -o $(CLIENT_TARGET)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# clean:
# 	rm -f $(OBJS_TEST_SERVER) $(OBJS_CLIENT) $(TEST_SERVER_TARGET) $(CLIENT_TARGET)
clean:
	rm -rf *.o ./test/*

start_server: test_server
	$(TEST_SERVER_TARGET)

start_client: client
	$(CLIENT_TARGET)
