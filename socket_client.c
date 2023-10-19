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

void lecture(int * sockfd){
  char c;
  while (1) {
    c=getchar();
    write (*sockfd,&c,1); 
    if(c=='\n'){
      break;
    }
  }
}
void affichage(int * sockfd, char * buffer){
  int i = recv(*sockfd, buffer, BUFSIZE, 0); 
  if(i>=0){
        buffer[i] = '\0';
        printf("%s\n", buffer);
  }
}

//lire un caractere envoyé par le serveur : 
//0 = non valide ou erreur ; 1 = valide 
char receptionValidite(int * sockfd){
  char c;
  read(*sockfd,&c,1);
  return c;
}

int main(int argc, char** argv )
{ 
  int    sockfd,newsockfd,clilen,chilpid,ok,nleft,nbwriten;
  char c;
  char buffer[BUFSIZE];
  struct sockaddr_in cli_addr,serv_addr;

  if (argc!=3) {printf ("usage  socket_client server port\n");exit(0);}
 
 
  /*
   *  partie client 
   */
  printf ("client starting\n");  

  /* initialise la structure de donnee */
  bzero((char*) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family       = AF_INET;
  serv_addr.sin_addr.s_addr  = inet_addr(argv[1]);
  serv_addr.sin_port         = htons(atoi(argv[2]));
  
  /* ouvre le socket */
  if ((sockfd=socket(AF_INET,SOCK_STREAM,0))<0)
    {printf("socket error\n");exit(0);}
  
  /* effectue la connection */
  if (connect(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr))<0)
    {printf("socket error\n");exit(0);}
    
  
  /* Repete dans le socket tout ce qu'il entend */
  bool accepteParServeur = false;
  while(!accepteParServeur){
    affichage(&sockfd, buffer);
    lecture(&sockfd);

    /*Validation du serveur*/
    if(receptionValidite(&sockfd)=='1'){
      accepteParServeur=true;
    }
  }
  

  /*Affichage du menu*/
  affichage(&sockfd, buffer);
  lecture(&sockfd);
  
  /*  attention il s'agit d'une boucle infinie 
   *  le socket n'est jamais ferme !
   */
   
   return 1;

}
