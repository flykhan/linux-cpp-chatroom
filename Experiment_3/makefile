CXX = g++
CXXFLAGS = -Wall -Wextra -g

SRCS_TEST_SERVER = test_server.cpp server.cpp
SRCS_TEST_CLIENT = test_client.cpp client.cpp 

OBJS_TEST_SERVER = $(SRCS_TEST_SERVER:.cpp=.o)
OBJS_TEST_CLIENT = $(SRCS_TEST_CLIENT:.cpp=.o)

TEST_SERVER_TARGET = ./test/test_server
TEST_CLIENT_TARGET = ./test/test_client

.PHONY: all clean start_server start_client

all: test_server test_client

test_server: $(OBJS_TEST_SERVER)
	$(CXX) $(CXXFLAGS) $^ -o $(TEST_SERVER_TARGET) -lpthread

test_client: $(OBJS_TEST_CLIENT)
	$(CXX) $(CXXFLAGS) $^ -o $(TEST_CLIENT_TARGET) -lpthread

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS_TEST_SERVER) $(OBJS_TEST_CLIENT) $(TEST_SERVER_TARGET) $(TEST_CLIENT_TARGET)

# clean:
# 	rm -rf *.o ./test/*

start_server: test_server
	$(TEST_SERVER_TARGET)

start_client: test_client
	$(TEST_CLIENT_TARGET)
