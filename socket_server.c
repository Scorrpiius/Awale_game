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
  bool finNormal;

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
  if (buffer == NULL)
  {
    perror("Erreur d'allocation mémoire");
    exit(1);
  }

  snprintf(buffer, 21, "\x1b[33m[%.2d:%.2d:%.2d]\x1b[0m", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

  return buffer;
}

int trouverIndiceCsv(char *pseudo)
{
  FILE *fichierCSV = fopen("liste_joueurs.csv", "r");
  if (fichierCSV == NULL)
  {
    perror("Erreur lors de l'ouverture du fichier");
    return -1; // Retourne -1 en cas d'erreur
  }

  char ligne[256];
  int indice = 0;

  while (fgets(ligne, sizeof(ligne), fichierCSV) != NULL)
  {
    char *pseudoDansLigne = strtok(ligne, ";");

    if (pseudoDansLigne != NULL && strcmp(pseudoDansLigne, pseudo) == 0)
    {
      fclose(fichierCSV);
      return indice;
    }

    indice++;
  }

  fclose(fichierCSV);
  return -1; // Retourne -1 si le pseudo n'est pas trouvé
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

void ecrireListeJoueur(Joueur *j)
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

void updateBiographie(Joueur *j)
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

    // Créer un fichier temporaire pour stocker les données mises à jour
    FILE *tempFile = fopen("temp.csv", "w");
    if (tempFile == NULL)
    {
      perror("Erreur lors de la création du fichier temporaire");
      fclose(fic);
    }

    char ligne[1024];

    // Lire le fichier ligne par ligne
    while (fgets(ligne, sizeof(ligne), fic))
    {
      char *pseudo = strtok(ligne, ";");
      char *biographie = strtok(NULL, ";");
      char *nbVictoires = strtok(NULL, ";");
      char *connecte = strtok(NULL, ";");

      // Comparer le pseudo de la ligne actuelle avec le pseudo à mettre à jour
      if (strcmp(pseudo, j->pseudo) == 0)
      {
        // Si le pseudo correspond, mettre à jour la biographie
        fprintf(tempFile, "%s;%s;%d;%d;\n", j->pseudo, j->biographie, j->nbVictoires, j->connecte);
      }
      else
      {
        // Si le pseudo ne correspond pas, conserver la ligne telle quelle
        fprintf(tempFile, "%s;%s;%s;%s;\n", pseudo, biographie, nbVictoires, connecte);
      }
    }

    // Fermer les fichiers
    fclose(fic);
    fclose(tempFile);

    // Supprimer l'ancien fichier CSV et renommer le fichier temporaire
    remove("liste_joueurs.csv");
    rename("temp.csv", "liste_joueurs.csv");

    printf("%s \x1b[1m\"%s\"\x1b[0m a mis à jour sa biographie.\n", getHeure(), j->pseudo);
  }
}

void updateListeJoueur(int indiceJoueur, Joueur *j)
{
  FILE *fic;

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

void initPartie(char *pseudo1, char *pseudo2)
{
  Partie p;
  p.finNormal = true;
  strcpy(p.pseudoJoueur1, pseudo1);
  strcpy(p.pseudoJoueur2, pseudo2);
  initPlateau(&p.scoreJoueur1, &p.scoreJoueur2, p.plateau);

  // tour du premier joueur au hasard
  srand(time(NULL));
  p.tourJoueur = rand() % 2 + 1;

  listePartiesEnCours[nbPartiesEnCours] = p;
  nbPartiesEnCours++;
  printf("%s Début d'une nouvelle partie entre \x1b[1m\"%s\"\x1b[0m et \x1b[1m\"%s\"\x1b[0m\n", getHeure(), p.pseudoJoueur1, p.pseudoJoueur2);
  // printf("DEBUG PARTIE : %s / %s\n", p.pseudoJoueur1, p.pseudoJoueur2);

  /*// DEBUG : afficher la liste des parties en cours pour vérifier que celle-ci a bien été créée
  for (int i = 0; i < nbPartiesEnCours; i++)
  {
    printf("Partie en cours entre %s et %s.\n", listePartiesEnCours[i].pseudoJoueur1, listePartiesEnCours[i].pseudoJoueur2);
  }*/
}

char lectureMessageClient(int *sockfd)
{
  // Caractère lu
  char c;
  // Caractère retourné par la fonction
  char retour;
  while (1)
  {

    int error = read(*sockfd, &c, 1);
    if (error == 0)
    {
      printf("%s Joueur déconnecté anormalement\n", getHeure());
      return 'E';
    }
    // printf("%c", c);
    if (c == '\n')
    {
      break;
    }
    retour = c;
  }
  return retour;
}

void detruirePartie(char *pseudoJoueur1, char *pseudoJoueur2)
{

  for (int i = 0; i < nbPartiesEnCours; i++)
  {

    if (strcmp(listePartiesEnCours[i].pseudoJoueur1, pseudoJoueur1) == 0 && strcmp(listePartiesEnCours[i].pseudoJoueur2, pseudoJoueur2) == 0)
    {
      memmove(listePartiesEnCours + i, listePartiesEnCours + i + 1, (nbPartiesEnCours - 1) * sizeof(Partie));
      nbPartiesEnCours--;
      break;
    }
  }
  printf("%s Partie terminée entre \x1b[1m\"%s\"\x1b[0m et \x1b[1m\"%s\"\x1b[0m\n", getHeure(), pseudoJoueur1, pseudoJoueur2);
  /* // DEBUG : afficher la liste des parties en cours pour vérifier que celle-ci a bien été détruite
  for (int i = 0; i < nbPartiesEnCours; i++)
  {
    printf("Partie en cours entre %s et %s.\n", listePartiesEnCours[i].pseudoJoueur1, listePartiesEnCours[i].pseudoJoueur2);
  }*/
}

int jouerPartie(Joueur *j, int *sockfd)
{

  bool partieTrouvee = false;
  Partie *p;
  int numJoueur;

  // trouver la partie que l'on va jouer
  while (!partieTrouvee)
  {

    for (int i = 0; i < nbPartiesEnCours; i++)
    {

      if (strcmp(listePartiesEnCours[i].pseudoJoueur1, j->pseudo) == 0)
      {
        p = &listePartiesEnCours[i];
        partieTrouvee = true;
        numJoueur = 1;
        break;
      }
      else if (strcmp(listePartiesEnCours[i].pseudoJoueur2, j->pseudo) == 0)
      {

        p = &listePartiesEnCours[i];
        partieTrouvee = true;
        numJoueur = 2;
        break;
      }
    }
  }

  // tirage au sort du premier joueur
  if (p->tourJoueur == numJoueur)
  {
    char request[100] = "\nLa partie commence : c'est à votre tour ! \n\0";
    // send(*sockfd, request, strlen(request), 0);
  }
  else
  {
    char request[100] = "\nLa partie commence : c'est au tour de votre adversaire ! \n\0";
    // send(*sockfd, request, strlen(request), 0);
  }

  // printf("TOUR JOUEUR DEBUT %d\n", p->tourJoueur);
  bool finDuJeu = false;
  while (finDuJeu == false && p->finNormal == true)
  {
    // affichage du plateau une fois que l'on a joué
    // afficherPlateau(p->plateau, sockfd, p->scoreJoueur1, p->scoreJoueur2, p->pseudoJoueur1, p->pseudoJoueur2);
    // joueur en attente
    while (p->tourJoueur != numJoueur && p->finNormal == true)
    {
    }
    finDuJeu = finDeJeu(p->plateau, numJoueur, p->scoreJoueur1, p->scoreJoueur2);
    if (finDuJeu == true || p->finNormal == false)
    {
      break;
    }
    // affichage du plateau après le tour de l'adversaire
    afficherPlateau(p->plateau, sockfd, p->scoreJoueur1, p->scoreJoueur2, p->pseudoJoueur1, p->pseudoJoueur2);

    // char request[100] = "\nC'est à votre tour, entrez votre coup : \n\0";
    // send(*sockfd, request, strlen(request), 0);

    // le joueur fait son coup
    int coup;
    int valide;
    do
    {
      char lectureCoup = lectureMessageClient(sockfd);
      if (lectureCoup == 'E')
      {
        printf("%s Fin de la partie dû à la déconnexion anormale de l'un des joueurs\n", getHeure());
        coup = '\0';
        p->finNormal = false;
        return 1;
      }
      coup = atoi(&lectureCoup);

      // verification du coup
      valide = coupValide(p->plateau, coup, numJoueur);
      char valideTransmis[2];
      itoa(valide, valideTransmis, 10);
      send(*sockfd, valideTransmis, strlen(valideTransmis), 0);
    } while (valide != 1 && p->finNormal == true);

    // le coup est joué, le jeu peut être terminé, sinon le tour est changé
    jouerCoup(p->plateau, coup, numJoueur, &p->scoreJoueur1, &p->scoreJoueur2);
    finDuJeu = finDeJeu(p->plateau, numJoueur, p->scoreJoueur1, p->scoreJoueur2);
    p->tourJoueur = (p->tourJoueur == 1) ? 2 : 1;
  }

  if (p->finNormal == true)
  {
    // la partie est finie
    char sortieDeJeu[10] = "FIN";
    char resultat = finDePartie(p->scoreJoueur1, p->scoreJoueur2);
    strcat(sortieDeJeu, &resultat);
    send(*sockfd, sortieDeJeu, strlen(sortieDeJeu), 0);

    // incrémenter le nombre de victoires du gagnant
    if (atoi(&resultat) == numJoueur)
    {
      j->nbVictoires++;
      listeJoueurs[trouverIndiceCsv(j->pseudo)].nbVictoires++;
      updateListeJoueur(trouverIndiceCsv(j->pseudo), &listeJoueurs[trouverIndiceCsv(j->pseudo)]);
    }
  }
  else
  {
    // la partie est finie

    char sortieDeJeu[10] = "FINDEC";
    send(*sockfd, sortieDeJeu, strlen(sortieDeJeu), 0);
  }

  strcpy(j->demandeurDeDefi, "\0");
  return 0;
}

void voirProfil(Joueur *j, int sockfd)
{
  char requestProfil[2048] = "\n--------------- Profil de \x1b[1m";
  strcat(requestProfil, j->pseudo);
  strcat(requestProfil, "\x1b[0m ---------------\n\n\x1b[1mBiographie :\x1b[0m ");
  strcat(requestProfil, j->biographie);
  strcat(requestProfil, "\n\n\x1b[1mNombre de victoires :\x1b[0m ");
  char nbVictoires[4];
  itoa(j->nbVictoires, nbVictoires, 10);
  strcat(requestProfil, nbVictoires);
  strcat(requestProfil, "\n\nTapez n'importe quoi pour revenir au menu.");
  send(sockfd, requestProfil, strlen(requestProfil), 0);

  // Le client peut taper n'importe quoi pour sortir
  char c;
  while (1)
  {
    read(sockfd, &c, 1);
    if (c == '\n')
    {
      break;
    }
  }
}

void voirListePartiesEnCours(Joueur *j, int sockfd)
{
  char requestListePartieEnCours[256] = "\n----- Liste des parties en cours -----\n\n";
  if (nbPartiesEnCours == 0)
  {
    strcat(requestListePartieEnCours, "Il n'y a aucune partie en cours.\n");
  }
  else
  {
    for (int i = 0; i < nbPartiesEnCours; i++)
    {
      char scoreJ1[3];
      char scoreJ2[3];

      itoa(listePartiesEnCours[i].scoreJoueur1, scoreJ1, 10);
      itoa(listePartiesEnCours[i].scoreJoueur2, scoreJ2, 10);

      strcat(requestListePartieEnCours, "   - \033[31m\x1b[1m");
      strcat(requestListePartieEnCours, listePartiesEnCours[i].pseudoJoueur1);
      strcat(requestListePartieEnCours, "\x1b[0m vs. \033[34m\x1b[1m");
      strcat(requestListePartieEnCours, listePartiesEnCours[i].pseudoJoueur2);
      strcat(requestListePartieEnCours, "\x1b[0m | Score : \x1b[32m\x1b[1m");
      strcat(requestListePartieEnCours, scoreJ1);
      strcat(requestListePartieEnCours, "\x1b[0m - \x1b[32m\x1b[1m");
      strcat(requestListePartieEnCours, scoreJ2);
      strcat(requestListePartieEnCours, "\x1b[0m\n");
    }
  }

  strcat(requestListePartieEnCours, "\nTapez n'importe quoi pour revenir au menu.");
  send(sockfd, requestListePartieEnCours, strlen(requestListePartieEnCours), 0);

  // Le client peut taper n'importe quoi pour sortir
  char sortieInput[100];
  int indice = 0;
  char c;
  while (1)
  {
    read(sockfd, &c, 1);
    // printf("%c", c);
    if (c == '\n')
    {
      break;
    }
    // enregistrer le pseudo
    sortieInput[indice] = c;
    indice++;
  }
  sortieInput[indice] = '\0';
}

void app(int scomm)
{
  char c;
  Joueur *j;

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

      j = (Joueur *)malloc(sizeof(Joueur));

      // creation d'un joueur
      j->connecte = 1;
      strcpy(j->biographie, "Votre biographie est vide.");
      j->occupe = false;
      j->nbVictoires = 0;
      strcpy(j->pseudo, pseudoInput);

      printf("%s Ajout du joueur \x1b[1m\"%s\"\x1b[0m dans la base de données\n", getHeure(), j->pseudo);
      ecrireListeJoueur(j);

      printf("%s Connexion acceptée : \x1b[1m\"%s\"\x1b[0m\n", getHeure(), j->pseudo);
      // On notifie le client d'un code de succès (1)
      char c = '1';
      write(scomm, &c, 1);
      validitePseudo = nbJoueurs;
      // listeJoueurs[validitePseudo] = *j;
      lireListeJoueur(listeJoueurs);
      j = &listeJoueurs[validitePseudo];
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

    char requestMenu[512] = "\n--------------- Bonjour \x1b[1m";
    strcat(requestMenu, j->pseudo);

    // Si le joueur a été défié, son menu est différent
    if (strcmp(j->demandeurDeDefi, "\0") != 0)
    {
      strcat(requestMenu, "\x1b[0m ---------------\n\n\x1b[31mLe joueur \x1b[1m");
      strcat(requestMenu, j->demandeurDeDefi);
      strcat(requestMenu, "\x1b[0m\x1b[31m vous a défié !\x1b[0m\n(y pour accepter / n pour refuser) ");
      send(scomm, requestMenu, strlen(requestMenu), 0);
    }
    // Si le joueur n'a pas été défié, son menu est normal
    else
    {
      strcat(requestMenu, "\x1b[0m ---------------\n\n \t\x1b[32m\x1b[1m1\x1b[0m: Défier un joueur \n\t\x1b[32m\x1b[1m2\x1b[0m: Voir son profil \n\t\x1b[32m\x1b[1m3\x1b[0m: Voir la liste des parties en cours \n\t\x1b[32m\x1b[1m4\x1b[0m: Modifer sa biographie \n\t\x1b[32m\x1b[1m5\x1b[0m: Rafraichir le menu\n\t\x1b[32m\x1b[1m6\x1b[0m: Déconnexion\n\0");
      send(scomm, requestMenu, strlen(requestMenu), 0);
    }

    char reponse = lectureMessageClient(&scomm);

    /* Défier un joueur */
    if (reponse == '1')
    {
      lireListeJoueur(listeJoueurs);
      char request[200] = "\n----- Liste des joueurs disponibles -----\n\n\x1b[33m";
      bool joueurTrouve = false;
      for (int i = 0; i < nbJoueurs; i++)
      {
        if (listeJoueurs[i].connecte == 1 && strcmp(listeJoueurs[i].pseudo, j->pseudo) != 0 && listeJoueurs[i].occupe == false)
        {
          joueurTrouve = true;
          strcat(request, "\t- ");
          strcat(request, listeJoueurs[i].pseudo);
          strcat(request, "\n");
        }
      }
      if (!joueurTrouve)
      {
        strcat(request, "Il n'y a aucun joueur disponible.\n");
      }

      strcat(request, "\x1b[0m\nEntrez le pseudo du joueur que vous souhaitez défier (0 pour revenir au menu) :");
      send(scomm, request, strlen(request), 0);

      // lecture du pseudo ou du retour menu
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
      if (pseudoDefiChoisi[0] == '0')
      {
        continue;
      }

      int i;
      char demandeEnvoye = '0';
      bool joueurTrouveDansListe = false;
      Joueur *adversaire;
      for (i = 0; i < nbJoueurs; i++)
      {
        if (strcmp(pseudoDefiChoisi, listeJoueurs[i].pseudo) == 0)
        {
          joueurTrouveDansListe = true;
          if (strcmp(listeJoueurs[i].demandeurDeDefi, "\0") == 0)
          {
            strcpy(listeJoueurs[i].demandeurDeDefi, j->pseudo);
            adversaire = &listeJoueurs[i];
            demandeEnvoye = '1';
            write(scomm, &demandeEnvoye, 1);
          }
          else
          {
            write(scomm, &demandeEnvoye, 1);
          }
          break;
        }
      }
      if (!joueurTrouveDansListe)
      {
        demandeEnvoye = '2';
        write(scomm, &demandeEnvoye, 1);
      }

      if (demandeEnvoye == '1')
      {
        printf("%s \x1b[1m\"%s\"\x1b[0m a défié \x1b[1m\"%s\"\x1b[0m\n", getHeure(), j->pseudo, adversaire->pseudo);
        while (strcmp(adversaire->demandeurDeDefi, "\0") != 0 && adversaire->occupe == false)
        {
        }
        if (strcmp(adversaire->demandeurDeDefi, "\0") == 0)
        {
          char refus = '0';
          write(scomm, &refus, 1);
        }
        else
        {
          char accepte = '1';
          write(scomm, &accepte, 1);
          j->occupe = true;
          initPartie(j->pseudo, adversaire->pseudo);
          int error = jouerPartie(j, &scomm);
          if (error == 1)
          {
            joueurSurMenu = false;
            j->connecte = 0;
            updateListeJoueur(validitePseudo, j);
          }
          detruirePartie(j->pseudo, adversaire->pseudo);
          j->occupe = false;
        }
      }
    }

    /* Voir son profil */
    else if (reponse == '2')
    {
      voirProfil(j, scomm);
    }
    /* Voir la liste des parties en cours */
    else if (reponse == '3')
    {
      voirListePartiesEnCours(j, scomm);
    }
    /* Modifier sa biographie */
    else if (reponse == '4')
    {
      char request[100] = "\nEntrez votre nouvelle biographie : \n";
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
      strcpy(listeJoueurs[trouverIndiceCsv(j->pseudo)].biographie, j->biographie);
      updateBiographie(j);
    }
    else if (reponse == '5')
    {
      continue;
    }
    /* Déconnexion */
    else if (reponse == '6' || reponse == 'E')
    {
      joueurSurMenu = false;
      j->connecte = 0;
      updateListeJoueur(validitePseudo, j);
    }
    /*Le joueur a été défié et accepte*/
    else if (strcmp(j->demandeurDeDefi, "\0") != 0 && reponse == 'y')
    {
      printf("%s \x1b[1m\"%s\"\x1b[0m a accepté la demande de défi de \x1b[1m\"%s\"\x1b[0m\n", getHeure(), j->pseudo, j->demandeurDeDefi);
      j->occupe = true;
      int error = jouerPartie(j, &scomm);
      if (error == 1)
      {
        joueurSurMenu = false;
        j->connecte = 0;
        updateListeJoueur(validitePseudo, j);
      }
      j->occupe = false;
    }
    /* Le joueur a été défié et refuse */
    else if (strcmp(j->demandeurDeDefi, "\0") != 0 && reponse == 'n')
    {
      printf("%s \x1b[1m\"%s\"\x1b[0m a refusé la demande de défi de \x1b[1m\"%s\"\x1b[0m\n", getHeure(), j->pseudo, j->demandeurDeDefi);
      strcpy(j->demandeurDeDefi, "\0");
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
  for (int i = 0; i < nbJoueurs; i++)
  {
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
