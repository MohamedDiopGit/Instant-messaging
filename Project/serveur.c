#include "pse.h"
#include <math.h>
#define CMD         "serveur"
#define NOM_JOURNAL "journal.log"
#define NB_WORKERS 3
#define TABLE_SIZE 1000
#define MAX_STR 1000

/* Déclaration de structures */

typedef struct MessageStock MessageStock;
struct MessageStock{
	char message[LIGNE_MAX];
	MessageStock *suivant;
};

typedef struct File File;
struct File{
    MessageStock *premier;
};

/**************************************/



/* Déclaration des prototypes */
void creerCohorteWorkers(void);		// Création de NB_WORKERS statiques en attente de connexion
void *threadWorker(void *arg);		// Thread worker pour la session client
void sessionClient(int canal);		// Permet la réception et l'envoi de message entre clients
void remiseAZeroJournal(void);		// Remet a zéro le journal des communications
unsigned long int hashing(char *chaine);		// fonction de hachage
void init_tab(int *tab,int size);			//Prend en argument le tableau pour initialiser ses valeurs à -1
void enFiler(File *file, char message[]);		// Ajoute un élement (message) à une file d'éléments
void deFiler(File *file, int canal);			// Défile entièrement une file de message
File *initialisation();					// Liste des files de messages stockés

/*****************************/


/* Déclaration des variables globales */

int fdJournal;
DataSpec dataWorkers[NB_WORKERS];

int tabConnexion[TABLE_SIZE] = {-1};
File *fileMessage[TABLE_SIZE] = {NULL};

/******************************/


/* Déclaration des sémaphores */

sem_t semLibre;

/******************************/







int main(int argc, char *argv[]) {

	system("clear");
	
	
	/* INITIALISATION DU SERVEUR  */ 
	short port;
	int ecoute, canal, ret;
	struct sockaddr_in adrEcoute, adrClient;
	unsigned int lgAdrClient;
	sem_init(&semLibre,0,NB_WORKERS);		// Initialsation sémaphore pour places libres
  
	for(int i=0; i<TABLE_SIZE; i++){		// Initialisation de la liste des files
		fileMessage[i] = initialisation();
	}
  

	/* Init socket serveur + ecoute */
	if (argc != 2)
		erreur("usage: %s port\n", argv[0]);

	port = (short)atoi(argv[1]);

	fdJournal = open(NOM_JOURNAL, O_CREAT|O_WRONLY|O_APPEND, 0644);
	if (fdJournal == -1)
		erreur_IO("ouverture journal");

	creerCohorteWorkers();

	printf("%s: creating a socket\n", CMD);
	ecoute = socket(AF_INET, SOCK_STREAM, 0);
	if (ecoute < 0)
		erreur_IO("socket");


	adrEcoute.sin_family = AF_INET;
	adrEcoute.sin_addr.s_addr = INADDR_ANY;
	adrEcoute.sin_port = htons(port);

	printf("%s: binding to INADDR_ANY address on port %d\n", CMD, port);
	printf("%s: maximum of %d connections\n", CMD, NB_WORKERS);
	ret = bind(ecoute, (struct sockaddr *)&adrEcoute, sizeof(adrEcoute));
	if (ret < 0)
		erreur_IO("bind");
  
	printf("%s: listening to socket\n\n", CMD);
	ret = listen(ecoute, 5);
	if (ret < 0)
		erreur_IO("listen");
		
	/***************************************/
	
	
	init_tab(tabConnexion,TABLE_SIZE);
	char codeValidation[LIGNE_MAX] = "OK";
  	
  	/**************************************/
  	
  	
  	
	while (VRAI) {
	
		/* ECOUTE SERVEUR + ATTENTE DE CONNEXION */
		
		lgAdrClient = sizeof(adrClient);
		canal = accept(ecoute, (struct sockaddr *)&adrClient, &lgAdrClient);
		printf("\n%s: accepting a connection\n", CMD);
		if (canal < 0){
			erreur_IO("accept");
		}
		printf("%s: adr %s, port %hu\n", CMD,
		stringIP(ntohl(adrClient.sin_addr.s_addr)), ntohs(adrClient.sin_port));

    		
    		/*****************************************/
    		
    		/* 	ATTENTE D'UNE PLACE LIBRE  	*/
    		
		printf("%s: connection waiting for free space...\n", CMD);
		sem_wait(&semLibre);
		ecrireLigne(canal,codeValidation);	// Envoi de l'information place libre au client
		printf("%s: free space found\n", CMD);
		
		/****************************************/
    
    
    		/* Recherche worker libre */
		int i;
		int trouve = 0;

		for (i = 0; i < NB_WORKERS; i++){		// Cherche un worker libre
			if (dataWorkers[i].canal == -1 && trouve == 0){
				dataWorkers[i].canal = canal;
				sem_post(&dataWorkers[i].sem);
       				trouve = 1;
			}
		}
		
		/***************************/
    
	}
	
	if (close(ecoute) == -1)		// Fermeture ecoute serveur
		erreur_IO("fermeture ecoute");

	if (close(fdJournal) == -1)
		erreur_IO("fermeture journal");
	exit(EXIT_SUCCESS);
}


void creerCohorteWorkers(void) {
  int i, ret;

  for (i = 0; i < NB_WORKERS; i++) {
	dataWorkers[i].canal = -1;
	dataWorkers[i].tid = i;
	sem_init(&dataWorkers[i].sem, 0, 0);
	ret = pthread_create(&dataWorkers[i].id, NULL, threadWorker, &dataWorkers[i]);
	if (ret != 0)
		erreur_IO("creation worker");
	}
}


void *threadWorker(void *arg) {
	DataSpec *dataSpec = (DataSpec *)arg; 
	while (VRAI) {
		sem_wait(&dataSpec->sem);
		//printf("worker %d: reveil\n", dataSpec->tid);
		sessionClient(dataSpec->canal);		// Démarrage de la session client
		//printf("worker %d: sommeil\n", dataSpec->tid);
		dataSpec->canal = -1;	// Canal client à -1s
		sem_post(&semLibre);
	}
	pthread_exit(NULL);
}

unsigned long int hashing(char *chaine){
	unsigned long int hashKey = 0;
	int k =0;

	while(chaine[k] != '\0' ){
		hashKey += chaine[k]*pow(128,k); //
		k++;
	}
	hashKey = hashKey % TABLE_SIZE; //Retourne un nombre entre 0 et TABLE_SIZE
	return hashKey;
}

void init_tab(int *tab,int size){  // Init la table de connexion avec -1
 	for(int i=0; i<size; i++)
 		tab[i] = -1;
}


/* Initialisation d'une file de message */

File *initialisation(){
    File *file = malloc(sizeof(*file)); //Allocation
    if (file == NULL) //verification allocation
    {
	printf("exit");
	exit(EXIT_FAILURE);
    }

    file->premier = NULL; // Init. premier élément
    return file;
}


void enFiler(File *file, char message[]){
	MessageStock *nouveauMsg = malloc(sizeof(*nouveauMsg));
	if (file == NULL || nouveauMsg == NULL) //verification allocation
	{
		printf("exit");
		exit(EXIT_FAILURE);
	}

	strcpy(nouveauMsg->message,message);
	nouveauMsg->suivant = NULL;

	if(file->premier != NULL){
		MessageStock *actuel = file->premier;
		while(actuel->suivant != NULL){
			actuel = actuel->suivant;
		}
		actuel->suivant = nouveauMsg;
	}
	else{
		file->premier = nouveauMsg;
	}
}


void deFiler(File *file, int canal){
	if (file == NULL) //verification allocation
	{
		printf("exit");
		exit(EXIT_FAILURE);
	}

	char message[LIGNE_MAX];
	while(file->premier != NULL)		// Envoi les messages de la file du dernier de liste au premier (ordre chronol.)
	{
		MessageStock *elementDeFile = file->premier;
		strcpy(message,elementDeFile->message);
		ecrireLigne(canal,message);		// Envoi dans le canal
		file->premier = elementDeFile->suivant;
		free(elementDeFile);	// Suppression de l'élément pour libérer la mémoire
	}
}



void sessionClient(int canal) {

	int fin = 0;
	char ligne[LIGNE_MAX];
	char emetteur[LIGNE_MAX];		// Client
	char destinataire[LIGNE_MAX];		// destinataire du client
	int lgLue, lgEcr;

	lireLigne(canal,emetteur);		// LECTURE PSEUDO CLIENT
	printf("%s: connexion client : %s\n",CMD,emetteur);  

	unsigned long int hashEmetteur = hashing(emetteur);
	tabConnexion[hashEmetteur] = canal;	// Correspondance client - Canal dans une table

	unsigned long int hashDestinataire;
	char message[MAX_STR];	// Message total à envoyer au destinataire = "client>message"
	int stop = 0;
	char etat[MAX_STR];	// Etat destinataire 
  
  
	while(!stop){		// Communication client server tant que pas "stop" côté client
	
		fin = 0;
		lireLigne(canal,destinataire);	// Récupère message de l'émetteur = Choix du destinataire pour message
		if(strcmp(destinataire,"stop") == 0){		// Fin de communication client - serveur
			tabConnexion[hashEmetteur] = -1;	// Client déconnecté
			stop = 1;
		}
		else{		// vérification client disponible
			hashDestinataire = hashing(destinataire);
			if(tabConnexion[hashDestinataire] == -1){	// Destinataire du client = indisponible
				strcpy(etat,"offline");
				printf("Destinataire : %s, indisponible.\n",destinataire);
				ecrireLigne(canal,etat);	// Envoi de l'information au client
			}
			else{	// Destinataire connecté / disponible
				strcpy(etat,"online");
				ecrireLigne(canal,etat);	// Envoi de l'information connecté au client 
				tabConnexion[hashEmetteur] = canal;	// Disponibilité du client
				deFiler(fileMessage[hashEmetteur],canal);	// Envoi des messages stocké pendant l'indisponibilité au client
				
				while (!fin) {	// Session d'envoi message client à destinataire
					
					lgLue = lireLigne(canal, ligne);	// Lecture du message
					strcpy(message,emetteur);
					strcat(message,">");
					strcat(message,ligne);		// Nouveau message = "client>message"
					if (lgLue == -1)
						erreur_IO("lecture canal");

					else {  // lgLue > 0
						if (strcmp(ligne, "fin") == 0) {	// Réception message fin --> fin de la session client -> destinataire
							fin = 1;
							printf("%s: fin session %s -> %s\n", CMD,emetteur,destinataire);
					  	}
					  	else if (strcmp(ligne, "init") == 0) {	// RAZ du journal de communication
							remiseAZeroJournal();
							printf("%s: remise a zero du journal\n", CMD);
					 	}
					  	else {	// Envoi du message
							lgEcr = ecrireLigne(fdJournal, message); 	// Dans le journal
							
							if(tabConnexion[hashDestinataire] == -1){	// Cas 1 : le destinataire est à nouveau indisponible
								enFiler(fileMessage[hashDestinataire],message);		// Stockage des messages côté serveur dans une file correspondant au destinataire
							}
							else{
								ecrireLigne(tabConnexion[hashDestinataire],message);	// Transmission du message au destinataire par le serveur
							}
							if (lgEcr < 0)
							  	erreur_IO("ecriture journal");
							printf("%s: %d octets - %s -> %s: %s\n",CMD,lgEcr,emetteur,destinataire,ligne);
						}
					}
				}
				tabConnexion[hashEmetteur] = -1;	// Fin de la session : Client = indisponible pour ne pas recevoir de message en même qu'il selectionne son destinataire
			}
		}

	}
	printf("%s: deconnexion client : %s\n",CMD,emetteur); 	// Fin de session client = deconnexion
	if (close(canal) == -1)
		erreur_IO("fermeture canal");
}

void remiseAZeroJournal(void) {
	if (close(fdJournal) < 0)
		erreur_IO("fermeture journal pour remise a zero");
	fdJournal = open("journal.log", O_TRUNC|O_WRONLY|O_APPEND);
	if (fdJournal < 0)
		erreur_IO("reouverture journal");
}
