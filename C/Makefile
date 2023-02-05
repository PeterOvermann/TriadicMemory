

# make file for triadicmemory and dyadicmemory command line tools

all:
	cc -Ofast triadicmemoryCL.c 	triadicmemory.c 	-o /usr/local/bin/triadicmemory
	cc -Ofast dyadicmemoryCL.c  	triadicmemory.c 	-o /usr/local/bin/dyadicmemory
	
	cc -Ofast temporalmemory.c  	triadicmemory.c 	-o /usr/local/bin/temporalmemory
	cc -Ofast deeptemporalmemory.c  triadicmemory.c 	-o /usr/local/bin/deeptemporalmemory
	
	cc -Ofast dyadicmemorytest.c  	triadicmemory.c 	-o /usr/local/bin/dyadicmemorytest
	cc -Ofast triadicmemorytest.c  	triadicmemory.c 	-o /usr/local/bin/triadicmemorytest
	
