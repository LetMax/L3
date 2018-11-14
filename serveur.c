/* TODO

TOUT L'AFFICHAGE
RAPPORT
INDENTATION

*/

#include "serveur.h"

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

	int i, j, k, r;
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

						if(strcmp(buffer, "/liste") == 0){
							liste_joueurs_i(conserv, nb_c, result, i);
						}

						//un joueur veut un défi
						else if((defi(buffer)) == 0){
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
						else if(spectate(buffer) == 0){
							int joueur_en_jeu = 0;
							joueur_en_jeu = buffer[10] - '0';

							if(joueur_en_jeu >= 0 && joueur_en_jeu < nb_c){
								if(conserv[joueur_en_jeu].statut == 2){
									if(conserv[joueur_en_jeu].partie_en_cours->nb_spec < 7){

										char first_string_send[50];

										//message au spectateur
										send_client(conserv[i].csock, "*** Vous observez ");
										send_client(conserv[i].csock, conserv[joueur_en_jeu].pseudo);
										send_client(conserv[i].csock, " Vs ");
										send_client(conserv[i].csock, conserv[joueur_en_jeu].adversaire->pseudo);
										send_client(conserv[i].csock, " ***\n");
										send_client(conserv[i].csock, "*** Mode spectateur ***\n");

										//mise à jour de la partie en cours des joueurs
										conserv[joueur_en_jeu].partie_en_cours->spec[conserv[joueur_en_jeu].partie_en_cours->nb_spec] = conserv[i];
										conserv[joueur_en_jeu].partie_en_cours->nb_spec++;
										conserv[joueur_en_jeu].adversaire->partie_en_cours = conserv[joueur_en_jeu].partie_en_cours;
										conserv[i].statut = 3;
										conserv[i].partie_en_cours = conserv[joueur_en_jeu].partie_en_cours;
										damier_to_string(first_string_send, conserv[i].partie_en_cours->damier);
										send_client(conserv[i].csock, first_string_send);

									}
									else{
										send_client(conserv[i].csock, "*** Trop de gens regardent deja cette partie ***\n");
									}
								}
								else{
									send_client(conserv[i].csock, "*** Ce joueur n'est pas en partie ***\n");
								}
							}
							else{
								send_client(conserv[i].csock, "*** Vous voulez observer un fantome (personne en gros) ***\n");
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

									conserv[j].partie_en_cours = &partie_tmp;

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

						char damier_string_send[50];

						//abandon du joueur
						if(strcmp(buffer, "/ff") == 0){
							if(strcmp(partie_tmp2->joueurA.pseudo, conserv[i].pseudo) == 0){
								partie_tmp2->pions_A = 0;
							}
							else{
								partie_tmp2->pions_B = 0;
							}
						}

						/* COUP DU JOUEUR */
						else{

							if(strcmp(partie_tmp2->joueurA.pseudo, conserv[i].pseudo) == 0){
								if(coup(buffer, partie_tmp2->damier, 0) == 0){
									//coup non valide du joueur A
									send_client(conserv[i].csock, "***Coup non valide, rejouez***\n");
								}
								//coup valide
								else{
									send_client(conserv[i].csock, "***Coup valide***\n");
									conserv[i].partie_en_cours = partie_tmp2;

									//on prévient l'adversaire
									for(j = 0; j < nb_c; j++){
										if(strcmp(conserv[i].adversaire->pseudo, conserv[j].pseudo) == 0){
											conserv[j].partie_en_cours = partie_tmp2;
											damier_to_string(damier_string_send, conserv[j].partie_en_cours->damier);
											send_client(conserv[j].csock, damier_string_send);
										}
									}
									//on gere les spectateurs
									for(j = 0; j < partie_tmp2->nb_spec; j++){
										send_client(partie_tmp2->spec[j].csock, damier_string_send);
									}
								}
							}
							else{
								if(coup(buffer, partie_tmp2->damier, 1) == 0){
									//coup non valide du joueur B
									send_client(conserv[i].csock, "***Coup non valide, rejouez***\n");
								}
								//coup valide
								else{
									send_client(conserv[i].csock, "***Coup valide***\n");
									conserv[i].partie_en_cours = partie_tmp2;

									//on prévient l'adversaire
									for(j = 0; j < nb_c; j++){
										if(strcmp(conserv[i].adversaire->pseudo, conserv[j].pseudo) == 0){
											conserv[j].partie_en_cours = partie_tmp2;
											damier_to_string(damier_string_send, conserv[j].partie_en_cours->damier);
											send_client(conserv[j].csock, damier_string_send);
										}
									}
									//on gere les spectateurs
									for(j = 0; j < partie_tmp2->nb_spec; j++){
										send_client(partie_tmp2->spec[j].csock, damier_string_send);
									}
								}
							}
						}

						//actualisation du damier
						actualiser_pions_damier(partie_tmp2->damier, partie_tmp2->pions_A, partie_tmp2->pions_B);

						/* TEST SI LA PARTIE EST FINIE */
						if(partie_tmp2->pions_A <= 0){

							//on prévient les spectateurs
							for(j = 0; j < partie_tmp2->nb_spec; j++){
								send_client(partie_tmp2->spec[j].csock, "*** FIN DE PARTIE ***\n");
								for(k = 0; k < nb_c; k++){
									if(strcmp(partie_tmp2->spec[j].pseudo, conserv[k].pseudo) == 0){
										conserv[k].statut = 0;
									}
								}
							}

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
										send_client(conserv[i].csock, "*** FIN DE PARTIE ***\n");
										send_client(conserv[i].csock, "*** Bravo vous avez gagné !***\n");

										send_client(conserv[j].csock, "*** FIN DE PARTIE ***\n");
										send_client(conserv[j].csock, "*** Vous avez perdu, c'est un peu triste ***\n");
									}

									else{
										//message de fin de partie
										send_client(conserv[j].csock, "*** FIN DE PARTIE ***\n");
										send_client(conserv[j].csock, "*** Bravo vous avez gagné !***\n");

										send_client(conserv[i].csock, "*** FIN DE PARTIE ***\n");
										send_client(conserv[i].csock, "*** Vous avez perdu, c'est un peu triste ***\n");
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

							//on prévient les spectateurs
							for(j = 0; j < partie_tmp2->nb_spec; j++){
								send_client(partie_tmp2->spec[j].csock, "*** FIN DE PARTIE ***\n");
							}

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
										send_client(conserv[j].csock, "*** FIN DE PARTIE ***\n");
										send_client(conserv[j].csock, "*** Bravo vous avez gagné !***\n");

										send_client(conserv[i].csock, "*** FIN DE PARTIE ***\n");
										send_client(conserv[i].csock, "*** Vous avez perdu, c'est un peu triste ***\n");
									}

									else{
										//message de fin de partie
										send_client(conserv[i].csock, "*** FIN DE PARTIE ***\n");
										send_client(conserv[i].csock, "*** Bravo vous avez gagné !***\n");

										send_client(conserv[j].csock, "*** FIN DE PARTIE ***\n");
										send_client(conserv[j].csock, "*** Vous avez perdu, c'est un peu triste ***\n");
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
					//message d'un spectateur
					else if(conserv[i].statut == 3){
						if(strcmp(buffer, "/exit") == 0){

							struct Partie* partie_quittee = NULL;
							partie_quittee = conserv[i].partie_en_cours;
							int i_remove;

							//on retrouve le spectateur qui veut quitter dans le tableau des spectateurs
							for(j = 0; j < partie_quittee->nb_spec; j++){
								if(strcmp(conserv[i].pseudo, partie_quittee->spec[j].pseudo) == 0){
									i_remove = j;
									break;
								}
							}

							//on le retire
							for(j = i_remove; j < partie_quittee->nb_spec; j++){
								partie_quittee->spec[j] = partie_quittee->spec[j+1];
							}

							partie_quittee->nb_spec--;

							//on retrouve les joueurs qu'il observait pour mettre à jour leur partie_en_cours
							for(j = 0; j < nb_c; j++){
								if(strcmp(conserv[i].partie_en_cours->joueurA.pseudo,conserv[j].pseudo) == 0){
									conserv[j].partie_en_cours = partie_quittee;
								}
							}

							for(j = 0; j < nb_c; j++){
								if(strcmp(conserv[i].partie_en_cours->joueurB.pseudo,conserv[j].pseudo) == 0){
									conserv[j].partie_en_cours = partie_quittee;
								}
							}
							send_client(conserv[i].csock, "*** FIN DE PARTIE ***\n");

							conserv[i].statut = 0;
							conserv[i].partie_en_cours = NULL;
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
