CC=gcc
CFLAGS=-g -ggdb3 -Wall -std=gnu99

# httpserver 相关
LDFLAGS=-pthread -llog4c -lexpat

EXECUTABLES=httpserver
SOURCE=httpserver.c libhttp.c map.c cJSON.c

PROXY_EXECUTABLES=proxyserver

# test_httpserver 相关
TEST_LDFLAGS=-lcunit -lcurl

TEST_SOURCE=test_httpserver.c
TEST_EXECUTABLE=test_httpserver


all: $(EXECUTABLES) $(TEST_EXECUTABLE) $(PROXY_EXECUTABLES)

httpserver: $(SOURCE)
	$(CC) --static $(CFLAGS) $(SOURCE) -o $@ $(LDFLAGS)

$(TEST_EXECUTABLE): $(TEST_SOURCE)
	$(CC) $(CFLAGS) $(TEST_SOURCE) -o $@ $(TEST_LDFLAGS)

proxyserver: $(SOURCE)
	$(CC) --static $(CFLAGS) $(SOURCE) -o $@ $(LDFLAGS)

clean:
	rm -f $(EXECUTABLES) $(TEST_EXECUTABLE)