/* Serveur sockets TCP
 * affichage de ce qui arrive sur la socket
 *    socket_server port (port > 1024 sauf root)
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <time.h>
#include "awale.h"

typedef struct Joueur
{
  char pseudo[100];
  bool occupe;
  int connecte;
  char biographie[1000];
  int nbVictoires;
  int sock;
  char demandeurDeDefi[100];

} Joueur;

typedef struct Partie
{
  char pseudoJoueur1[100];
  char pseudoJoueur2[100];
  int plateau[12];
  int scoreJoueur1;
  int scoreJoueur2;
  int tourJoueur;
  pthread_mutex_t mutex;

} Partie;

Joueur listeJoueurs[200];
Partie listePartiesEnCours[100];
int nbPartiesEnCours;
int nbJoueurs;

char *getHeure()
{
    time_t rawtime;
    struct tm *timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);

    char *buffer = (char *)malloc(21); // "[\x1b[33mHH:MM:SS\x1b[0m]\0" requiert 21 caractères
    if (buffer == NULL) {
        perror("Erreur d'allocation mémoire");
        exit(1);
    }

    snprintf(buffer, 21, "\x1b[33m[%.2d:%.2d:%.2d]\x1b[0m", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

    return buffer;
}

void initPartie(char* pseudo1, char* pseudo2){
  Partie p;
  strcpy(p.pseudoJoueur1, pseudo1);
  strcpy(p.pseudoJoueur2, pseudo2);
  initPlateau(&p.scoreJoueur1, &p.scoreJoueur2, p.plateau);

  //tour du premier joueur au hasard
  srand(time(NULL));
  p.tourJoueur = rand() % 2 + 1;

  //printf("TEST RAND : %d\n", tourJoueur);
  listePartiesEnCours[nbPartiesEnCours] = p;
  nbPartiesEnCours++;
  printf("%s Début d'une nouvelle partie entre \x1b[1m\"%s\"\x1b[0m et \x1b[1m\"%s\"\x1b[0m\n", getHeure(), p.pseudoJoueur1, p.pseudoJoueur2);
  //printf("DEBUG PARTIE : %s / %s\n", p.pseudoJoueur1, p.pseudoJoueur2);
}

char lectureMessageClient(int * sockfd){
  //Caractère lu
  char c;
  //Caractère retourné par la fonction
  char retour;
  while (1)
    {

      read(*sockfd, &c, 1);
      // printf("%c", c);
      if (c == '\n')
      {
        break;
      }
      retour = c;
    }
    return retour;
}

void jouerPartie(Joueur * j, int *sockfd){

 bool partieTrouvee = false;
  Partie * p;
  int numJoueur;

  //trouver la partie que l'on va jouer
  while(!partieTrouvee){

    for(int i = 0; i< nbPartiesEnCours; i++){

      if(strcmp(listePartiesEnCours[i].pseudoJoueur1, j->pseudo) == 0 ){
        p = &listePartiesEnCours[i];
        partieTrouvee = true;
        numJoueur = 1;
        break;

      }else if(strcmp(listePartiesEnCours[i].pseudoJoueur2, j->pseudo)== 0){
        
        p = &listePartiesEnCours[i];
        partieTrouvee = true;
        numJoueur = 2;
        break;

      }
    }
  }


  //tirage au sort du premier joueur
  if(p->tourJoueur == numJoueur){
    char request[100] = "\nLa partie commence : c'est à votre tour ! \n\0";
    //send(*sockfd, request, strlen(request), 0);
  }else{
    char request[100] = "\nLa partie commence : c'est au tour de votre adversaire ! \n\0";
    //send(*sockfd, request, strlen(request), 0);
  }

  //printf("TOUR JOUEUR DEBUT %d\n", p->tourJoueur);
  bool finDuJeu = false;
  while(!finDuJeu){
    //affichage du plateau une fois que l'on a joué
    //afficherPlateau(p->plateau, sockfd, p->scoreJoueur1, p->scoreJoueur2, p->pseudoJoueur1, p->pseudoJoueur2);
    //joueur en attente
    while(p->tourJoueur != numJoueur){}
    //affichage du plateau après le tour de l'adversaire
    afficherPlateau(p->plateau, sockfd, p->scoreJoueur1, p->scoreJoueur2, p->pseudoJoueur1, p->pseudoJoueur2);

    //char request[100] = "\nC'est à votre tour, entrez votre coup : \n\0";
    //send(*sockfd, request, strlen(request), 0);

    //le joueur fait son coup
    int coup;
    int valide;
    do{
      char lectureCoup =  lectureMessageClient(sockfd);
      coup = atoi(&lectureCoup);

      //verification du coup 
      valide = coupValide(p->plateau, coup, numJoueur); 
      char valideTransmis[2];
      itoa(valide, valideTransmis, 10);
      send(*sockfd, valideTransmis, strlen(valideTransmis), 0);
    }while(valide != 1);

    //le coup est joué, le jeu peut être terminé, sinon le tour est changé
    jouerCoup(p->plateau, coup, numJoueur, &p->scoreJoueur1, &p->scoreJoueur2);
    finDuJeu = finDeJeu(p->plateau, numJoueur, p->scoreJoueur1, p->scoreJoueur2);
    p->tourJoueur = (p->tourJoueur == 1) ? 2 : 1;


  }
  //la partie est finie
  finDePartie(p->scoreJoueur1, p->scoreJoueur2);
  
}

// la méthode renvoie -2 si le joueur n'existe pas;
// l'indice du joueur dans la liste si le joueur a déjà créé son compte et n'est pas connecté ;
// -1 si un autre joueur est connecté avec le même pseudo
int joueurExistant(char *pseudoNouveauJoueur)
{

  for (int i = 0; i < nbJoueurs; i++)
  {
    Joueur j = listeJoueurs[i];
    if (strcmp(pseudoNouveauJoueur, j.pseudo) == 0)
    {
      if (j.connecte == 0)
      {
        return i;
      }
      else
      {
        return -1;
      }
    }
  }
  return -2;
}

void ecrireListeJoueur(Joueur* j)
{
  FILE *fic;

  // ouverture du fic de données CSV
  int indice = 0;
  fic = fopen("liste_joueurs.csv", "r+");
  if (fic == NULL)
  {
    printf("Ouverture fic impossible !");
  }
  else
  {
    fseek(fic, 0, SEEK_END);
    fprintf(fic, "%s;%s;%d;%d;\n", j->pseudo, j->biographie, j->nbVictoires, j->connecte);
  }
  fclose(fic);
}

void lireListeJoueur(Joueur *listeJoueurParam)
{
  FILE *fic;

  nbJoueurs = 0;
  // ouverture du fic de données CSV
  int indice = 0;
  fic = fopen("liste_joueurs.csv", "r");
  if (fic == NULL)
  {
    printf("Ouverture fic impossible !");
  }
  else
  {
    char c;
    bool fileEnd = false;

    int indice = 0;
    while (!fileEnd)
    {
      char ligne[300];
      int index = 0;
      while ((c = fgetc(fic)) != '\n')
      {
        ligne[index] = c;
        index++;
      }
      ligne[index] = '\0';
      char *infosJoueur = strtok(ligne, ";");
      strcpy(listeJoueurParam[indice].pseudo, infosJoueur);
      infosJoueur = strtok(NULL, ";");
      strcpy(listeJoueurParam[indice].biographie, infosJoueur);
      infosJoueur = strtok(NULL, ";");
      listeJoueurParam[indice].nbVictoires = atoi(infosJoueur);
      infosJoueur = strtok(NULL, ";");
      listeJoueurParam[indice].connecte = atoi(infosJoueur);

      //printf("%s , bio : %s \n", listeJoueurParam[indice].pseudo, listeJoueurParam[indice].biographie);

      if ((c = fgetc(fic)) == EOF)
      {
        fileEnd = true;
        break;
      }
      fseek(fic, -1L, SEEK_CUR);
      indice++;
    }
    nbJoueurs = indice + 1;
  }
  fclose(fic);
}

void updateListeJoueur(int indiceJoueur, Joueur* j)
{
  FILE *fic;

  //nbJoueurs = 0;
  // ouverture du fic de données CSV
  fic = fopen("liste_joueurs.csv", "r+");
  if (fic == NULL)
  {
    printf("Ouverture fic impossible !");
  }
  else
  {
    int ligneCourante = -1;
    char ligne[1024];

    while (fgets(ligne, sizeof(ligne), fic))
    {
      ligneCourante++;
      if (ligneCourante == indiceJoueur)
      {
        // Remplace la nième ligne par les données
        fseek(fic, -strlen(ligne), SEEK_CUR); // Mettre le curseur au début de la ligne
        fprintf(fic, "%s;%s;%d;%d;", j->pseudo, j->biographie, j->nbVictoires, j->connecte);
        break;
      }
    }
  }
  fclose(fic);
}


void app (int scomm){
  char c;
  Joueur * j;

  int validitePseudo;
  /* traiter la communication */

  bool joueurNonValide = true;
  while (joueurNonValide)
  {
    // demander pseudo à joueur
    char requestPseudo[] = "Veuillez entrer votre pseudo, svp.";

    send(scomm, requestPseudo, strlen(requestPseudo), 0);

    // recevoir le pseudo de notre cher client
    char pseudoInput[100];
    int indice = 0;
    // printf("%s Pseudo reçu : ", getHeure());
    while (1)
    {

      read(scomm, &c, 1);
      // printf("%c", c);
      if (c == '\n')
      {
        break;
      }
      // enregistrer le pseudo
      pseudoInput[indice] = c;
      indice++;
    }
    pseudoInput[indice] = '\0';
    printf("%s Pseudo reçu : \x1b[1m\"%s\"\x1b[0m\n", getHeure(), pseudoInput);

    lireListeJoueur(listeJoueurs);
    validitePseudo = joueurExistant(pseudoInput);

    // Cas 1 : joueur non existant -> on créée un nouveau joueur
    if (validitePseudo == -2)
    {
      //char requestValidation[] = "Votre pseudo est :\0";
      
      j = (Joueur*)malloc(sizeof(Joueur));

      // creation d'un joueur
      printf("bien rentré1\n");
      j->connecte = 1;
      printf("bien rentré2\n");
      strcpy(j->biographie, "Votre biographie est vide. Allez la remplir ! ");
      j->occupe = false;
      printf("bien rentré3\n");
      j->nbVictoires = 0;
      strcpy(j->pseudo, pseudoInput);
      printf("bien rentré4\n");
      //strcat(requestValidation, j->pseudo);
      

      printf("%s Ajout du joueur \x1b[1m\"%s\"\x1b[0m dans la base de données\n", getHeure(), j->pseudo);
      ecrireListeJoueur(j);

      printf("%s Connexion acceptée : \x1b[1m\"%s\"\x1b[0m\n", getHeure(), j->pseudo);
      // On notifie le client d'un code de succès (1)
      char c = '1';
      write(scomm, &c, 1);
      validitePseudo = nbJoueurs;
      joueurNonValide = false;
    }
    // Cas 2 : joueur existant et connecté -> on refuse la connexion
    else if (validitePseudo == -1)
    {
      printf("%s Connexion refusée : le joueur \x1b[1m\"%s\"\x1b[0m est déjà connecté\n", getHeure(), pseudoInput);
      // On notifie le client d'un code d'erreur (0)
      char c = '0';
      write(scomm, &c, 1);
    }
    // Cas 3 : joueur existant et déconnecté -> on accepte la connexion
    else
    {
      j = &listeJoueurs[validitePseudo];
      j->connecte = 1;

      printf("%s Connexion acceptée : \x1b[1m\"%s\"\x1b[0m\n", getHeure(), j->pseudo);
      // On notifie le client d'un code de succès (1)
      char c = '1';
      write(scomm, &c, 1);

      updateListeJoueur(validitePseudo, j);
      joueurNonValide = false;
    }
  }


  bool joueurSurMenu = true;
  while (joueurSurMenu)
  {
      // On remet à jour la liste des joueurs avec les potentielles modifications (ajout de joueur ou joueur connecté)
      lireListeJoueur(listeJoueurs);
    
      char requestMenu[200] = "\n--------------- Bonjour \x1b[1m";
      strcat(requestMenu, j->pseudo);
      if(strcmp(j->demandeurDeDefi, "\0") != 0){
        strcat(requestMenu, "\x1b[0m ---------------\n\nLe joueur ");
        strcat(requestMenu, j->demandeurDeDefi);
        strcat(requestMenu, " vous a défié ! (y pour accepter / n pour refuser) ");
        send(scomm, requestMenu, strlen(requestMenu), 0);
      }else{
        strcat(requestMenu, "\x1b[0m ---------------\n\n \t1: Défier un joueur \n\t2: Voir son profil \n\t3: Modifer sa biographie \n\t4: Rafraichir le menu\n\t5: Déconnexion\n\0");
        send(scomm, requestMenu, strlen(requestMenu), 0);
      }

      char reponse = lectureMessageClient(&scomm);


      if (reponse == '1')
      {
        lireListeJoueur(listeJoueurs);
        char request[200] = "Liste des joueurs connectés :\n\n";
        bool joueurTrouve = false;
        for(int i = 0; i< nbJoueurs; i++){
          if(listeJoueurs[i].connecte == 1 && strcmp(listeJoueurs[i].pseudo,j->pseudo) != 0)
          {
            joueurTrouve = true;
            strcat(request, "\t");
            strcat(request, listeJoueurs[i].pseudo);
            strcat(request, "\n");
          }
        }
        if (!joueurTrouve) {
          strcat(request, "\tIl n'y a aucun autre joueur connecté.\n");
        }

        strcat(request, "\nEntrez le pseudo du joueur que vous souhaitez défier (0 pour revenir au menu) :");
        send(scomm, request, strlen(request), 0);

        //lecture du pseudo ou du retour menu
        char pseudoDefiChoisi[100];
        int indice = 0;
        while (1)
        {

          read(scomm, &c, 1);
          if (c == '\n')
          {
            break;
          }
          pseudoDefiChoisi[indice] = c;
          indice++;

        }
        if(pseudoDefiChoisi[0] == '0'){
            continue;
        }

        int i;
        char demandeEnvoye = '0';
        bool joueurTrouveDansListe = false;
        Joueur *adversaire;
        for(i = 0; i<nbJoueurs; i++){
          if(strcmp(pseudoDefiChoisi, listeJoueurs[i].pseudo) == 0){
            joueurTrouveDansListe = true;
            if(strcmp(listeJoueurs[i].demandeurDeDefi, "\0")==0){
                strcpy(listeJoueurs[i].demandeurDeDefi,j->pseudo);
                /*char buffer[100] = "Votre demande a bien été envoyée \n\0";
                send(scomm, buffer, strlen(buffer), 0);*/
                adversaire = &listeJoueurs[i];
                demandeEnvoye = '1';
                write(scomm, &demandeEnvoye, 1);
            }else{
              /*char buffer[100] = "Le joueur est déjà défié...\n\0";
              send(scomm, buffer, strlen(buffer), 0);*/
              write(scomm, &demandeEnvoye, 1);
            }
            break;
          }
        }
        if (!joueurTrouveDansListe){
          printf("rentré au mauvais endoit...\n");
          /*char buffer[100] = "Le joueur n'existe pas";
          send(scomm, buffer, strlen(buffer), 0);*/
          demandeEnvoye = '2';
          write(scomm, &demandeEnvoye, 1);
        }

        if(demandeEnvoye == '1'){
          while(strcmp(adversaire->demandeurDeDefi, "\0") != 0 && adversaire->occupe == false){ }
          if(strcmp(adversaire->demandeurDeDefi, "\0") == 0){
            char refus = '0';
            /*char buffer[100] = " Le joueur a refusé votre demande...\n\0";
            send(scomm, buffer, strlen(buffer), 0);*/
            write(scomm, &refus, 1);
          }else{
            char accepte = '1';
            /*char buffer[100] = " Le joueur a accepté votre demande, la partie va commencer !\n\0";
            send(scomm, buffer, strlen(buffer), 0);*/
            write(scomm, &accepte, 1);
            j->occupe = true; 
            initPartie(j->pseudo, adversaire->pseudo);
            jouerPartie(j, &scomm);
          }
        }
          
      }else if (reponse == '3')
      {
        char request[100] = "Entrez votre nouvelle biographie : \n";
        send(scomm, request, strlen(request), 0);
        
        int indice = 0;
        char buffer[100];
        while (1)
        {
          read(scomm, &c, 1);
          if (c == '\n')
          {
            break;
          }
          // enregistrer le pseudo
          buffer[indice] = c;
          indice++;
        }
        buffer[indice] = '\0';
        strcpy(j->biographie, buffer);
        updateListeJoueur(validitePseudo, j);
      }
      else if (reponse == '5')
      {
        joueurSurMenu = false;
        j->connecte = 0;
        updateListeJoueur(validitePseudo, j);
      }else if (reponse == 'y')
      {
        j->occupe = true;
        jouerPartie(j, &scomm);
      }else if (reponse == 'n')
      {
        strcpy(j->demandeurDeDefi,"\0");
      }
}
    
  printf("%s Le joueur \x1b[1m\"%s\"\x1b[0m s'est deconnecté\n", getHeure(), j->pseudo);

  close(scomm);

}


int main(int argc, char **argv)
{
  char datas[] = "hello\n";
  int sockfd, scomm, clilen, chilpid, ok, nleft, nbwriten;
  struct sockaddr_in cli_addr, serv_addr;

  nbPartiesEnCours = 0;

  if (argc != 2)
  {
    printf("usage: socket_server port\n");
    exit(0);
  }

  printf("%s Démarrage du serveur...\n", getHeure());

  /* ouverture du socket */
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
  {
    printf("%s ERREUR : Impossible d'ouvrir le socket\n", getHeure());
    exit(0);
  }

  /* initialisation des parametres */
  bzero((char *)&serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(atoi(argv[1]));

  /* effecture le bind */
  if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
  {
    printf("%s ERREUR : Impossible de faire le bind\n", getHeure());
    exit(0);
  }

  /* petit initialisation */
  listen(sockfd, 1);

  /* On met 'connecte' à 0 à tous les joueurs dans le .csv par précaution
  (il ne peut pas y avoir de joueur connecté avant le démarrage du serveur) */
  lireListeJoueur(listeJoueurs);
  for (int i = 0 ; i < nbJoueurs ; i++) {
    listeJoueurs[i].connecte = 0;
    updateListeJoueur(i, &listeJoueurs[i]);
  }

  printf("%s Serveur opérationnel !\n", getHeure());
  while (1)
  {
    // Nouvelle connexion -> nouveau thread
    scomm = accept(sockfd, NULL, NULL);
    printf("%s Nouvelle connexion reçue\n", getHeure());
    pthread_t t_id;
    int r = pthread_create(&t_id, NULL, app, scomm);
  }

  return 1;
}
