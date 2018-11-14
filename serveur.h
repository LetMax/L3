#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#define h_addr h_addr_list[0]
#define BUFFSIZE 100          /* taille des buffers envoyés */
#define SOCKET_ERROR -1       /* code d'erreur des sockets */
#define MAX_CLIENT 10

struct Client{
	int give_mdp; //0 : ne doit pas encore le donner | 1 : le rentre pour la premiere fois | 2 : se connecte avec
  char pseudo[BUFFSIZE];
  int csock;
	char mdp[BUFFSIZE];
	int statut; //0 : libre | 1 : en attente d'un défi | 2 : entrain de jouer | 3 : mode spectateur
	struct Client *adversaire;
	struct Partie *partie_en_cours;
};

struct Partie{
  int damier[10][10]; // 0 : case vide | 1 : pion noir | 2 : dame noire | 3 : pion rouge | 4 : dame rouge
	//possède les pions rouges (joue en premier)
	struct Client joueurA;
	int pions_A;
	//possède les pions noirs (joue en deuxieme)
	struct Client joueurB;
	int pions_B;
	struct Client spec[8];
	int nb_spec;
};

//gestion des coups
struct Partie creer_partie(struct Client joueurA, struct Client joueurB);
void actualiser_pions_damier(int damier[10][10], int pions_A, int pions_B);
int coup(char buffer[BUFFSIZE], int damier[10][10], int num_joueur);
int remplissage_tab_coord(int tab_coord[20][2], char buffer[BUFFSIZE], int length);
int test_int_buffer(char buffer[BUFFSIZE], int n);
int deplacement_dame(int case_depart[2], int case_arrivee[2], int damier[10][10], int num_joueur, int cpt_deplacement, int cpt_prise);
int moyenne (int a, int b);

//gestion des damiers
void string_to_damier(char buffer[50], int damier[10][10]);
void damier_to_string(char buffer[50], int damier[10][10]);

//commandes à disposition des joueurs
int spectate(char buffer[BUFFSIZE]);
int defi(char buffer[BUFFSIZE]);
void liste_joueurs_i(struct Client tab[MAX_CLIENT], int nb_c, char result[1], int i);

//affichage automatique des joueurs
void afficher_clients(struct Client tab[MAX_CLIENT], int nb_c);
void liste_joueurs(struct Client tab[MAX_CLIENT], int nb_c, char result[1]);
char* inttos(char result[1], int j);

//gestion des connexions
int verif_pseudo(struct Client conserv[MAX_CLIENT], char pseudo[BUFFSIZE], int nb_c);
void ajouter_joueur_fichier(FILE* fichier, struct Client client);
int verif_pseudo_fichier(FILE* fichier, char pseudo[BUFFSIZE]);
int test_mdp(FILE* fichier, char pseudo[BUFFSIZE], char mdp[BUFFSIZE]);

//réseau
int listen_socket(int port);
struct Client add_client(int sock, int* max_fd, int* nb_c);
struct Client add_client_connect(struct Client client, int* max_fd, int* nb_c);
void rmv_client(struct Client* clients, int i_to_remove, int* nb_c);
void rmv_client_en_connect(struct Client* clients, int i_to_remove, int* nb_c);
int recv_client(int csock, char *buffer);
int send_client(int csock, char *buffer);
