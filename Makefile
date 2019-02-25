SOURCES=main.cpp  proxy.cpp request.cpp response.cpp cache.cpp
OBJS=$(patsubst %.cpp, %.o, $(SOURCES))
CPPFLAGS=-ggdb3 -Wall -Werror -pedantic -std=gnu++11

proxy: $(OBJS)
	g++ $(CPPFLAGS) -o proxy $(OBJS) -lpthread
%.o: %.cpp proxy.h request.h response.h cache.h
	g++ $(CPPFLAGS) -c $< -lpthread

clean:
	rm proxy *~ *.o
