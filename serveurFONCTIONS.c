#include "serveur.h"

void actualiser_pions_damier(int damier[10][10], int pions_A, int pions_B){
	pions_A = 0;
	pions_B = 0;

	int i, j;

	for(i = 0; i < 10; i++){
		for(j = 0; j < 10; j++){
			//pions noirs
			if(damier[i][j] == 1 || damier[i][j] == 2){
				pions_B++;
			}
			//pions rouges
			else if(damier[i][j] == 3 || damier[i][j] == 4){
				pions_A++;
			}
		}
	}
}

int coup(char buffer[BUFFSIZE], int damier[10][10], int num_joueur){
	int length = strlen(buffer);
	int cpt_deplacement = 0;
	int cpt_prise = 0;

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
				if(num_joueur == 0){
					//case depart = pion rouge
					if(damier[case_depart[0]][case_depart[1]] == 3){
						//déplacement normal
						if(cpt_deplacement == 0 && cpt_prise == 0 && (case_depart[0] - case_arrivee[0]) == 1 && ((case_arrivee[1] - case_depart[1]) == 1 || (case_arrivee[1] - case_depart[1]) == -1 )){
							cpt_deplacement = 1;
							damier[case_depart[0]][case_depart[1]] = 0;
							//le pion devient une dame
							if(case_arrivee[0] == 0){
								damier[case_arrivee[0]][case_arrivee[1]] = 4;
							}
							else damier[case_arrivee[0]][case_arrivee[1]] = 3;
						}
						//prise
						else if(cpt_deplacement == 0 && ((case_depart[0] - case_arrivee[0]) == 2 || (case_depart[0] - case_arrivee[0]) == -2) && ((case_arrivee[1] - case_depart[1]) == 2 || (case_arrivee[1] - case_depart[1]) == -2 )){
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
							cpt_prise = 1;
						}

						else{
							printf("probleme case d'arrivée (ni déplacement ni prise)\n");
							return 0;
						}
					}
					//dame rouge
					else if(damier[case_depart[0]][case_depart[1]] == 4){
						if(deplacement_dame(case_depart, case_arrivee, damier, num_joueur, cpt_deplacement, cpt_prise) == 0){
							return 0;
						}
					}

					else{
						printf("probleme case départ\n");
						return 0;
					}
				}

				//joueur B
				else if(num_joueur == 1){
					//case depart = pion noir
					if(damier[case_depart[0]][case_depart[1]] == 1){
						//déplacement normal
						if(cpt_deplacement == 0 && cpt_prise == 0 && (case_depart[0] - case_arrivee[0]) == -1 && ((case_arrivee[1] - case_depart[1]) == 1 || (case_arrivee[1] - case_depart[1]) == -1 )){
							cpt_deplacement = 1;
							damier[case_depart[0]][case_depart[1]] = 0;
							//le pion devient une dame
							if(case_arrivee[0] == 9){
								damier[case_arrivee[0]][case_arrivee[1]] = 2;
							}
							else damier[case_arrivee[0]][case_arrivee[1]] = 1;
						}
						//prise
						else if(cpt_deplacement == 0 && ((case_depart[0] - case_arrivee[0]) == 2 || (case_depart[0] - case_arrivee[0]) == -2) && ((case_arrivee[1] - case_depart[1]) == 2 || (case_arrivee[1] - case_depart[1]) == -2 )){
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
							cpt_prise = 1;
						}
						else {
							printf("probleme case d'arrivée (ni déplacement ni prise du pion noir)\n");
							return 0;
						}
					}
					//dame noire
					else if(damier[case_depart[0]][case_depart[1]] == 2){
						if(deplacement_dame(case_depart, case_arrivee, damier, num_joueur, cpt_deplacement, cpt_prise) == 0){
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

int deplacement_dame(int case_depart[2], int case_arrivee[2], int damier[10][10], int num_joueur, int cpt_deplacement, int cpt_prise){
	int i, j;
	cpt_prise = 0;

	//en diagonale
	if(cpt_deplacement == 0 && abs(case_depart[0] - case_arrivee[0]) == abs(case_depart[1] - case_arrivee[1])){
		//joueur A
		if(num_joueur == 0){
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
							cpt_prise = 1;
						}
					}
				}
				damier[case_arrivee[0]][case_arrivee[1]] = 4;
				damier[case_depart[0]][case_depart[1]] = 0;
				if(cpt_prise == 0){
					cpt_deplacement = 1;
				}
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
							cpt_prise = 1;
						}
					}
				}
				damier[case_arrivee[0]][case_arrivee[1]] = 4;
				damier[case_depart[0]][case_depart[1]] = 0;
				if(cpt_prise == 0){
					cpt_deplacement = 1;
				}
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
							cpt_prise = 1;
						}
					}
				}
				damier[case_arrivee[0]][case_arrivee[1]] = 4;
				damier[case_depart[0]][case_depart[1]] = 0;
				if(cpt_prise == 0){
					cpt_deplacement = 1;
				}
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
							cpt_prise = 1;
						}
					}
				}
				damier[case_arrivee[0]][case_arrivee[1]] = 4;
				damier[case_depart[0]][case_depart[1]] = 0;
				if(cpt_prise == 0){
					cpt_deplacement = 1;
				}
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
							cpt_prise = 1;
						}
					}
				}
				damier[case_arrivee[0]][case_arrivee[1]] = 2;
				damier[case_depart[0]][case_depart[1]] = 0;
				if(cpt_prise == 0){
					cpt_deplacement = 1;
				}
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
							cpt_prise = 1;
						}
					}
				}
				damier[case_arrivee[0]][case_arrivee[1]] = 2;
				damier[case_depart[0]][case_depart[1]] = 0;
				if(cpt_prise == 0){
					cpt_deplacement = 1;
				}
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
							cpt_prise = 1;
						}
					}
				}
				damier[case_arrivee[0]][case_arrivee[1]] = 2;
				damier[case_depart[0]][case_depart[1]] = 0;
				if(cpt_prise == 0){
					cpt_deplacement = 1;
				}
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
							cpt_prise = 1;
						}
					}
				}
				damier[case_arrivee[0]][case_arrivee[1]] = 2;
				damier[case_depart[0]][case_depart[1]] = 0;
				if(cpt_prise == 0){
					cpt_deplacement = 1;
				}
			}
		}
		return 1;
	}
	printf("déplacement dame hors diagonale\n");
	return 0;
}

int moyenne (int a, int b){
	return (a+b)/2;
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
        k++;
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
	partie.nb_spec = 0;

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

int spectate(char buffer[BUFFSIZE]){
	if(buffer[0] == '/' && buffer[1] == 's' && buffer[2] == 'p' && buffer[3] == 'e' && buffer[4] == 'c' && buffer[5] == 't' && buffer[6] == 'a' && buffer[7] == 't' && buffer[8] == 'e'){
		return 0;
	}
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

void liste_joueurs(struct Client tab[MAX_CLIENT], int nb_c, char result[1]){

	int k, j;

	for(k = 0; k < nb_c; k++){
		if(tab[k].statut != 2 && tab[k].statut != 3){
			send_client(tab[k].csock, "--- Joueurs connectés ---\n");
			for(j = 0; j < nb_c; j++){
				send_client(tab[k].csock, "joueur ");
				send_client(tab[k].csock, inttos(result,j));
				send_client(tab[k].csock, " : ");
				send_client(tab[k].csock, tab[j].pseudo);
				if (tab[k].pseudo == tab[j].pseudo) send_client(tab[k].csock, " (vous)");
				if (tab[j].statut == 2) send_client(tab[k].csock, "(en jeu)");
				if (tab[j].statut == 3) send_client(tab[k].csock, "(regarde une partie)");
				send_client(tab[k].csock, "\n");
			}
			send_client(tab[k].csock, "-------------------------\n");
		}
	}
}

void liste_joueurs_i(struct Client tab[MAX_CLIENT], int nb_c, char result[1], int i){
	int k;
	send_client(tab[i].csock, "--- Joueurs connectés ---\n");
	for(k = 0; k < nb_c; k++){
		send_client(tab[i].csock, "joueur ");
		send_client(tab[i].csock, inttos(result,k));
		send_client(tab[i].csock, " : ");
		send_client(tab[i].csock, tab[k].pseudo);
		if (tab[i].pseudo == tab[k].pseudo) send_client(tab[i].csock, " (vous)");
		if (tab[k].statut == 2) send_client(tab[i].csock, "(en jeu)");
		if (tab[k].statut == 3) send_client(tab[i].csock, "(regarde une partie)");
		send_client(tab[i].csock, "\n");
	}
	send_client(tab[i].csock, "-------------------------\n");
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
