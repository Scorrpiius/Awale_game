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

typedef struct Joueur{
  char pseudo[100];
  bool occupe;
  int connecte;
  char biographie[1000];
  int nbVictoires;
  
} Joueur;

Joueur listeJoueurs[200];
void ecrireListeJoueur(Joueur j){
  FILE* fic ;

//ouverture du fic de données CSV
    int indice = 0;
    fic = fopen( "liste_joueurs.csv", "r+") ;
    if (fic==NULL)
    {
        printf("Ouverture fic impossible !");
    }else{ 
        fseek(fic, 0, SEEK_END);
        fprintf(fic, "%s;%d;%d;\n", j.pseudo, j.nbVictoires, j.connecte);
    }
    fclose(fic);

} 

void lireListeJoueur(Joueur* listeJoueurParam){
    FILE* fic ;


//ouverture du fic de données CSV
    int indice = 0;
    fic = fopen( "liste_joueurs.csv", "r") ;
    if (fic==NULL)
    {
        printf("Ouverture fic impossible !");
    }else{
      char c;
      bool fileEnd = false;
      
      int indice =0;
      while (!fileEnd) {
            
  
            char ligne[300];
            int index=0;
            while((c=fgetc(fic)) != '\n'){
              ligne[index] = c;
              index++;
            }
            ligne[index]='\0';
            char * infosJoueur = strtok(ligne, ";");
            strcpy(listeJoueurParam[indice].pseudo, infosJoueur);
            infosJoueur = strtok(NULL, ";");
            listeJoueurParam[indice].nbVictoires = atoi(infosJoueur);
            infosJoueur = strtok(NULL, ";");
            listeJoueurParam[indice].connecte = atoi(infosJoueur);
            

            printf("%s avec %d nombres de victoires\n", listeJoueurParam[indice].pseudo, listeJoueurParam[indice].nbVictoires);

            if((c=fgetc(fic))== EOF){
              fileEnd = true; 
              break;
            }
            fseek(fic, -1L, SEEK_CUR);
            indice++;

            
        }
    } 
    fclose(fic);

} 

int main(int argc, char** argv )
{ 
  char datas[] = "hello\n";
  int    sockfd,scomm,clilen,chilpid,ok,nleft,nbwriten;
  char c;
  int pid;
  struct sockaddr_in cli_addr,serv_addr;
 
  
  int nbJoueur = 0;
  

  if (argc!=2) {printf ("usage: socket_server port\n");exit(0);}
 
  printf ("server starting...\n");  
  
  /* ouverture du socket */
  sockfd = socket (AF_INET,SOCK_STREAM,0);
  if (sockfd<0) {printf ("impossible d'ouvrir le socket\n");exit(0);}

  /* initialisation des parametres */
  bzero((char*) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family       = AF_INET;
  serv_addr.sin_addr.s_addr  = htonl(INADDR_ANY);
  serv_addr.sin_port         = htons(atoi(argv[1]));

  /* effecture le bind */
  if (bind(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr))<0)
     {printf ("impossible de faire le bind\n");exit(0);}

  /* petit initialisation */
  listen(sockfd,1);
  
  while (1)
  { 
            
      scomm = accept(sockfd, NULL,NULL);
      printf ("connection accepted\n");
      pid = fork();
      if (pid == 0) /* c’est le fils */
      {
          close(sockfd); /* socket inutile pour le fils */
          /* traiter la communication */

          //demander pseudo à joueur
          char requestPseudo[] = "Veuillez entrer votre pseudo, svp.";

          send(scomm, requestPseudo, strlen(requestPseudo), 0);

          //recevoir le pseudo de notre cher client
          char pseudoInput[100];
          int indice = 0;
          printf("Pseudo reçu : ");
          while(1){
            
            read(scomm,&c,1);
            printf("%c",c); 
            if(c=='\n'){
              break;
            }
            //enregistrer le pseudo
            pseudoInput[indice] = c;
            indice++;
          }
          printf("\n");
          
          char requestValidation [] = "Votre pseudo est : ";
          Joueur j;
          j.connecte = 1;
          strcpy(j.biographie,"Votre biographie est vide. Allez la remplir ! ");
          j.occupe = false;
          j.nbVictoires = 0;
          strcpy(j.pseudo, pseudoInput);
          strcat(requestValidation, j.pseudo);
          send(scomm, requestValidation, strlen(requestValidation), 0);


          printf("Ajout du joueur\n");
          ecrireListeJoueur(j);

          char requestMenu[] = "--------------- Menu ---------------\n \t1: Défier un joueur \n\t2: Voir son profil \n\t3: Modifer sa biographie \n\t4: Déconnexion";
          send(scomm, requestMenu, strlen(requestMenu), 0);

          lireListeJoueur(listeJoueurs);
          printf("3eme joueur pseudo : %s", listeJoueurs[2].pseudo);

          close(scomm);
          exit(0); /* on force la terminaison du fils */
      }
      else /* c’est le pere */
      {
        close(scomm); /* socket inutile pour le pere */
      }
  }

   return 1;
 }
