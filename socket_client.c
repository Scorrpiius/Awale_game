/* Client pour les sockets
 *    socket_client ip_server port
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFSIZE 1024

/*Le client rentre un caractère attendu en réponse par le serveur*/
char rentrerReponse(int *sockfd)
{
  char c, retour;
  while (1)
  {
    c = getchar();
    write(*sockfd, &c, 1);
    if (c == '\n')
      break;
    retour = c;
  }
  return retour;
}

/*Le client affiche ce que le serveur lui envoie*/
void affichageMessageServeur(int *sockfd, char *buffer)
{
  int i = recv(*sockfd, buffer, BUFSIZE, 0);
  if (i >= 0)
  {
    buffer[i] = '\0';
    printf("%s\n", buffer);
  }
}

// lire un caractere envoyé par le serveur :
// 0 = non valide ou erreur ; 1 = valide
char receptionValidite(int *sockfd)
{
  char c;
  read(*sockfd, &c, 1);
  return c;
}

void jouerPartie(int *sockfd, char *buffer)
{
  bool finDePartie = false;
  bool premierTour = true;
  while (!finDePartie)
  {
    /*Affichage du plateau après le tour de l'adversaire*/
    if (!premierTour)
    {
      printf("\nC'est au tour de l'adversaire, patientez.\n");
    }
    premierTour = false;

    affichageMessageServeur(sockfd, buffer);

    /*Fin de partie, interruption de la boucle et affichage du gagnant*/
    if (strstr(buffer, "FIN1") != NULL)
    {
      printf("Le joueur lanceur de défi a gagné ! Félicitations !\n");
      finDePartie = true;
      break;
    }
    else if (strstr(buffer, "FIN2") != NULL)
    {
      printf("Le joueur défié a gagné ! Félicitations !\n");
      finDePartie = true;
      break;
    }
    else if (strstr(buffer, "FIN3") != NULL)
    {
      printf("Personne n'a gagné, égalité.\n");
      finDePartie = true;
      break;
    }else if (strstr(buffer, "FINDEC") != NULL)
    {
      printf("Votre adversaire a lâchement fui devant votre superiorité...\n");
      finDePartie = true;
      break;
    }

    /*Début du tour*/
    printf("C'est à votre tour, entrez votre coup :\n");

    int valide = 0;
    char validiteCoup[2]; // Variable dans laquelle le serveur envoie si le coup est valide ou non

    do
    {
      /*Choix du coup*/
      char coupChoisi = rentrerReponse(sockfd);
      int i = recv(*sockfd, validiteCoup, 2, 0);
      if (i >= 0)
        validiteCoup[i] = '\0';

      valide = atoi(validiteCoup);

      /*Vérification de la validité du coup*/
      if (valide == 0)
      {
        printf("\n\x1b[31mSaisie invalide. Veuillez saisir un nombre entre 1 et 6.\x1b[0m\n");
      }
      else if (valide == 3)
      {
        printf("\n\x1b[31mVous ne pouvez pas choisir une case vide.\x1b[0m\n");
      }
      else if (valide == 2)
      {
        printf("\n\x1b[31mVous devez nourrir le joueur adverse.\x1b[0m\n");
      }
      else
      {
        break; // Sort de la boucle si la saisie est valide
      }
    } while (valide != 1);
  }
}

int main(int argc, char **argv)
{
  int sockfd, newsockfd, clilen, chilpid, ok, nleft, nbwriten;
  char choixMenu;
  char buffer[BUFSIZE];
  struct sockaddr_in cli_addr, serv_addr;

  if (argc != 3)
  {
    printf("usage  socket_client server port\n");
    exit(0);
  }

  /*
   *  Partie client
   */
  printf("Démarrage du client...\n");

  /* Initialise la structure de donnée */
  bzero((char *)&serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
  serv_addr.sin_port = htons(atoi(argv[2]));

  /* Ouvre le socket */
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    printf("socket error\n");
    exit(0);
  }

  /* Effectue la connection */
  if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
  {
    printf("socket error\n");
    exit(0);
  }

  printf("Client opérationnel !\n\n");

  /* Repete dans le socket tout ce qu'il entend */
  bool accepteParServeur = false;
  while (!accepteParServeur)
  {
    affichageMessageServeur(&sockfd, buffer);
    rentrerReponse(&sockfd);

    /*Validation du serveur*/
    if (receptionValidite(&sockfd) == '1')
    {
      accepteParServeur = true;
    }
    else
    {
      printf("ERREUR : Un joueur est déjà connecté avec ce pseudo\n");
    }
  }

  bool clientConnecte = true;

  while (clientConnecte)
  {

    /* Affichage du menu */
    affichageMessageServeur(&sockfd, buffer);
    choixMenu = rentrerReponse(&sockfd);

    /* Traitement du choix du joueur */
    switch (choixMenu)
    {
    /* Défier un joueur */
    case '1':
      affichageMessageServeur(&sockfd, buffer);
      char reponse = rentrerReponse(&sockfd);
      if (reponse == '0')
        continue;

      char statusDemande;
      read(sockfd, &statusDemande, 1);

      switch (statusDemande)
      {
      case '1':
        printf("\n\x1b[32mVotre demande a bien été envoyée.\n\n\x1b[33mEn attente de la réponse du joueur...\x1b[0m\n");
        char reponseDefi;
        read(sockfd, &reponseDefi, 1);

        if (reponseDefi == '1')
        {
          printf("\n\x1b[32mLe joueur a accepté votre demande, la partie va commencer !\x1b[0m\n");
          jouerPartie(&sockfd, buffer);
        }
        else
        {
          printf("\n\x1b[31mLe joueur a pris peur et a refusé votre demande...\x1b[0m\n");
        }
        break;

      case '0':
        printf("\n\x1b[31mLe joueur est déjà défié...\x1b[0m\n");
        break;

      default:
        printf("\n\x1b[31mLe joueur n'existe pas...\x1b[0m\n");
        break;
      }
      break;
    /* Voir son profil */
    case '2':
      affichageMessageServeur(&sockfd, buffer);
      rentrerReponse(&sockfd);
      break;
    /* Voir la liste des parties en cours */
    case '3':
      affichageMessageServeur(&sockfd, buffer);
      rentrerReponse(&sockfd);
      break;
    /* Modifier sa biographie */
    case '4':
      affichageMessageServeur(&sockfd, buffer);
      rentrerReponse(&sockfd);
      break;
    /* Rafraichir le menu */
    case '5':
      break;
    /* Se déconnecter */
    case '6':
      clientConnecte = false;
      break;

    /* Accepter un défi */
    case 'y':
      jouerPartie(&sockfd, buffer);
      break;

    /* Retourner au menu */
    default:
      continue;
      break;
    }
  }

  printf("\n\x1b[31mDéconnexion\n\x1b[0m");

  return 1;
}
