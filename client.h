#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <SDL2/SDL.h>
#include <math.h>

#define h_addr h_addr_list[0]
#define BUFFSIZE 100          /* taille des buffers envoyés */
#define SOCKET_ERROR -1       /* code d'erreur des sockets */
#define WINDOW_SIZE 500
#define LIBRE 0
#define EN_PARTIE 1
#define A 0
#define B 1

//fonctions pour la SDL
void actualiser_damier_SDL(SDL_Renderer *renderer, int damier[10][10], int x, int y);
void dessiner_ligne(SDL_Renderer *r, int x, int y, int w);
void dessiner_pion(SDL_Renderer *r, int cx, int cy, int rayon);
void dessiner_dame(SDL_Renderer *r, int cx, int cy, int rayon);

//gestion des damiers
void creer_damier(int damier[10][10]);
void string_to_damier(char buffer[50], int damier[10][10]);
void damier_to_string(char buffer[50], int damier[10][10]);
void damiercpy(int damier_tmp[10][10], int damier[10][10]);

//gestion des coups
int coup(char buffer[BUFFSIZE], int damier[10][10], int num_joueur);
int remplissage_tab_coord(int tab_coord[20][2], char buffer[BUFFSIZE], int length);
int test_int_buffer(char buffer[BUFFSIZE], int n);
int deplacement_dame(int case_depart[2], int case_arrivee[2], int damier[10][10], int num_joueur, int cpt_deplacement, int cpt_prise);
int moyenne (int a, int b);

//réseau
int connect_socket(char* adresse, int port);
int recv_server(int sock, char *buffer);
int send_server(int sock, char *buffer);
