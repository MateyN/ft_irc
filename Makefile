NAME			=	ircserv

SRC_DIR			=	src/
SRCS			=	main.cpp			\
					Server.cpp			\
					Client.cpp			\
					Channel.cpp			\
					Cmd.cpp				\
					Message.cpp

OBJ_DIR			= 	obj/
OBJS			=	$(addprefix $(OBJ_DIR), $(SRCS:.cpp=.opp))

CPP				=	c++
CPPFLAGS		=	-Wall -Wextra -Werror -g -fsanitize=address
STD98			=	-std=c++98
RM				=	rm -rf

all:				$(NAME)

$(OBJ_DIR)%.opp :	$(SRC_DIR)%.cpp
					@mkdir -p $(OBJ_DIR)
					$(CPP) -c $(CPPFLAGS) $(STD98) -I inc/ $< -o $@

tags:
					ctags --recurse=yes --exclude=.git --exclude=BUILD --exclude=.svn

$(NAME):			$(OBJS)	
					$(CPP) $(CPPFLAGS) $(STD98) $(OBJS) -o $(NAME)

clean:
					$(RM) $(OBJ_DIR)
					$(RM) $(OBJS)

fclean:				clean
					$(RM) $(NAME)

re:					fclean all

.PHONY:				all clean fclean re tags
