#include "pse.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define CMD   "client"
#define MAX_STR 100
#define couleur(param) printf("\033[%sm",param)
/*   param devant être un const char *, vide (identique à "0") ou formé
     d'une où plusieurs valeurs séparées par des ; parmi
         0  réinitialisation         1  haute intensité (des caractères)
         5  clignotement             7  video inversé
         30, 31, 32, 33, 34, 35, 36, 37 couleur des caractères
         40, 41, 42, 43, 44, 45, 46, 47 couleur du fond
            les couleurs, suivant la logique RGB, étant respectivement
               noir, rouge, vert, jaune, bleu, magenta, cyan et blanc */




/* Déclaration de structures */


typedef struct pseudoSock pseudoSock;
struct pseudoSock{
	int sock;
	char pseudo[LIGNE_MAX];
};

/*******************************/


/* Déclaration des prototypes */


void *threadLecture(void *arg);
void *threadEcriture(void *arg);
unsigned long int hashing(char *chaine);

/*******************************/


/* Déclaration des sémaphores */

sem_t semLect;
sem_t semEcrit;

/*******************************/


/* Déclaration des variables globales */

int stop = 0;
int fin = 1;

/**************************************/



// Thread permettant la lecture des données de la fifo

void *threadLecture(void *arg) {
	pseudoSock *structClient = (pseudoSock *)arg;
	int sock = structClient->sock;
	char pseudo[LIGNE_MAX];
	strcpy(pseudo, structClient->pseudo);  

	char ligneRecue[LIGNE_MAX];
	int n;
  
  	while(stop == 0){
	  	lireLigne(sock,ligneRecue);
	  	
	  	if(fin == 1){	// Communication avec le serveur 
	  	
		  	sem_wait(&semLect);	// Attend le choix d'un destinataire côté thread écriture
		  	if(strcmp(ligneRecue,"offline") == 0){	// Le destinataire choisi est indispo.
		  		printf("Cette personne n'est pas connecté.\n");
		  		fin = 1;
		  	}
		  	if(strcmp(ligneRecue,"online") == 0){	// Le destinataire choisi est dispo.
		  		printf("Cette personne est connecté.\n");
		  		fin = 0;	// Démarrage communication + envoi message
		  	}
		  	sem_post(&semEcrit);	// Synchronisation avec thread écriture
		}
		
	  	if(fin == 0){	// Recepetion de message d'un autre client
			n=hashing(ligneRecue);
			switch(n)	// Affichage du message coloré en fonction du nom de l'émetteur du message reçu
			{
				case 0:
					couleur("31");
					printf("%s\n",ligneRecue);
					couleur("0");
					break;
				case 1:
					couleur("32");
					printf("%s\n",ligneRecue);
					couleur("0");
					break;
				case 2:
					couleur("33");
					printf("%s\n",ligneRecue);
					couleur("0");
					break;
				case 3:
					couleur("34");
					printf("%s\n",ligneRecue);
					couleur("0");
					break;
				case 4:
					couleur("35");
					printf("%s\n",ligneRecue);
					couleur("0");
					break;
				case 5:
					couleur("33");
					printf("%s\n",ligneRecue);
					couleur("0");
					break;
				case 6:
					couleur("32");
					printf("%s\n",ligneRecue);
					couleur("0");
					break;
			}
		}
	}
  	pthread_exit(NULL);
}


// Thread permettant l'écriture de données dans la fifo

void *threadEcriture(void *arg) {

	pseudoSock *structClient = (pseudoSock *)arg;
	int sock = structClient->sock;
	char ligne[LIGNE_MAX];
	char pseudo[LIGNE_MAX];
	strcpy(pseudo, structClient->pseudo);
	pseudo[strlen(pseudo)-1] = '\0';
	char *destinataire = malloc(sizeof(char)*LIGNE_MAX);
  
  	while(stop == 0){
  	
  		// CHOIX DESTINATAIRE :
  		
	  	couleur("5");
	  	printf("Destinaire ? : ");
	  	couleur("0");
	  	fgets(destinataire,LIGNE_MAX,stdin);
		destinataire[strlen(destinataire)-1] = '\0';
		
		if(strcmp(destinataire,"stop") == 0){		// ENVOI STOP = FIN SESSION CLIENT AU SERVEURs
			stop = 1;
			ecrireLigne(sock,destinataire);		
		}
		else{
			ecrireLigne(sock,destinataire);		// ENVOI DESTINATAIRE AU SERVEUR
		}
		
		// SYNCHRONISATION LECTURE - ECRITURE DE FIFO
		sem_post(&semLect);	// Démarre la lecture côté thread lecture pour savoir la disponibilité du destinataire
		sem_wait(&semEcrit);	// Attend la lecture du thread lecture
		
		while (fin == 0) {	// Envoi de message au destinataire
		
			if (fgets(ligne, LIGNE_MAX, stdin) == NULL){		// Récupère le message
	    		}
	    		
		    	else {
		      		
		      		if (ecrireLigne(sock, ligne) == -1){		// Envoi message au serveur 
					erreur_IO("ecriture socket");
		      		}
				ligne[strlen(ligne)-1] = '\0';		// Permutation \n avec \0
				
		      		if (strcmp(ligne,"fin") == 0){  // Fin envoi message au destinataire	
					fin = 1;	
	      			}
		    	}	
	  	}
  	}
  	pthread_exit(NULL);
}

unsigned long int hashing(char *chaine){	// Fonction de hachage pour "client" et affichage coloré
    unsigned long int hashKey = 0;
    int k =0;
    while(chaine[k] != '>'){
	hashKey += chaine[k]*pow(128,k); //
	k++;

    }
    hashKey = hashKey % 7; //Retourne un nombre entre 0 et 7
    return hashKey;
}



int main(int argc, char *argv[]) {

	/* INIT. SOCKET + SEMAPHORES */
	
	int sock, ret;
	struct sockaddr_in *adrServ;

	sem_init(&semLect,0,0);
	sem_init(&semEcrit,0,0);
	
	
	/******************************/
	
	
	/* Choix nom du client */

	char *pseudo = malloc(sizeof(char)*LIGNE_MAX);
	system("clear");

	printf("Votre nom.prenom en min. : ");

	fgets(pseudo,MAX_STR,stdin);
	pseudo[strlen(pseudo)-1] = '\0';



  	
  	
  	/* CONNEXION AVEC LE SERVEUR */
  	
  	signal(SIGPIPE, SIG_IGN);

	if (argc != 3)
		erreur("usage: %s machine port\n", argv[0]);

	printf("%s: creating a socket\n", CMD);
	sock = socket (AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
		erreur_IO("socket");

	printf("%s: DNS resolving for %s, port %s\n", CMD, argv[1], argv[2]);
	adrServ = resolv(argv[1], argv[2]);
	if (adrServ == NULL)
		erreur("adresse %s port %s inconnus\n", argv[1], argv[2]);

	printf("%s: adr %s, port %hu\n", CMD,
	       stringIP(ntohl(adrServ->sin_addr.s_addr)),
	       ntohs(adrServ->sin_port));

	printf("%s: connecting the socket...", CMD);
	ret = connect(sock, (struct sockaddr *)adrServ, sizeof(struct sockaddr_in));
	if (ret < 0){
		erreur_IO("connect");
	}
  
  	/************************************************/
  	
  	
  	/* ATTENTE DE VALIDATION - PLACES LIBRES COTE SERVEUR */
  	
	char connexionValidation[LIGNE_MAX] = "NO";
  
	while(strcmp(connexionValidation,"NO") == 0){
		lireLigne(sock,connexionValidation);
	}
  	printf("OK\n\n");

	
	/******************************************************/
	
	
	// Envoi du pseudo client au serveur 
  	ecrireLigne(sock,pseudo);
  

  	/* DEMARRAGE THREAD LECTURE / ECRITURE DU CLIENT = COMMUNICATION AVEC SERVEUR */
  	
	pseudoSock structClient;
	structClient.sock = sock;
	strcpy(structClient.pseudo,pseudo);
  

  
	pthread_t id_Lecture;
	pthread_create(&id_Lecture, NULL, threadLecture, &structClient);

	pthread_t id_Ecriture;
	pthread_create(&id_Ecriture, NULL, threadEcriture, &structClient);  
	
	/******************************************************************************/
	
	
	stop = 0;
	while(!stop){ 	// Condition d'arrêt de la session client 
	}
	printf("Connection closed.\n");
	if (close(sock) == -1)		// Fermeture socket
		erreur_IO("fermeture socket");

	exit(EXIT_SUCCESS);
}





	
