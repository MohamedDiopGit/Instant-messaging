Pour faire tourner le projet :

Faire make en préalable

1:
	Faire tourner le serveur en cliquant sur dossier "Projet" puis ouvrir un terminal
	Faire "./serveur 2000" avec 2000 le nom du port (2001,2002,... ou autre)

2:
	Faire tourner programme client correspondant à une personne se connectant
	en cliquant sur dossier "Projet" puis ouvrir un terminal :
	"./client localhost 2000" avec localhost (ou adresse ip du serveur - machine hote) et 2000 par ex ou le nom du port choisi)


NB : Le nombre maximal de clients pouvant se connecter au serveur est de 20 par défaut, réglage avec NB_WORKERS dans le code source de serveur.c en début de fichier.


Lorsqu'on se connecte côté client, il demande votre nom : 
	mettre "marc" par ex.

Conseil : 
	Faire démarrer 2 clients avec deux noms différents pour pouvoir communiquer et tester le programme.

Un fois les deux personnes connectées (avant le choix destinataire), il est possible de dialoguer :
	Choisir le nom du destinataire dans les deux clients
	taper les lignes de commandes pour parler
	
	Une fin de communication "client" -> destinataire se fait avec "fin"

Pour arrêter un client :
	taper "stop" lorsqu'on demande "Destinataire ? : "
