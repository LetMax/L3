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

void actualiser_damier_SDL(SDL_Renderer *renderer, int damier[10][10], int x, int y);
void dessiner_ligne(SDL_Renderer *r, int x, int y, int w);
void dessiner_pion(SDL_Renderer *r, int cx, int cy, int rayon);
void dessiner_dame(SDL_Renderer *r, int cx, int cy, int rayon);

void creer_damier(int damier[10][10]);

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

	//titre de la fenetre SDL
	char title[BUFFSIZE];
	memset(title, '\0', sizeof(title));
	strcpy(title, "Damier de ");
	strcat(title, pseudo);

	int i;

	fd_set readfds;
	char buffer[BUFFSIZE];

	int sock = connect_socket("localhost", port);
	send_server(sock, pseudo);

	//version abstraite du damier
	int damier[10][10];

	/*SDL_Surface *ecran = NULL, *rectangle = NULL;*/
  SDL_Init(SDL_INIT_VIDEO);
	SDL_Window* window = NULL;
	SDL_Renderer* renderer = NULL;//Déclaration du renderer


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

				//début d'une partie, le client est le joueur A
				if(strcmp(buffer, "*** La partie commence : tu joues les pions rouges  ***\n") == 0){
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
					creer_damier(damier);

					SDL_CreateWindowAndRenderer(WINDOW_SIZE, WINDOW_SIZE, SDL_WINDOW_SHOWN, &window, &renderer);
					SDL_SetWindowPosition(window, WINDOW_SIZE + 100, 50);
					SDL_SetWindowTitle(window, title);
				  actualiser_damier_SDL(renderer, damier, WINDOW_SIZE, WINDOW_SIZE);
					SDL_RenderPresent(renderer);
				}

				//victoire
				else if(strcmp(buffer, "Tu vois quand tu veux, t'es pas si mauvais !") == 0){
					SDL_DestroyWindow(window);
					SDL_DestroyRenderer(renderer); // Libération de la surface
					SDL_Quit();
				}

				//défaite
				else if(strcmp(buffer, "Bon, on va rien dire ... (loser)") == 0){
					SDL_DestroyWindow(window);
					SDL_DestroyRenderer(renderer); // Libération de la surface
					SDL_Quit();
				}

				//deconnexion par le serveur
				else if(strcmp(buffer, "au revoir\n") == 0){
					break;
				}
    	}

    	if(FD_ISSET(STDIN_FILENO, &readfds)){
			//message à envoyer
    		fgets(buffer, BUFFSIZE, stdin);
    		buffer[strlen(buffer)-1]='\0';

			//deconnexion
    		if(strcmp(buffer, "/quit") == 0){
    			break;
    		}
    		send_server(sock, buffer);
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
