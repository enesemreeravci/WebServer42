NAME = WebServer
CXX = c++ 
CXXFLAGS = -Wall -Wextra -Werror -std=c++98


SRCS = main.cpp #later we will change
OBJS = $(SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)
	@echo "Compiled $(NAME) succesfully"

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean $(NAME)

.PHONY: all clean fclean re