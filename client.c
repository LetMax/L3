#include "client.h"

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
	int is_spectateur = 0;

	int i;

	fd_set readfds;
	char buffer[BUFFSIZE];

	int sock = connect_socket("localhost", port);
	send_server(sock, pseudo);

	//version abstraite du damier
	int damier[10][10];
	//damier temporaire
	int damier_tmp[10][10];

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

			if(is_spectateur == 1){

				//fin de partie
				if(strcmp(buffer, "*** FIN DE PARTIE ***\n") == 0){
					SDL_DestroyWindow(window);
					SDL_DestroyRenderer(renderer); // Libération de la surface
					SDL_Quit();
					is_spectateur = 0;
				}

				else{
					string_to_damier(buffer, damier);
					actualiser_damier_SDL(renderer, damier, WINDOW_SIZE, WINDOW_SIZE);
					SDL_RenderPresent(renderer);
				}
			}
			else{
				if(tour == 0){
					//fin de partie
					if(strcmp(buffer, "*** FIN DE PARTIE ***\n") == 0){
						statut = LIBRE;
						SDL_DestroyWindow(window);
						SDL_DestroyRenderer(renderer); // Libération de la surface
						SDL_Quit();
					}

					//deconnexion par le serveur
					else if(strcmp(buffer, "au revoir\n") == 0){
						break;
					}

					else{
						string_to_damier(buffer, damier);
						actualiser_damier_SDL(renderer, damier, WINDOW_SIZE, WINDOW_SIZE);
						SDL_RenderPresent(renderer);
						printf("***A vous de jouer***\n");
						tour = 1;
					}
				}
				else{
					printf("%s", buffer);
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

					//fin de partie
					if(strcmp(buffer, "*** FIN DE PARTIE ***\n") == 0){
						statut = LIBRE;
						SDL_DestroyWindow(window);
						SDL_DestroyRenderer(renderer); // Libération de la surface
						SDL_Quit();
					}

					//mode spectateur
					else if(strcmp(buffer, "*** Mode spectateur ***\n") == 0){

						creer_damier(damier);

						SDL_CreateWindowAndRenderer(WINDOW_SIZE, WINDOW_SIZE, SDL_WINDOW_SHOWN, &window, &renderer);
						SDL_SetWindowPosition(window, WINDOW_SIZE*2 + 175, 50);
						SDL_SetWindowTitle(window, "Mode spectateur");
					  actualiser_damier_SDL(renderer, damier, WINDOW_SIZE, WINDOW_SIZE);
						SDL_RenderPresent(renderer);
						is_spectateur = 1;
					}

					//deconnexion par le serveur
					else if(strcmp(buffer, "au revoir\n") == 0){
						break;
					}
				}
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
						send_server(sock, buffer);
						damiercpy(damier, damier_tmp);
					}
				}
				else printf("Ce n'est pas votre tour\n");
			}
  	}
	}
	return 0;
}
