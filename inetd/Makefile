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

clean: 
	@for name in *.o ; do \
		if [ -r $${name} ] ; then \
			echo ">>> Deleting $${name} `basename $${name} .o`"; \
			rm `basename $${name} .o`; \
			rm $${name}; \
		fi;\
	done	
