CC = clang++
SRC = Pthread.cpp
C_FLAGS = -O3 -lpthread -mavx -mavx2 -g  -Wall -Wextra  	    \
	-Wcast-align -Wcast-qual 	   					\
	-Wchar-subscripts -Wconversion        				\
	-Wempty-body -Wfloat-equal 		   						\
	-Wformat-nonliteral -Wformat-security       				\
	-Wformat=2 -Winline -Wlarger-than=8192            				\
	-Wmissing-declarations -Wopenmp 	   					\
	-Wpacked -Wpointer-arith -Wredundant-decls 				\
	-Wshadow -Wsign-conversion     				\
	-Wstrict-overflow=2 			   						\
  -Wswitch-default 	   				\
	-Wswitch-enum -Wundef -Wunreachable-code -Wunused   				\
	-Wvariadic-macros 	   					\
	-Wno-missing-field-initializers -Wno-narrowing 	           					\
	-Wno-varargs -fcheck-new 		   						\
	-fstack-check -fstack-protector            				\
	-fstrict-overflow 	   		   						\
	-fno-omit-frame-pointer                                         				\
	-fsanitize=address 	                                           				\
	-fsanitize=alignment                                            				\
	-fsanitize=bool                                                 				\
	-fsanitize=bounds                                               				\
	-fsanitize=enum                                                 				\
	-fsanitize=float-cast-overflow 	                           					\
	-fsanitize=float-divide-by-zero 			           							\
	-fsanitize=integer-divide-by-zero                               				\
	-fsanitize=leak 	                                           					\
	-fsanitize=nonnull-attribute                                    				\
	-fsanitize=null 	                                           					\
	-fsanitize=object-size                                          				\
	-fsanitize=return 		                                   					\
	-fsanitize=returns-nonnull-attribute                            				\
	-fsanitize=shift                                                				\
	-fsanitize=signed-integer-overflow                              				\
	-fsanitize=undefined                                            				\
	-fsanitize=unreachable                                          				\
	-fsanitize=vla-bound                                            				\
	-fsanitize=vptr                                                 				\

all:
	$(CC) $(C_FLAGS) $(SRC) -o task

clear:
	rm -rf *.o task
