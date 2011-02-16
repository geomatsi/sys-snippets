#
# simple make file
#

all: compile link

link: compile
	@for name in *.o ; do \
		echo ">>> Processing $$name"; \
		gcc -o `basename $${name} .o` $${name}; \
	done

compile: *.c
	gcc -c $?

clean: *.o
	@for name in $? ; do \
		echo ">>> Deleting $${name} `basename $${name} .o`"; \
		rm `basename $${name} .o`; \
		rm $${name}; \
	done
	
