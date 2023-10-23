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

typedef struct Joueur
{
  char pseudo[100];
  bool occupe;
  int connecte;
  char biographie[1000];
  int nbVictoires;
  int sock;
  char * demandeurDeDefi;

} Joueur;

Joueur listeJoueurs[200];
int nbJoueurs;

char *getHeure()
{
  time_t rawtime;
  struct tm *timeinfo;
  time(&rawtime);
  timeinfo = localtime(&rawtime);

  char *buffer = (char *)malloc(11); // "hh:mm\0" requires 6 characters
  if (buffer == NULL)
  {
    perror("Memory allocation error");
    exit(1);
  }

  strftime(buffer, 11, "[%H:%M:%S]", timeinfo);

  return buffer;
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

void ecrireListeJoueur(Joueur j)
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
    fprintf(fic, "%s;%s;%d;%d;\n", j.pseudo, j.biographie, j.nbVictoires, j.connecte);
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

void updateListeJoueur(int indiceJoueur, Joueur j)
{
  FILE *fic;

  nbJoueurs = 0;
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
        fprintf(fic, "%s;%s;%d;%d;", j.pseudo, j.biographie, j.nbVictoires, j.connecte);
        break;
      }
    }
  }
  fclose(fic);
}


void app (int scomm){
  char c;
  Joueur j;

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
      char requestValidation[] = "Votre pseudo est : ";

      // creation d'un joueur
      j.connecte = 1;
      strcpy(j.biographie, "Votre biographie est vide. Allez la remplir ! ");
      j.occupe = false;
      j.nbVictoires = 0;
      strcpy(j.pseudo, pseudoInput);
      strcat(requestValidation, j.pseudo);

      printf("%s Ajout du joueur \x1b[1m\"%s\"\x1b[0m dans la base de données\n", getHeure(), j.pseudo);
      ecrireListeJoueur(j);

      printf("%s Connexion acceptée : \x1b[1m\"%s\"\x1b[0m\n", getHeure(), j.pseudo);
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
      j = listeJoueurs[validitePseudo];
      j.connecte = 1;

      printf("%s Connexion acceptée : \x1b[1m\"%s\"\x1b[0m\n", getHeure(), j.pseudo);
      // On notifie le client d'un code de succès (1)
      char c = '1';
      write(scomm, &c, 1);

      updateListeJoueur(validitePseudo, j);
      joueurNonValide = false;
    }
  }


  // On remet à jour la liste des joueurs avec les potentielles modifications (ajout de joueur ou joueur connecté)
  lireListeJoueur(listeJoueurs);
  bool joueurSurMenu = true;
  while (joueurSurMenu)
  {
    char requestMenu[200] = "--------------- Bonjour \x1b[1m";
    strcat(requestMenu, j.pseudo);
    strcat(requestMenu, "\x1b[0m ---------------\n \t1: Défier un joueur \n\t2: Voir son profil \n\t3: Modifer sa biographie \n\t4: Déconnexion\n\0");
    send(scomm, requestMenu, strlen(requestMenu), 0);
    
    
    char reponse;
    char c;
    while (1)
    {

      read(scomm, &c, 1);
      if (c == '\n')
      {
        break;
      }
      reponse = c;

    }


    if (reponse == '1')
    {
      lireListeJoueur(listeJoueurs);
      char request[200] = "Liste des joueurs connectés : \n";
      for(int i = 0; i< nbJoueurs; i++){
        if(listeJoueurs[i].connecte == 1 && strcmp(listeJoueurs[i].pseudo,j.pseudo) != 0)
        {
          strcat(request, listeJoueurs[i].pseudo);
          strcat(request, "\n");
        }
      }
      strcat(request, "Entrez le pseudo du joueur que vous souhaitez défier (0 pour revenir au menu) : \n");
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
      for(i = 0; i<nbJoueurs; i++){
        if(strcmp(pseudoDefiChoisi, listeJoueurs[i].pseudo) == 0){
          if(listeJoueurs[i].demandeurDeDefi != NULL){
              strcpy(listeJoueurs[i].demandeurDeDefi,j.pseudo);
          }
          
           break;
        }
        


      }
      if (i == nbJoueurs -1){
        char buffer[100] = "Le joueur n'existe pas";
        send(scomm, request, strlen(request), 0);
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
      strcpy(j.biographie, buffer);
      updateListeJoueur(validitePseudo, j);
    }else if (reponse == '4')
    {
      joueurSurMenu = false;
      j.connecte = 0;
      updateListeJoueur(validitePseudo, j);
    }
  }
  printf("%s Le joueur \x1b[1m\"%s\"\x1b[0m s'est deconnecté\n", getHeure(), j.pseudo);

  close(scomm);
  exit(0); /* on force la terminaison du fils */

}


int main(int argc, char **argv)
{
  char datas[] = "hello\n";
  int sockfd, scomm, clilen, chilpid, ok, nleft, nbwriten;
  struct sockaddr_in cli_addr, serv_addr;

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

  printf("%s Serveur opérationnel !\n", getHeure());
  while (1)
  {

    scomm = accept(sockfd, NULL, NULL);
    printf("%s Nouvelle connexion reçue\n", getHeure());
    pthread_t t_id;
    int r = pthread_create(&t_id, NULL, app, scomm);
  }

  return 1;
}
