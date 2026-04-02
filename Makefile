CXX 		= 	c++
CXXFLAGS	= 	-Wall -Wextra -Werror -std=c++98 -g
NAME		= 	webserv

INCLUDES	= 	-I./inc

SRCS		= 	srcs/main.cpp \
				srcs/server/Webserv.cpp \
				srcs/server/Client.cpp \
				srcs/server/ParseConfig.cpp \
				srcs/server/ServerConfig.cpp \
				srcs/server/Server.cpp \
				srcs/server/Route.cpp \
				srcs/server/utils.cpp \
				srcs/server/Request.cpp \
				srcs/server/ParseRequest.cpp \
				srcs/cgi/CGI.cpp \
				srcs/cgi/CGIUtils.cpp \

OBJS		= 	$(SRCS:.cpp=.o)

%.o: %.cpp
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ -c $^

$(NAME): $(OBJS)
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(NAME) $(OBJS)

run: $(NAME)
	@./$(NAME)

clean:
	@rm -rf $(OBJS)

fclean: clean
	@rm -rf $(NAME)

re : fclean $(NAME)

.PHONY: clean fclean re