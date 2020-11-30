libbuddy.so: buddy.o
	gcc -shared -o libbuddy.so buddy.o

buddy.o: buddy.c
	gcc -c -fpic buddy.c -Wall -Werror

clean:
	rm buddy.o buddy libbuddy.so
