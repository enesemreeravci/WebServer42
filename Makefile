NAME = webserv
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

INCLUDES = -Iinclude -Iinclude/network

SRCS = src/main.cpp \
	   src/network/Server.cpp \
	   src/network/Client.cpp \
	   src/http/HttpRequest.cpp \
	   src/http/HttpParser.cpp \
	   src/cgi/CgiHandler.cpp \
	   src/utils/Utils.cpp

OBJS = $(SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)
	@echo "Compiled $(NAME) successfully"

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
