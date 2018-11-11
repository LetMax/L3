/* TODO

déplacement dame en diagonale obligatoire

*/

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

void actualiser_damier_SDL(SDL_Renderer *renderer, int damier[10][10], int x, int y);
void dessiner_ligne(SDL_Renderer *r, int x, int y, int w);
void dessiner_pion(SDL_Renderer *r, int cx, int cy, int rayon);
void dessiner_dame(SDL_Renderer *r, int cx, int cy, int rayon);

void creer_damier(int damier[10][10]);
void string_to_damier(char buffer[50], int damier[10][10]);
void damier_to_string(char buffer[50], int damier[10][10]);
int coup(char buffer[BUFFSIZE], int damier[10][10], int num_joueur);
int remplissage_tab_coord(int tab_coord[20][2], char buffer[BUFFSIZE], int length);
int test_int_buffer(char buffer[BUFFSIZE], int n);
int deplacement_dame(int case_depart[2], int case_arrivee[2], int damier[10][10], int num_joueur);
void damiercpy(int damier_tmp[10][10], int damier[10][10]);
int moyenne (int a, int b);

int connect_socket(char* adresse, int port);
int recv_server(int sock, char *buffer);
int send_server(int sock, char *buffer);

int main(int argc, char* argv[]){

	if(argc != 3) {
	  printf("L'appel doit etre de type %s pseudo port\n",argv[0]);
	  exit(-1);
	}

	char* pseudo = argv[1];
	int port = atoi(argv[2]);
	int statut = LIBRE;
	int num_joueur;
	int tour;

	int i;

	fd_set readfds;
	char buffer[BUFFSIZE];

	int sock = connect_socket("localhost", port);
	send_server(sock, pseudo);

	//version abstraite du damier
	int damier[10][10];
	//damier temporaire
	int damier_tmp[10][10];
	char damier_string_send[50];

	/*SDL_Surface *ecran = NULL, *rectangle = NULL;*/
  SDL_Init(SDL_INIT_VIDEO);
	SDL_Window* window = NULL;
	SDL_Renderer* renderer = NULL;//Déclaration du

	//titre de la fenetre SDL
	char title[BUFFSIZE];
	memset(title, '\0', sizeof(title));
	strcpy(title, "Damier de ");
	strcat(title, pseudo);


	while(1) {

		for(i = 0; i<BUFFSIZE; i++){
			buffer[i] = '\0';
		}
		//descripteurs de fichier
		FD_ZERO(&readfds);
  	FD_SET(sock, &readfds);
  	FD_SET(STDIN_FILENO, &readfds);

  	int s = select(sock+1, &readfds,NULL,NULL,NULL);
  	if(s==-1){
  		perror("select");
  		exit(-1);
  	}

  	if(FD_ISSET(sock, &readfds)){

  		recv_server(sock, buffer);
  		printf("%s", buffer);

			if(tour == 0){

				printf("Damier recu = |");
				for(i = 0; i<strlen(buffer); i++){
					printf("%c", buffer[i]);
				}
				printf("|\n");

				string_to_damier(buffer, damier);
				actualiser_damier_SDL(renderer, damier, WINDOW_SIZE, WINDOW_SIZE);
				SDL_RenderPresent(renderer);
				printf("***A vous de jouer***\n");
				tour = 1;

				//victoire
				if(strcmp(buffer, "Tu vois quand tu veux, t'es pas si mauvais !") == 0){
					statut = LIBRE;
					SDL_DestroyWindow(window);
					SDL_DestroyRenderer(renderer); // Libération de la surface
					SDL_Quit();
				}

				//défaite
				else if(strcmp(buffer, "Bon, on va rien dire ... (loser)") == 0){
					statut = LIBRE;
					SDL_DestroyWindow(window);
					SDL_DestroyRenderer(renderer); // Libération de la surface
					SDL_Quit();
				}

				//deconnexion par le serveur
				else if(strcmp(buffer, "au revoir\n") == 0){
					break;
				}
			}
			else{
				//début d'une partie, le client est le joueur A
				if(strcmp(buffer, "*** La partie commence : tu joues les pions rouges  ***\n") == 0){
					statut = EN_PARTIE;
					num_joueur = A;
					tour = 1;

					printf("*** A toi de jouer ! ***\n");
					creer_damier(damier);

					SDL_CreateWindowAndRenderer(WINDOW_SIZE, WINDOW_SIZE, SDL_WINDOW_SHOWN, &window, &renderer);
					SDL_SetWindowPosition(window, 50, 50);
					SDL_SetWindowTitle(window, title);
				  actualiser_damier_SDL(renderer, damier, WINDOW_SIZE, WINDOW_SIZE);
					SDL_RenderPresent(renderer);
				}

				//début d'une partie, le client est le joueur B
				else if(strcmp(buffer, "*** La partie commence : tu joues les pions noirs  ***\n") == 0){
					statut = EN_PARTIE;
					num_joueur = B;
					tour = 0;

					creer_damier(damier);

					SDL_CreateWindowAndRenderer(WINDOW_SIZE, WINDOW_SIZE, SDL_WINDOW_SHOWN, &window, &renderer);
					SDL_SetWindowPosition(window, WINDOW_SIZE + 100, 50);
					SDL_SetWindowTitle(window, title);
				  actualiser_damier_SDL(renderer, damier, WINDOW_SIZE, WINDOW_SIZE);
					SDL_RenderPresent(renderer);
				}

				//validation du coup par le serveur
				else if(strcmp(buffer, "***Coup valide***\n") == 0){
					actualiser_damier_SDL(renderer, damier, WINDOW_SIZE, WINDOW_SIZE);
					SDL_RenderPresent(renderer);
					printf("***Votre adversaire joue ...***\n");
					tour = 0;
				}

				//victoire
				else if(strcmp(buffer, "Tu vois quand tu veux, t'es pas si mauvais !") == 0){
					statut = LIBRE;
					SDL_DestroyWindow(window);
					SDL_DestroyRenderer(renderer); // Libération de la surface
					SDL_Quit();
				}

				//défaite
				else if(strcmp(buffer, "Bon, on va rien dire ... (loser)") == 0){
					statut = LIBRE;
					SDL_DestroyWindow(window);
					SDL_DestroyRenderer(renderer); // Libération de la surface
					SDL_Quit();
				}

				//deconnexion par le serveur
				else if(strcmp(buffer, "au revoir\n") == 0){
					break;
				}
			}
		}

  	if(FD_ISSET(STDIN_FILENO, &readfds)){
			//message à envoyer
  		fgets(buffer, BUFFSIZE, stdin);
  		buffer[strlen(buffer)-1]='\0';
			printf("|%s|(%ld)\n", buffer, strlen(buffer));

			//deconnexion
  		if(strcmp(buffer, "/quit") == 0){
  			break;
  		}

			if(statut == LIBRE){
				send_server(sock, buffer);
			}

			else if(statut == EN_PARTIE){
				//abandon
				if(strcmp(buffer, "/ff") == 0){
					send_server(sock, buffer);
				}
				else if(tour == 1){

					damiercpy(damier_tmp, damier);
					//coup non valide
					if(coup(buffer, damier_tmp, num_joueur) == 0){
						printf("*** Coup non valide, rejouez ***\n");
					}
					//coup valide
					else {
						damier_to_string(damier_string_send, damier_tmp);
						send_server(sock, damier_string_send);
						damiercpy(damier, damier_tmp);
					}
				}
				else printf("Ce n'est pas votre tour\n");
			}

  	}
	}
	return 0;
}

void actualiser_damier_SDL(SDL_Renderer *renderer, int damier[10][10], int x, int y){

	int xcase = x/10;
	int ycase = y/10;

	SDL_Rect r;
	r.x = 0;
	r.y = 0;
	r.w = xcase;
	r.h = ycase;

	int i,j;

	//on recreer le damier
	for(i = 0; i < 10; i++){
		for(j = 0; j < 10; j++){
			if(((i + j) % 2) == 0){
				// Remplissage de la surface avec du vanille
				SDL_SetRenderDrawColor(renderer, 225, 206, 154, 0);
				SDL_RenderFillRect(renderer, &r);
			}
			else{
				SDL_SetRenderDrawColor(renderer, 159, 85, 30, 0);
				SDL_RenderFillRect(renderer, &r);
			}
			r.x += xcase;
		}
		r.y += ycase;
		r.x = 0;
	}

	int disqueX = xcase;
	int disqueY = ycase;

  //caractéristiques des disques
  int positionX = disqueX/2;
  int positionY = disqueY/2;
  int rayon = disqueX/2 - 5;

	//on place les pions
	for(i = 0; i < 10; i++){
		for(j = 0; j < 10; j++){
			//pion noir
			if(damier[i][j] == 1){
				SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
				dessiner_pion(renderer, positionX, positionY, rayon);
			}
			//dame noire
			if(damier[i][j] == 2){
				SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
				dessiner_pion(renderer, positionX, positionY, rayon);
				SDL_SetRenderDrawColor(renderer, 255, 215, 0, 0);
				dessiner_dame(renderer, positionX, positionY, rayon);
			}
			//pion rouge
			if(damier[i][j] == 3){
				SDL_SetRenderDrawColor(renderer, 255, 0, 0, 0);
				dessiner_pion(renderer, positionX, positionY, rayon);
			}
			//dame rouge
			if(damier[i][j] == 4){
				SDL_SetRenderDrawColor(renderer, 255, 0, 0, 0);
				dessiner_pion(renderer, positionX, positionY, rayon);
				SDL_SetRenderDrawColor(renderer, 255, 215, 0, 0);
				dessiner_dame(renderer, positionX, positionY, rayon);
			}
      positionX += disqueX;
		}
    positionY += disqueY;
    positionX = 25;
	}
}

void dessiner_ligne(SDL_Renderer *r, int x, int y, int w){
	SDL_Rect rect;

	rect.x = x;
	rect.y = y;
	rect.w = w;
	rect.h = 1;

	SDL_RenderDrawRect(r, &rect);
}

void dessiner_pion(SDL_Renderer *r, int cx, int cy, int rayon){

  int d, y, x;

  d = 3 - (2 * rayon);
  x = 0;
  y = rayon;

  while (y >= x) {
    dessiner_ligne(r, cx - x, cy - y, 2 * x + 1);
    dessiner_ligne(r, cx - x, cy + y, 2 * x + 1);
    dessiner_ligne(r, cx - y, cy - x, 2 * y + 1);
    dessiner_ligne(r, cx - y, cy + x, 2 * y + 1);

    if (d < 0) d = d + (4 * x) + 6;
    else{
      d = d + 4 * (x - y) + 10;
      y--;
    }
    x++;
  }
}

void dessiner_dame(SDL_Renderer *r, int cx, int cy, int rayon){

  int d, y, x;

  d = 3 - rayon;
  x = 0;
  y = rayon/2;

  while (y >= x) {
    dessiner_ligne(r, cx - x, cy - y, 2 * x + 1);
    dessiner_ligne(r, cx - x, cy + y, 2 * x + 1);
    dessiner_ligne(r, cx - y, cy - x, 2 * y + 1);
    dessiner_ligne(r, cx - y, cy + x, 2 * y + 1);

    if (d < 0) d = d + (4 * x) + 6;
    else{
      d = d + 4 * (x - y) + 10;
      y--;
    }
    x++;
  }
}

void creer_damier(int damier[10][10]){

	int i, j;

	for(i = 0; i < 10; i++){
		for(j = 0; j < 10; j++){
			//cases jouables
			if(((i + j) % 2) == 1){
				//joueur A (en haut du damier, pions noirs)
				if(i < 4){
					damier[i][j] = 1;
				}
				//cases vides du milieu
				else if(i < 6){
					damier[i][j] = 0;
				}
				//joueur B (en bas du damier, pions rouges)
				else {
					damier[i][j] = 3;
				}
			}
      //cases non jouables
      else{
        damier[i][j] = -1;
      }
		}
	}
}

void string_to_damier(char buffer[50], int damier[10][10]){
	int i, j;
	int tmp;
	int k = 0;

	for(i = 0; i < 10; i++){
		for(j = 0; j < 10; j++){
			//cases jouables
			if(((i + j) % 2) == 1){
				tmp = 0;
				tmp = buffer[k] - '0';
				damier[i][j] = tmp;
				k++;
			}
			else damier[i][j] = -1;
		}
	}
}

void damier_to_string(char buffer[50], int damier[10][10]){
	int i, j;
	char tmp[1];
  int k = 0;

	for(i = 0; i < 10; i++){
		for(j = 0; j < 10; j++){
			//cases jouables
			if(((i + j) % 2) == 1){
				tmp[0] = damier[i][j] + '0';
        buffer[k] = tmp[0];
        printf("On a ajouté |%c| au buffer\n", buffer[k]);
        k++;
			}
		}
	}
	buffer[50] = '\0';
}

int coup(char buffer[BUFFSIZE], int damier[10][10], int num_joueur){
	int length = strlen(buffer);

	int i, j;
	int tab_coord[20][2];
	//initialisation des coordonnées
	for(i = 0; i < 20; i++){
		for(j = 0; j < 2; j++){
			tab_coord[i][j] = -1;
		}
	}

	if(test_int_buffer(buffer, length) == 0){
		printf("probleme test_int\n");
		return 0;
	}

	remplissage_tab_coord(tab_coord, buffer, length);

	for(i = 0; i < 20; i++){
		for(j = 0; j < 2; j++){
			if(tab_coord[i][j] != -1){
				printf("%d,", tab_coord[i][j]);
			}
		}
		printf(" ");
	}
	printf("\n");

	int case_depart[2];
	int case_arrivee[2];

	for(i = 0; i < 19; i++){
		case_depart[0] = tab_coord[i][0];
		case_depart[1] = tab_coord[i][1];
		case_arrivee[0] = tab_coord[i+1][0];
		case_arrivee[1] = tab_coord[i+1][1];
		//coordonnées valides
		if(case_depart[0] != -1 && tab_coord[i+1][0] != -1){
			//case d'arrivée = libre
			if(damier[case_arrivee[0]][case_arrivee[1]] == 0){
				//joueur A
				if(num_joueur == A){
					//case depart = pion rouge
					if(damier[case_depart[0]][case_depart[1]] == 3){
						//déplacement normal
						if((case_depart[0] - case_arrivee[0]) == 1 && ((case_arrivee[1] - case_depart[1]) == 1 || (case_arrivee[1] - case_depart[1]) == -1 )){
							damier[case_depart[0]][case_depart[1]] = 0;
							//le pion devient une dame
							if(case_arrivee[0] == 0){
								damier[case_arrivee[0]][case_arrivee[1]] = 4;
							}
							else damier[case_arrivee[0]][case_arrivee[1]] = 3;
						}
						//prise
						else if(((case_depart[0] - case_arrivee[0]) == 2 || (case_depart[0] - case_arrivee[0]) == -2) && ((case_arrivee[1] - case_depart[1]) == 2 || (case_arrivee[1] - case_depart[1]) == -2 )){
							//pion noir ou dame noire pris
							if((damier[moyenne(case_depart[0],case_arrivee[0])][moyenne(case_depart[1],case_arrivee[1])] == 1) || (damier[moyenne(case_depart[0],case_arrivee[0])][moyenne(case_depart[1],case_arrivee[1])] == 2)){
								damier[case_depart[0]][case_depart[1]] = 0;
								damier[moyenne(case_depart[0],case_arrivee[0])][moyenne(case_depart[1],case_arrivee[1])] = 0;
								//le pion devient une dame
								if(case_arrivee[0] == 0){
									damier[case_arrivee[0]][case_arrivee[1]] = 4;
								}
								else damier[case_arrivee[0]][case_arrivee[1]] = 3;
							}
							else {
								printf("probleme prise\n");
								return 0;
							}
						}

						else{
							printf("probleme case d'arrivée (ni déplacement ni prise)\n");
							return 0;
						}
					}
					//dame rouge
					else if(damier[case_depart[0]][case_depart[1]] == 4){
						if(deplacement_dame(case_depart, case_arrivee, damier, num_joueur) == 0){
							return 0;
						}
					}

					else{
						printf("probleme case départ\n");
						return 0;
					}
				}

				//joueur B
				else if(num_joueur == B){
					//case depart = pion noir
					if(damier[case_depart[0]][case_depart[1]] == 1){
						//déplacement normal
						if((case_depart[0] - case_arrivee[0]) == -1 && ((case_arrivee[1] - case_depart[1]) == 1 || (case_arrivee[1] - case_depart[1]) == -1 )){
							damier[case_depart[0]][case_depart[1]] = 0;
							//le pion devient une dame
							if(case_arrivee[0] == 9){
								damier[case_arrivee[0]][case_arrivee[1]] = 2;
							}
							else damier[case_arrivee[0]][case_arrivee[1]] = 1;
						}
						//prise
						else if(((case_depart[0] - case_arrivee[0]) == 2 || (case_depart[0] - case_arrivee[0]) == -2) && ((case_arrivee[1] - case_depart[1]) == 2 || (case_arrivee[1] - case_depart[1]) == -2 )){
							//pion rouge ou dame rouge pris
							if((damier[moyenne(case_depart[0],case_arrivee[0])][moyenne(case_depart[1],case_arrivee[1])] == 3) || (damier[moyenne(case_depart[0],case_arrivee[0])][moyenne(case_depart[1],case_arrivee[1])] == 4)){
								damier[case_depart[0]][case_depart[1]] = 0;
								damier[moyenne(case_depart[0],case_arrivee[0])][moyenne(case_depart[1],case_arrivee[1])] = 0;
								//le pion devient une dame
								if(case_arrivee[0] == 9){
									damier[case_arrivee[0]][case_arrivee[1]] = 2;
								}
								else damier[case_arrivee[0]][case_arrivee[1]] = 1;
							}

							else {
								printf("probleme prise (pion noir)\n");
								return 0;
							}
						}
						else {
							printf("probleme case d'arrivée (ni déplacement ni prise du pion noir)\n");
							return 0;
						}
					}
					//dame noire
					else if(damier[case_depart[0]][case_depart[1]] == 2){
						if(deplacement_dame(case_depart, case_arrivee, damier, num_joueur) == 0){
							return 0;
						}
					}

					else{
						printf("probleme case départ pion noir\n");
						return 0;
					}
				}
			}

			else{
				printf("probleme case d'arrivée (non libre)\n");
				return 0;
			}
		}
		else break;
	}
	return 1;
}

int remplissage_tab_coord(int tab_coord[20][2], char buffer[BUFFSIZE], int length){

	int i;
	int tmp = 0, k = 0;

	//Remplissage
	for(i = 0; i < 20; i++){
		while(k < length){
			tmp = 0;
			tmp = buffer[k] - '0';
			tab_coord[i][0] = tmp;
			k += 2;

			tmp = 0;
			tmp = buffer[k] - '0';
			tab_coord[i][1] = tmp;
			k += 2;
			break;
		}
	}
	return 1;
}

int test_int_buffer(char buffer[BUFFSIZE], int n){

	int tmp = 0;
	int i;
	if(n >= 7 && n <= 87){
		for(i = 0; i < n; i++){
			if((i % 2) == 0){
				tmp = 0;
				tmp = buffer[i] - '0';
				if(tmp < 0 || tmp > 9){
					return 0;
				}
			}
			else if((i % 2) == 1){
				if(buffer[i] != ',' && buffer[i] != ' '){
					return 0;
				}
			}
		}
	}
	else{
		return 0;
	}
	//on peut analyser le coup
	return 1;
}

int deplacement_dame(int case_depart[2], int case_arrivee[2], int damier[10][10], int num_joueur){
	int i, j;

	//joueur A
	if(num_joueur == A){
		//vers le haut gauche
		if(case_depart[0] > case_arrivee[0] && case_depart[1] > case_arrivee[1]){
			for(i = case_depart[0] - 1, j = case_depart[1] - 1; i > case_arrivee[0] && j > case_arrivee[1]; i--, j--){
				//pion allié
				if(damier[i][j] == 3 || damier[i][j] == 4){
					printf("probleme dame : pion allié rencontré\n");
					return 0;
				}
				//pion ennemi
				else if(damier[i][j] == 1 || damier[i][j] == 2){
					//la case d'apres doit etre libre
					if(damier[i-1][j-1] != 0){
						printf("probleme dame : case arrivée non libre\n");
						return 0;
					}
					else{
						damier[i][j] = 0;
					}
				}
			}
			damier[case_arrivee[0]][case_arrivee[1]] = 4;
			damier[case_depart[0]][case_depart[1]] = 0;
		}
		//vers le haut droite
		else if(case_depart[0] > case_arrivee[0] && case_depart[1] < case_arrivee[1]){
			for(i = case_depart[0] - 1, j = case_depart[1] + 1; i > case_arrivee[0] && j < case_arrivee[1]; i--, j++){
				//pion allié
				if(damier[i][j] == 3 || damier[i][j] == 4){
					printf("probleme dame : pion allié rencontré\n");
					return 0;
				}
				//pion ennemi
				else if(damier[i][j] == 1 || damier[i][j] == 2){
					//la case d'apres doit etre libre
					if(damier[i-1][j+1] != 0){
						printf("probleme dame : case arrivée non libre\n");
						return 0;
					}
					else{
						damier[i][j] = 0;
					}
				}
			}
			damier[case_arrivee[0]][case_arrivee[1]] = 4;
			damier[case_depart[0]][case_depart[1]] = 0;
		}
		//vers le bas gauche
		else if(case_depart[0] < case_arrivee[0] && case_depart[1] > case_arrivee[1]){
			for(i = case_depart[0] + 1, j = case_depart[1] - 1; i < case_arrivee[0] && j > case_arrivee[1]; i++, j--){
				//pion allié
				if(damier[i][j] == 3 || damier[i][j] == 4){
					printf("probleme dame : pion allié rencontré\n");
					return 0;
				}
				//pion ennemi
				else if(damier[i][j] == 1 || damier[i][j] == 2){
					//la case d'apres doit etre libre
					if(damier[i+1][j-1] != 0){
						printf("probleme dame : case arrivée non libre\n");
						return 0;
					}
					else{
						damier[i][j] = 0;
					}
				}
			}
			damier[case_arrivee[0]][case_arrivee[1]] = 4;
			damier[case_depart[0]][case_depart[1]] = 0;
		}
		//vers le bas droite
		else if(case_depart[0] < case_arrivee[0] && case_depart[1] < case_arrivee[1]){
			for(i = case_depart[0] + 1, j = case_depart[1] + 1; i < case_arrivee[0] && j < case_arrivee[1]; i++, j++){
				//pion allié
				if(damier[i][j] == 3 || damier[i][j] == 4){
					printf("probleme dame : pion allié rencontré\n");
					return 0;
				}
				//pion ennemi
				else if(damier[i][j] == 1 || damier[i][j] == 2){
					//la case d'apres doit etre libre
					if(damier[i+1][j+1] != 0){
						printf("probleme dame : case arrivée non libre\n");
						return 0;
					}
					else{
						damier[i][j] = 0;
					}
				}
			}
			damier[case_arrivee[0]][case_arrivee[1]] = 4;
			damier[case_depart[0]][case_depart[1]] = 0;
		}
		return 1;
	}
	//joueur B
	else{
		//vers le haut gauche
		if(case_depart[0] > case_arrivee[0] && case_depart[1] > case_arrivee[1]){
			for(i = case_depart[0] - 1, j = case_depart[1] - 1; i > case_arrivee[0] && j > case_arrivee[1]; i--, j--){
				//pion allié
				if(damier[i][j] == 1 || damier[i][j] == 2){
					printf("probleme dame : pion allié rencontré\n");
					return 0;
				}
				//pion ennemi
				else if(damier[i][j] == 3 || damier[i][j] == 4){
					//la case d'apres doit etre libre
					if(damier[i-1][j-1] != 0){
						printf("probleme dame : case arrivée non libre\n");
						return 0;
					}
					else{
						damier[i][j] = 0;
					}
				}
			}
			damier[case_arrivee[0]][case_arrivee[1]] = 2;
			damier[case_depart[0]][case_depart[1]] = 0;
		}
		//vers le haut droite
		else if(case_depart[0] > case_arrivee[0] && case_depart[1] < case_arrivee[1]){
			for(i = case_depart[0] - 1, j = case_depart[1] + 1; i > case_arrivee[0] && j < case_arrivee[1]; i--, j++){
				//pion allié
				if(damier[i][j] == 1 || damier[i][j] == 2){
					printf("probleme dame : pion allié rencontré\n");
					return 0;
				}
				//pion ennemi
				else if(damier[i][j] == 3 || damier[i][j] == 4){
					//la case d'apres doit etre libre
					if(damier[i-1][j+1] != 0){
						printf("probleme dame : case arrivée non libre\n");
						return 0;
					}
					else{
						damier[i][j] = 0;
					}
				}
			}
			damier[case_arrivee[0]][case_arrivee[1]] = 2;
			damier[case_depart[0]][case_depart[1]] = 0;
		}
		//vers le bas gauche
		else if(case_depart[0] < case_arrivee[0] && case_depart[1] > case_arrivee[1]){
			for(i = case_depart[0] + 1, j = case_depart[1] - 1; i < case_arrivee[0] && j > case_arrivee[1]; i++, j--){
				//pion allié
				if(damier[i][j] == 1 || damier[i][j] == 2){
					printf("probleme dame : pion allié rencontré\n");
					return 0;
				}
				//pion ennemi
				else if(damier[i][j] == 3 || damier[i][j] == 4){
					//la case d'apres doit etre libre
					if(damier[i+1][j-1] != 0){
						printf("probleme dame : case arrivée non libre\n");
						return 0;
					}
					else{
						damier[i][j] = 0;
					}
				}
			}
			damier[case_arrivee[0]][case_arrivee[1]] = 2;
			damier[case_depart[0]][case_depart[1]] = 0;
		}
		//vers le bas droite
		else if(case_depart[0] < case_arrivee[0] && case_depart[1] < case_arrivee[1]){
			for(i = case_depart[0] + 1, j = case_depart[1] + 1; i < case_arrivee[0] && j < case_arrivee[1]; i++, j++){
				//pion allié
				if(damier[i][j] == 1 || damier[i][j] == 2){
					printf("probleme dame : pion allié rencontré\n");
					return 0;
				}
				//pion ennemi
				else if(damier[i][j] == 3 || damier[i][j] == 4){
					//la case d'apres doit etre libre
					if(damier[i+1][j+1] != 0){
						printf("probleme dame : case arrivée non libre\n");
						return 0;
					}
					else{
						damier[i][j] = 0;
					}
				}
			}
			damier[case_arrivee[0]][case_arrivee[1]] = 2;
			damier[case_depart[0]][case_depart[1]] = 0;
		}
	}
	return 1;
}

void damiercpy(int damier_tmp[10][10], int damier[10][10]){
	int i, j;
	for(i = 0; i < 10; i++){
		for(j = 0; j < 10; j++){
			damier_tmp[i][j] = damier[i][j];
		}
	}
}

int moyenne (int a, int b){
	return (a+b)/2;
}

int connect_socket(char* adresse, int port){

	int sock = socket(AF_INET, SOCK_STREAM, 0); /* créé un socket TCP */
	if(sock==SOCKET_ERROR){
		perror("Erreur socket");
		exit(-1);
	}

	struct hostent* hostinfo = gethostbyname(adresse); /* infos du serveur */
	if (hostinfo==NULL){
		perror("Erreur serveur");
		exit(-1);
	}

	struct sockaddr_in sin;		/* structure qui possède toutes les infos pour le socket */
	sin.sin_addr = *(struct in_addr*) hostinfo->h_addr; /* on spécifie l'addresse */
	sin.sin_port = htons(port); 	/* le port */
	sin.sin_family = AF_INET; 	/* et le protocole (AF_INET pour IP) */
	int c = connect(sock, (struct sockaddr*) &sin, sizeof(struct sockaddr)); /* demande de connection */
	if(c==-1){
		perror("Erreur connect");
		exit(-1);
	}
	return sock;
}

int recv_server(int sock, char *buffer){

	int r = recv(sock, buffer, BUFFSIZE, 0);
	if(r==-1){
		perror("Erreur receive");
		exit(-1);
	}
	return r;
}

int send_server(int sock, char *buffer){

	int s = send(sock, buffer, BUFFSIZE, 0);
	if (s==-1){
		perror("Erreur send");
		exit(-1);
	}
	return s;
}
