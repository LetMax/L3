all : serveur client

serveur : serveurFONCTIONS.o serveur.o
	gcc -o serveur serveurFONCTIONS.o serveur.o

serveur.o : serveurFONCTIONS.o serveur.h
	gcc -c -Wall serveur.c

serveurFONCTIONS.o : serveurFONCTIONS.c
	gcc -c -Wall serveurFONCTIONS.c

client : clientFONCTIONS.o client.o
	gcc `sdl2-config --cflags --libs` -o client clientFONCTIONS.o client.o

client.o : clientFONCTIONS.o client.h
	gcc `sdl2-config --cflags --libs` -c -Wall client.c

clientFONCTIONS.o : clientFONCTIONS.c
	gcc `sdl2-config --cflags --libs` -c -Wall clientFONCTIONS.c

clean:
	rm -rf *.o
