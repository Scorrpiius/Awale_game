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
int nbJoueurs;

// la méthode renvoie -2 si le joueur n'existe pas;
//l'indice du joueur dans la liste si le joueur a déjà créé son compte et n'est pas connecté ;
// -1 si un autre joueur est connecté avec le même pseudo
int joueurExistant(char* pseudoNouveauJoueur){

    for(int i = 0; i<nbJoueurs; i++){
      Joueur j = listeJoueurs[i];
      if(strcmp(pseudoNouveauJoueur, j.pseudo) == 0){
          if(j.connecte == 0){
            return i;
          }else{
            return -1;
          }
      }
    }
    return -2;
}

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

  nbJoueurs = 0;
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
            

            //printf("%s avec %d nombres de victoires\n", listeJoueurParam[indice].pseudo, listeJoueurParam[indice].nbVictoires);

            if((c=fgetc(fic))== EOF){
              fileEnd = true; 
              break;
            }
            fseek(fic, -1L, SEEK_CUR);
            indice++;

            
        }
        nbJoueurs = indice+1;
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
        Joueur j;
        close(sockfd); /* socket inutile pour le fils */
        /* traiter la communication */

        bool joueurNonValide = true;
        while(joueurNonValide){
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
          
          lireListeJoueur(listeJoueurs);
          int validitePseudo = joueurExistant(pseudoInput);

          if(validitePseudo == -2){
            char requestValidation [] = "Votre pseudo est : ";

            //creation d'un joueur
            j.connecte = 1;
            strcpy(j.biographie,"Votre biographie est vide. Allez la remplir ! ");
            j.occupe = false;
            j.nbVictoires = 0;
            strcpy(j.pseudo, pseudoInput);
            strcat(requestValidation, j.pseudo);

            printf("Ajout du joueur %s dans la base de données\n", j.pseudo);
            ecrireListeJoueur(j);

            char c = '1';
            write(scomm, &c, 1);
            joueurNonValide = false;
          }else if (validitePseudo == -1){
            char c = '0';
            write(scomm, &c, 1);
          }else{
            j = listeJoueurs[validitePseudo];
            j.connecte = 1;
            char c = '1';
            write(scomm, &c, 1);
            joueurNonValide = false;
          }

        }
          
         

          char requestMenu[] = "--------------- Bonjour ";
          strcat(requestMenu, strcat(j.pseudo,"---------------\n \t1: Défier un joueur \n\t2: Voir son profil \n\t3: Modifer sa biographie \n\t4: Déconnexion")) ;
          send(scomm, requestMenu, strlen(requestMenu), 0);
        

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
