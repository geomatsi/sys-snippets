#
# simple make file
#

all: compile link

debug: compile_debug link

link: 
	@for name in *.o ; do \
		echo ">>> Processing $$name"; \
		gcc -o  `basename $${name} .o` $${name}; \
	done

compile: *.c
	gcc -c $?

compile_debug: *.c
	gcc -c -g $?

clean: *.o
	@for name in $? ; do \
		echo ">>> Deleting $${name} `basename $${name} .o`"; \
		rm `basename $${name} .o`; \
		rm $${name}; \
	done
	
