#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <regex.h>

#define h_addr h_addr_list[0]
#define BUFFSIZE 100          /* taille des buffers envoyés */
#define SOCKET_ERROR -1       /* code d'erreur des sockets */
#define MAX_CLIENT 10

struct Client{
	int give_mdp; //0 : ne doit pas encore le donner | 1 : le rentre pour la premiere fois | 2 : se connecte avec
  char pseudo[BUFFSIZE];
  int csock;
	char mdp[BUFFSIZE];
	int statut; //0 : libre | 1 : en attente d'un défi | 2 : entrain de jouer
	int tour; //1 : le joueur a la main | 2 : l'adversaire a la main
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
};

void damier_to_string(char buffer[50], int damier[10][10]);
struct Partie creer_partie(struct Client joueurA, struct Client joueurB);
int verif_pseudo(struct Client conserv[MAX_CLIENT], char pseudo[BUFFSIZE], int nb_c);
int defi(char buffer[BUFFSIZE]);
int match_str_reg(const char *string, const char *pattern);
char* inttos(char result[1], int j);
void afficher_clients(struct Client tab[MAX_CLIENT], int nb_c);
int listen_socket(int port);
struct Client add_client(int sock, int* max_fd, int* nb_c);
struct Client add_client_connect(struct Client client, int* max_fd, int* nb_c);
void rmv_client(struct Client* clients, int i_to_remove, int* nb_c);
void rmv_client_en_connect(struct Client* clients, int i_to_remove, int* nb_c);
int recv_client(int csock, char *buffer);
int send_client(int csock, char *buffer);
void infos_client(struct Client c);
void liste_joueurs(struct Client tab[MAX_CLIENT], int nb_c, char result[1]);
void ajouter_joueur_fichier(FILE* fichier, struct Client client);
int verif_pseudo_fichier(FILE* fichier, char pseudo[BUFFSIZE]);
int test_mdp(FILE* fichier, char pseudo[BUFFSIZE], char mdp[BUFFSIZE]);

int main(int argc, char* argv[]){

	if(argc != 2) {
       printf("L'appel doit etre de type %s port\n",argv[0]);
       exit(-1);
   	}

	int port = atoi(argv[1]);

	int sock = listen_socket(port);

	//tableau des joueurs connectés
	struct Client conserv[MAX_CLIENT];
	int nb_c = 0;
	int max_fd = sock;

	//tableau des joeurs qui se connectent
	struct Client conserv_bis[MAX_CLIENT];
	int nb_cc = 0;

	//variable qui sert à recuperer la partie à traiter
	struct Partie partie_tmp;

	fd_set readfds;

	int i, j, r;
	char result[1];

	char buffer[BUFFSIZE];
	char msgadieu[BUFFSIZE] = " quitte ce lieu maudit ***\n";
	char msgbienvenue[BUFFSIZE] = "*** Bienvenue dans le game : ";
	int joueur_d;

	FILE* fichier = NULL;

	while(1) {

		for(i = 0; i < BUFFSIZE; i++){
			buffer[i] = '\0';
		}

		FD_ZERO(&readfds);
		FD_SET(sock, &readfds);
		FD_SET(STDIN_FILENO, &readfds);

		for(i = 0; i < nb_c; i++){
    		FD_SET(conserv[i].csock, &readfds);
    	}

		for(i = 0; i < nb_cc; i++){
    		FD_SET(conserv_bis[i].csock, &readfds);
    	}

    	int s = select(max_fd+1, &readfds,NULL,NULL,NULL);
    	if(s==-1){
    		perror("select");
    		exit(-1);
    	}

		if(FD_ISSET(STDIN_FILENO, &readfds)){
			scanf("%s", buffer);
			buffer[strlen(buffer)]='\0';
			send_client(sock, buffer);
		}

    	//connexion d'un client
    	if(FD_ISSET(sock, &readfds)){
    		//ajout du client
    		conserv_bis[nb_cc] = add_client(sock, &max_fd, &nb_cc);

			send_client(conserv_bis[nb_cc-1].csock, "*** Entrez 0 pour créer un compte, 1 pour vous connecter, 2 pour entrer en tant que guest ***\n");
		}

		for(i = 0; i < nb_cc; i++){
			//joueur entrain de se connecter
			if(FD_ISSET(conserv_bis[i].csock, &readfds)){
		  		r = recv_client(conserv_bis[i].csock, buffer);
				if(r!=0){

					if(conserv_bis[i].give_mdp == 0){

						//création du compte
						if(strcmp(buffer, "0") == 0){
							//test de la validité du pseudo
							if(verif_pseudo_fichier(fichier, conserv_bis[i].pseudo) == 1 || verif_pseudo(conserv_bis, conserv_bis[i].pseudo, nb_c) > 1 || verif_pseudo(conserv, conserv_bis[i].pseudo, nb_c) > 0){
								send_client(conserv_bis[i].csock, "Ce pseudo est deja utilisé\n");
								send_client(conserv_bis[i].csock, "au revoir\n");
								rmv_client(conserv_bis, i, &nb_cc);
							}
							else{
								conserv_bis[i].give_mdp = 1;
								send_client(conserv_bis[i].csock, "*** Entrez un mot passe (souvenez-vous en) ***\n");
							}
						}
						//connexion
						else if(strcmp(buffer, "1") == 0){
							//test si le pseudo apparait 1 fois dans le fichier des comptes
							if(verif_pseudo_fichier(fichier, conserv_bis[i].pseudo) != 1){
								send_client(conserv_bis[i].csock, "*** Vous n'avez pas de compte ***\n");
								send_client(conserv_bis[i].csock, "*** Entrez 0 pour créer un compte ou 2 pour entrer en tant que guest ***\n");
							}
							//test si ce pseudo apparait dans les joueurs connectés ou entrain
							else if(verif_pseudo(conserv_bis, conserv_bis[i].pseudo, nb_c) > 1 || verif_pseudo(conserv, conserv_bis[i].pseudo, nb_c) > 0){
								send_client(conserv_bis[i].csock, "*** Vous etes, soit un double maléfique soit mal intentionné ***\n");
								send_client(conserv_bis[i].csock, "au revoir\n");
							}
							else{
								conserv_bis[i].give_mdp = 2;
								send_client(conserv_bis[i].csock, "*** Entrez votre mot passe ***\n");
							}
						}
						//guest
						else if(strcmp(buffer, "2") == 0){
							//test de la validité du pseudo
							if(verif_pseudo_fichier(fichier, conserv_bis[i].pseudo) == 1 || verif_pseudo(conserv_bis, conserv_bis[i].pseudo, nb_c) > 1 || verif_pseudo(conserv, conserv_bis[i].pseudo, nb_c) > 0){
								send_client(conserv_bis[i].csock, "Ce pseudo est deja utilisé\n");
								send_client(conserv_bis[i].csock, "au revoir\n");
								rmv_client(conserv_bis, i, &nb_cc);
							}
							else{

								printf("%s est arrivé dans le game !\n", conserv_bis[i].pseudo);
								//message de bienvenue
								send_client(conserv_bis[i].csock, msgbienvenue);
								send_client(conserv_bis[i].csock, conserv_bis[i].pseudo);
								send_client(conserv_bis[i].csock, " ***\n");

								conserv[nb_c] = add_client_connect(conserv_bis[i], &max_fd, &nb_c);
								rmv_client_en_connect(conserv_bis, i, &nb_cc);

								//on actualise la liste des joueurs et on la montre à tout le monde
								afficher_clients(conserv, nb_c);
								liste_joueurs(conserv, nb_c, result);
							}
						}
						else{
							send_client(conserv_bis[i].csock, "*** 0, 1 ou 2 (pas tres complexe ...) ***\n");
						}
					}
					//joueur qui donne son mdp pour la premiere fois
					else if(conserv_bis[i].give_mdp == 1){

						strcpy(conserv_bis[i].mdp, buffer);

						printf("%s est arrivé dans le game !\n", conserv_bis[i].pseudo);

						//message de bienvenue
						send_client(conserv_bis[i].csock, msgbienvenue);
						send_client(conserv_bis[i].csock, conserv_bis[i].pseudo);
						send_client(conserv_bis[i].csock, " ***\n");

						conserv[nb_c] = add_client_connect(conserv_bis[i], &max_fd, &nb_c);
						rmv_client_en_connect(conserv_bis, i, &nb_cc);

						//on actualise la liste des joueurs et on la montre à tout le monde
						afficher_clients(conserv, nb_c);
						liste_joueurs(conserv, nb_c, result);

						ajouter_joueur_fichier(fichier, conserv[nb_c-1]);
					}
					//joueur qui se connecte avec son mdp
					else if(conserv_bis[i].give_mdp == 2){
						//mdp incorrect
						if(test_mdp(fichier, conserv_bis[i].pseudo, buffer) == 0){
							send_client(conserv_bis[i].csock, "*** Mot de passe incorrect, réessayez ***\n");
						}
						//mdp correct
						else{
							printf("%s est revenu dans le game !\n", conserv_bis[i].pseudo);

							//message de bienvenue
							send_client(conserv_bis[i].csock, "*** Re-bienvenue ");
							send_client(conserv_bis[i].csock, conserv_bis[i].pseudo);
							send_client(conserv_bis[i].csock, " ***\n");

							conserv[nb_c] = add_client_connect(conserv_bis[i], &max_fd, &nb_c);
							rmv_client_en_connect(conserv_bis, i, &nb_cc);

							//on actualise la liste des joueurs et on la montre à tout le monde
							afficher_clients(conserv, nb_c);
							liste_joueurs(conserv, nb_c, result);
						}
					}
				}
				else{
					printf("%s ne s'est finalement pas connecté\n", conserv_bis[i].pseudo);
					rmv_client(conserv_bis, i, &nb_cc);
				}
			}
		}

		for(i = 0; i < nb_c; i++){
			//joueur connecté
			if(FD_ISSET(conserv[i].csock, &readfds)){
				r = recv_client(conserv[i].csock, buffer);
				if(r!=0){
					//message d'un joueur libre
					if(conserv[i].statut == 0){

						//un joueur veut un défi
						if((defi(buffer)) == 0){
							joueur_d = 0;
							joueur_d = buffer[8] - '0';
							if(joueur_d >= 0 && joueur_d < nb_c){

								//On test si le joueur défié a un adversaire
								if(conserv[joueur_d].adversaire != NULL){
									send_client(conserv[i].csock, "*** Vous défiez un joueur qui a deja un adversaire ***\n");
								}

								//On test si le joueur qui défi a un adversaire
								else if(conserv[i].adversaire != NULL){
									send_client(conserv[i].csock, "*** Vous avez deja un adversaire ***\n");
								}

								//On test si il se défie lui meme
								else if(strcmp(conserv[i].pseudo, conserv[joueur_d].pseudo) == 0){
									printf("%s se défie lui meme\n", conserv[i].pseudo);
									send_client(conserv[i].csock, "*** vous vous defiez vous meme ... ***\n");
								}

								//la demande peut se faire
								else {

									//on met à jour le demandeur de défi
									conserv[i].adversaire = &conserv[joueur_d];

									//on met à jour le joueur demandé en défi
									conserv[joueur_d].adversaire = &conserv[i];
									conserv[joueur_d].statut = 1;

									//message au serveur
									printf("%s veut defier %s !\n", conserv[i].pseudo, conserv[joueur_d].pseudo);

									//le joueur lanceur du défi attends la réponse de l'autre
									send_client(conserv[i].csock, "*** Vous défiez ");
									send_client(conserv[i].csock, conserv[joueur_d].pseudo);
									send_client(conserv[i].csock, " , en attente de sa réponse ... ***\n");

									//on demande au joueur défié s'il accepte
									send_client(conserv[joueur_d].csock, "*** ");
									send_client(conserv[joueur_d].csock, conserv[i].pseudo);
									send_client(conserv[joueur_d].csock, " souhaite vous exterminer ! (/accept|/refuse) ***\n");

									//on prévient de la demande d'affrontement aux autres
									for(j = 0; j < nb_c; j++){
										if(strcmp(conserv[j].pseudo,conserv[i].pseudo) != 0 && strcmp(conserv[j].pseudo,conserv[joueur_d].pseudo) != 0 ){
											send_client(conserv[j].csock, "*** ");
											send_client(conserv[j].csock, conserv[i].pseudo);
											send_client(conserv[j].csock, " souhaite torpiller ");
											send_client(conserv[j].csock, conserv[joueur_d].pseudo);
											send_client(conserv[j].csock, " ! ***\n");
										}
									}
								}
							}
							else{
								send_client(conserv[i].csock, "*** Vous défiez un fantome (personne en gros) ***\n");
							}
						}
						else {
							//message d'un joueur dans le chat
							for(j=0;j<nb_c;j++){
								if(strcmp(conserv[j].pseudo, conserv[i].pseudo) != 0 && conserv[j].statut != 2){
									send_client(conserv[j].csock, conserv[i].pseudo);
									send_client(conserv[j].csock, " : ");
									send_client(conserv[j].csock, buffer);
									send_client(conserv[j].csock, "\n");
								}
							}
						}
					}

					//message d'un joueur qui doit répondre à une demande de défi
					else if(conserv[i].statut == 1){

						//défi accepté
						if(strcmp(buffer, "/accept") == 0){

							for(j = 0; j < nb_c; j++){
								//on prévient les autres de l'affrontement
								if(strcmp(conserv[i].pseudo, conserv[j].pseudo) != 0 && strcmp(conserv[i].adversaire->pseudo, conserv[j].pseudo) != 0 ){
									send_client(conserv[j].csock, "*** ");
									send_client(conserv[j].csock, conserv[i].pseudo);
									send_client(conserv[j].csock, " et ");
									send_client(conserv[j].csock, conserv[i].adversaire->pseudo);
									send_client(conserv[j].csock, " vont pouvoir se la donner severe ! ***\n");
								}
							}

							for(j = 0; j < nb_c; j++){

								//on retrouve le demandeur du défi
								if(strcmp(conserv[i].adversaire->pseudo, conserv[j].pseudo) == 0){

									//message au serveur
									printf("%s accepte le defi de %s !\n", conserv[i].pseudo, conserv[j].pseudo);

									//message de début de partie
									//joueur qui a été défié, joueur A
									send_client(conserv[i].csock, "*** La partie commence : tu joues les pions rouges  ***\n");

									// joueur à l'origine du défi, joueur B
									send_client(conserv[j].csock, "*** La partie commence : tu joues les pions noirs  ***\n");

									//initialisation de la partie
									partie_tmp = creer_partie(conserv[i], conserv[j]);

									conserv[i].partie_en_cours = &partie_tmp;
									conserv[i].tour = 1;

									conserv[j].partie_en_cours = &partie_tmp;
									conserv[j].tour = 2;

									//Les deux joueurs passent "en partie"
									conserv[i].statut = 2;
									conserv[j].statut = 2;

									break;
								}
							}
						}

						//défi refusé
						else if(strcmp(buffer, "/refuse") == 0){

							for(j = 0; j < nb_c; j++){
								//on prévient les autres
								if(strcmp(conserv[i].pseudo, conserv[j].pseudo) != 0 && strcmp(conserv[i].adversaire->pseudo, conserv[j].pseudo) != 0 ){
									send_client(conserv[j].csock, "*** ");
									send_client(conserv[j].csock, conserv[i].pseudo);
									send_client(conserv[j].csock, " s'est lachement enfui devant ");
									send_client(conserv[j].csock, conserv[i].adversaire->pseudo);
									send_client(conserv[j].csock, " ***\n");
								}
							}

							for(j = 0; j < nb_c; j++){

								//messages aux joueurs impliqués dans le conflit
								if(strcmp(conserv[i].adversaire->pseudo, conserv[j].pseudo) == 0){

									printf("%s refuse le defi de %s !\n", conserv[i].pseudo, conserv[j].pseudo);

									send_client(conserv[j].csock, "*** ");
									send_client(conserv[j].csock, conserv[i].pseudo);
									send_client(conserv[j].csock, " a surement eu peur ... ***\n");

									send_client(conserv[i].csock, "*** ");
									send_client(conserv[i].csock, "On appelle ca se defiler .. ***\n");

									conserv[i].adversaire = NULL;
									conserv[i].statut = 0;

									conserv[j].adversaire = NULL;
									break;
								}
							}
						}
						else {
							//message normal du joeur joueur dans le chat
							for(j=0;j<nb_c;j++){
								if(conserv[j].pseudo!=conserv[i].pseudo && conserv[j].statut != 2){
									send_client(conserv[j].csock, conserv[i].pseudo);
									send_client(conserv[j].csock, " : ");
									send_client(conserv[j].csock, buffer);
									send_client(conserv[j].csock, "\n");
								}
							}

							//On lui rappelle qu'il doit répondre à une demande de défi
							send_client(conserv[i].csock, "*** RAPPEL : ");
							//joueur à l'origine du défi
							for(j = 0; j < nb_c; j++){
								if(strcmp(conserv[i].adversaire->pseudo, conserv[j].pseudo) == 0){
									send_client(conserv[i].csock, conserv[j].pseudo);
								}
							}
							send_client(conserv[i].csock, " attends une reponse ! envoyez /accept ou /refuse ***\n");
						}
					}

					//joueur entrain de jouer
					else if(conserv[i].statut == 2){

						struct Partie* partie_tmp2;
						partie_tmp2 = conserv[i].partie_en_cours;

						char damier_string[50];


						//abandon du joueur
						if(strcmp(buffer, "/ff") == 0){
							if(strcmp(partie_tmp2->joueurA.pseudo, conserv[i].pseudo) == 0){
								partie_tmp2->pions_A = 0;
							}
							else{
								partie_tmp2->pions_B = 0;
							}
						}

						//test si c'est son tour
						else if(conserv[i].tour == 2){
							send_client(conserv[i].csock, "*** Ce n'est pas tour, un peu de patience ! ***\n");
						}

						/* COUP DU JOUEUR */
						else{




							//coup valide
							send_client(conserv[i].csock, "***Coup valide***\n");
							conserv[i].partie_en_cours = partie_tmp2;

							//on prévient l'adversaire
							for(j = 0; j < nb_c; j++){
								if(strcmp(conserv[i].adversaire->pseudo, conserv[j].pseudo) == 0){
									send_client(conserv[j].csock, "***A vous de jouer***\n");
									conserv[j].partie_en_cours = partie_tmp2;
									damier_to_string(damier_string, conserv[j].partie_en_cours->damier);
									send_client(conserv[j].csock, damier_string);
								}
							}

							//Fin du tour
							conserv[i].tour = 2;
							for(j = 0; j < nb_c; j++){
								if(strcmp(conserv[i].adversaire->pseudo, conserv[j].pseudo) == 0){
									conserv[j].tour = 1;
								}
							}
						}

						/* TEST SI LA PARTIE EST FINIE */
						if(partie_tmp2->pions_A <= 0){
							//le joueur A a perdu
							for(j = 0; j < nb_c; j++){
								//on prévient les autres
								if(strcmp(conserv[i].pseudo, conserv[j].pseudo) != 0 && strcmp(conserv[i].adversaire->pseudo, conserv[j].pseudo) != 0 ){
									if(strcmp(conserv[i].pseudo, partie_tmp2->joueurA.pseudo) != 0){
										send_client(conserv[j].csock, "*** ");
										send_client(conserv[j].csock, conserv[i].pseudo);
										send_client(conserv[j].csock, " a atomisé ");
										send_client(conserv[j].csock, conserv[i].adversaire->pseudo);
										send_client(conserv[j].csock, " ! ***\n");
									}
									else{
										send_client(conserv[j].csock, "*** ");
										send_client(conserv[j].csock, conserv[i].adversaire->pseudo);
										send_client(conserv[j].csock, " a atomisé ");
										send_client(conserv[j].csock, conserv[i].pseudo);
										send_client(conserv[j].csock, " ! ***\n");
									}
								}
							}

							for(j = 0; j < nb_c; j++){
								if(strcmp(conserv[i].adversaire->pseudo, conserv[j].pseudo) == 0){

									printf("la partie entre %s et %s est terminée\n", conserv[i].pseudo, conserv[j].pseudo);

									if(strcmp(conserv[i].pseudo, partie_tmp2->joueurA.pseudo)){
										//message de fin de partie
										send_client(conserv[i].csock, "*** ");
										send_client(conserv[i].csock, "Tu vois quand tu veux, t'es pas si mauvais !");
										send_client(conserv[i].csock, " ***\n");

										send_client(conserv[j].csock, "*** ");
										send_client(conserv[j].csock, "Bon, on va rien dire ... (loser)");
										send_client(conserv[j].csock, " ***\n");
									}

									else{
										//message de fin de partie
										send_client(conserv[j].csock, "*** ");
										send_client(conserv[j].csock, "Tu vois quand tu veux, t'es pas si mauvais !");
										send_client(conserv[j].csock, " ***\n");

										send_client(conserv[i].csock, "*** ");
										send_client(conserv[i].csock, "Bon, on va rien dire ... (loser)");
										send_client(conserv[i].csock, " ***\n");
									}

									conserv[i].adversaire = NULL;
									conserv[j].adversaire = NULL;

									conserv[i].partie_en_cours = NULL;
									conserv[j].partie_en_cours = NULL;

									conserv[i].statut = 0;
									conserv[j].statut = 0;
									break;
								}
							}
						}
						else if(partie_tmp2->pions_B <= 0){
							//le joueur B a perdu
							for(j = 0; j < nb_c; j++){
								//on prévient les autres
								if(strcmp(conserv[i].pseudo, conserv[j].pseudo) != 0 && strcmp(conserv[i].adversaire->pseudo, conserv[j].pseudo) != 0 ){
									if(strcmp(conserv[i].pseudo, partie_tmp2->joueurB.pseudo) != 0){
										send_client(conserv[j].csock, "*** ");
										send_client(conserv[j].csock, conserv[i].pseudo);
										send_client(conserv[j].csock, " a atomisé ");
										send_client(conserv[j].csock, conserv[i].adversaire->pseudo);
										send_client(conserv[j].csock, " ! ***\n");
									}
									else{
										send_client(conserv[j].csock, "*** ");
										send_client(conserv[j].csock, conserv[i].adversaire->pseudo);
										send_client(conserv[j].csock, " a atomisé ");
										send_client(conserv[j].csock, conserv[i].pseudo);
										send_client(conserv[j].csock, " ! ***\n");
									}
								}
							}

							for(j = 0; j < nb_c; j++){

								if(strcmp(conserv[i].adversaire->pseudo, conserv[j].pseudo) == 0){

									printf("la partie entre %s et %s est terminée\n", conserv[i].pseudo, conserv[j].pseudo);

									if(strcmp(conserv[i].pseudo, partie_tmp2->joueurA.pseudo)){
										//message de fin de partie
										send_client(conserv[j].csock, "*** ");
										send_client(conserv[j].csock, "Tu vois quand tu veux, t'es pas si mauvais !");
										send_client(conserv[j].csock, " ***\n");

										send_client(conserv[i].csock, "*** ");
										send_client(conserv[i].csock, "Bon, on va rien dire ... (loser)");
										send_client(conserv[i].csock, " ***\n");
									}

									else{
										//message de fin de partie
										send_client(conserv[i].csock, "*** ");
										send_client(conserv[i].csock, "Tu vois quand tu veux, t'es pas si mauvais !");
										send_client(conserv[i].csock, " ***\n");

										send_client(conserv[j].csock, "*** ");
										send_client(conserv[j].csock, "Bon, on va rien dire ... (loser)");
										send_client(conserv[j].csock, " ***\n");
									}

									conserv[i].adversaire = NULL;
									conserv[j].adversaire = NULL;

									conserv[i].partie_en_cours = NULL;
									conserv[j].partie_en_cours = NULL;

									conserv[i].statut = 0;
									conserv[j].statut = 0;
									break;
								}
							}
						}
					}
				}
				else{
					printf("%s s'en va (sage décision)\n", conserv[i].pseudo);

					for(j=0;j<nb_c;j++){
						if(conserv[j].pseudo!=conserv[i].pseudo ){
							send_client(conserv[j].csock, "*** ");
							send_client(conserv[j].csock, conserv[i].pseudo);
							send_client(conserv[j].csock, msgadieu);
						}
					}

					rmv_client(conserv, i, &nb_c);
					afficher_clients(conserv, nb_c);

					//on actualise la liste des joueurs et on la montre à tout le monde
					liste_joueurs(conserv, nb_c, result);
				}
			}
		}
	}
	return 0;
}

void damier_to_string(char buffer[50], int damier[10][10]){
	int i, j;
	char tmp[1];

	for(i = 0; i < 10; i++){
		for(j = 0; j < 10; j++){
			//cases jouables
			if(((i + j) % 2) == 1){
				strcat(buffer, inttos(tmp, damier[i][j]));
			}
		}
	}
	buffer[50] = '\0';
}

struct Partie creer_partie(struct Client joueurA, struct Client joueurB){

	int i, j;

	struct Partie partie;

	partie.joueurA = joueurA;
	partie.joueurB = joueurB;
	partie.pions_A = 20;
	partie.pions_B = 20;

	for(i = 0; i < 10; i++){
		for(j = 0; j < 10; j++){
			//cases jouables
			if(((i + j) % 2) == 1){
				//joueur A (en haut du damier, pions noirs)
				if(i < 4){
					partie.damier[i][j] = 1;
				}
				//cases vides du milieu
				else if(i < 6){
					partie.damier[i][j] = 0;
				}
				//joueur B (en bas du damier, pions rouges)
				else {
					partie.damier[i][j] = 3;
				}
			}
		}
	}


	return partie;
}

int verif_pseudo(struct Client conserv[MAX_CLIENT], char pseudo[BUFFSIZE], int nb_c){

	int cpt = 0;

	int i;
	for(i = 0; i < nb_c; i++){
		if(strcmp(conserv[i].pseudo, pseudo) == 0){
			cpt++;
		}
	}

	return cpt;
}

int defi(char buffer[BUFFSIZE]){
	if(buffer[0] == '/' && buffer[1] == 'b' && buffer[2] == 'a' && buffer[3] == 't' && buffer[4] == 't' && buffer[5] == 'l' && buffer[6] == 'e'){
		return 0;
	}
	return 1;
}

int match_str_reg(const char *string, const char *pattern){
    regex_t re;
    if (regcomp(&re, pattern, REG_EXTENDED|REG_NOSUB) != 0) return 0;
    int status = regexec(&re, string, 0, NULL, 0);
    regfree(&re);
    if (status != 0) return 0;
    return 1;
}

char* inttos(char result[1], int j){
	sprintf(result, "%d", j);
	return result;
}


void afficher_clients(struct Client tab[MAX_CLIENT], int nb_c){

	printf("--- Clients connectés ---\n");
	int i;
	for(i = 0; i < nb_c; i++){
		printf("joueur %d : %s\n", i, tab[i].pseudo);
	}
	printf("-------------------------\n");
}

int listen_socket(int port){

	int sock = socket(AF_INET, SOCK_STREAM, 0); /* créé un socket TCP */
     if(sock==SOCKET_ERROR){
          perror("Erreur socket");
          exit(-1);
     }
	struct sockaddr_in sin;
	sin.sin_addr.s_addr = htonl(INADDR_ANY); /* on accepte toute adresse */
	sin.sin_port = htons(port);
	sin.sin_family = AF_INET;
	int b = bind (sock, (struct sockaddr*) &sin, sizeof(sin)); /* on lie le socket à sin */
	if(b==-1){
		perror("Erreur bind");
		exit(-1);
	}
	int l = listen(sock, 1); /* notre socket est prêt à écouter une connection */
		if(l==-1){
			perror("Erreur listen");
			exit(-1);
		}
	return sock;
}

struct Client add_client(int sock, int* max_fd, int* nb_c){

	struct Client c;

	struct sockaddr_in csin;
	unsigned int csinsize=sizeof(csin);
	int csock = accept(sock, (struct sockaddr *) &csin, &csinsize); /* accepter un client */
	if (csock==-1){
		perror("Erreur accept");
		exit(-1);
	}

	c.csock = csock;
	c.statut = 0;
	c.adversaire = NULL;
	c.tour = 0;
	c.give_mdp = 0;

	recv_client(csock, c.pseudo);

	(*nb_c)++;
	if (*max_fd<csock) *max_fd=csock;
	return c;
}

struct Client add_client_connect(struct Client client, int* max_fd, int* nb_c){
	(*nb_c)++;
	if (*max_fd<client.csock) *max_fd=client.csock;
	return client;
}

void rmv_client(struct Client* clients, int i_to_remove, int* nb_c){
	int k;
	close(clients[i_to_remove].csock);
	for(k=i_to_remove;k<*nb_c;k++){
		clients[k]=clients[k+1];
	}
	(*nb_c)--;
}

void rmv_client_en_connect(struct Client* clients, int i_to_remove, int* nb_c){
	int k;
	for(k=i_to_remove;k<*nb_c;k++){
		clients[k]=clients[k+1];
	}
	(*nb_c)--;
}

int recv_client(int csock, char *buffer){
	int r = recv(csock, buffer, BUFFSIZE, 0);
	if(r==-1){
		perror("Erreur receive");
		exit(-1);
	}
	return r;
}

int send_client(int csock, char *buffer){
	int s = send(csock, buffer, BUFFSIZE, 0);
	if (s==-1){
		perror("Erreur send");
		exit(-1);
	}
	return s;
}

void infos_client(struct Client c){
	printf("--- %s ---\n", c.pseudo);
	printf("sock : %d\n", c.csock);
}

void liste_joueurs(struct Client tab[MAX_CLIENT], int nb_c, char result[1]){

	int k, j;

	for(k = 0; k < nb_c; k++){
		if(tab[k].statut != 2){
			send_client(tab[k].csock, "--- Joueurs connectés ---\n");
			for(j = 0; j < nb_c; j++){
				send_client(tab[k].csock, "joueur ");
				send_client(tab[k].csock, inttos(result,j));
				send_client(tab[k].csock, " : ");
				send_client(tab[k].csock, tab[j].pseudo);
				if (tab[k].pseudo == tab[j].pseudo) send_client(tab[k].csock, " (vous)");
				if (tab[j].statut == 2) send_client(tab[k].csock, "(en jeu)");
				send_client(tab[k].csock, "\n");
			}
			send_client(tab[k].csock, "-------------------------\n");
		}
	}
}

void ajouter_joueur_fichier(FILE* fichier, struct Client client){

	fichier = fopen("comptes.txt", "a");
	fprintf(fichier, "\n%s\n%s\n", client.pseudo, client.mdp);
	fclose(fichier);
}

int verif_pseudo_fichier(FILE* fichier, char pseudo[BUFFSIZE]){
	//printf("\n--- Début verif_pseudo_fichier ---\n");
	int i;

	char pseudo_test[BUFFSIZE];
	int cpt = 0;

	fichier = fopen("comptes.txt", "r");

	while(fgets(pseudo_test, BUFFSIZE, fichier) != NULL){
		//on enleve le retour chariot de fin de ligne
		for(i = 0; i < BUFFSIZE; i++){
			if(pseudo_test[i] == '\n'){
				pseudo_test[i] = '\0';
				break;
			}
		}
		if(strcmp(pseudo_test, pseudo) == 0){
			cpt = 1;
			break;
		}
		//printf("Comparaison PSEUDO : |%s|(%d) et |%s|(%d)\n", pseudo, strlen(pseudo), pseudo_test, strlen(pseudo_test));
	}

	if(cpt == 1){
		//printf("\n--- FIN verif_pseudo_fichier, on a trouvé ---\n");
		fclose(fichier);
		return cpt;
	}
	//printf("\n--- FIN verif_pseudo_fichier, rien trouvé ---\n");
	fclose(fichier);
	return cpt;
}

int test_mdp(FILE* fichier, char pseudo[BUFFSIZE], char mdp[BUFFSIZE]){
	//printf("\n--- Début test_mdp ---\n");
	int i;

	char pseudo_test[BUFFSIZE];
	char mdp_test[BUFFSIZE];

	int cpt = 0;

	fichier = fopen("comptes.txt", "r");

	while(fgets(pseudo_test, BUFFSIZE, fichier) != NULL){
		//on enleve le retour chariot de fin de ligne
		for(i = 0; i < BUFFSIZE; i++){
			if(pseudo_test[i] == '\n'){
				pseudo_test[i] = '\0';
				break;
			}
		}
		if(strcmp(pseudo_test, pseudo) == 0){

			fgets(mdp_test, BUFFSIZE, fichier);
			//on enleve le retour chariot de fin de ligne
			for(i = 0; i < BUFFSIZE; i++){
				if(mdp_test[i] == '\n'){
					mdp_test[i] = '\0';
					break;
				}
			}
			if(strcmp(mdp_test, mdp) == 0){
				cpt = 1;
				break;
			}
			//printf("Comparaison MDP : |%s|(%d) et |%s|(%d)\n", mdp, strlen(mdp), mdp_test, strlen(mdp_test));
		}
		//printf("Comparaison PSEUDO : |%s|(%d) et |%s|(%d)\n", pseudo, strlen(pseudo), pseudo_test, strlen(pseudo_test));
	}

	if(cpt == 1){
		//printf("\n--- FIN test_mdp, on a trouvé ---\n");
		fclose(fichier);
		return cpt;
	}

	//printf("\n--- FIN test_mdp, rien trouvé ---\n");
	fclose(fichier);
	return 0;
}
