all : serveur client clean

serveur : serveur.o
	gcc -o serveur serveur.o

serveur.o : serveur.c
	gcc -c serveur.c -Wall

client : client.o
	gcc `sdl2-config --cflags --libs` -o client client.o

client.o : client.c
	gcc `sdl2-config --cflags --libs` -Wall -c client.c

clean:
	rm -rf *.o
