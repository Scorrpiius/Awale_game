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

// Constantes de jeu

#define MAUVAISE_SAISIE 0
#define COUP_VALIDE 1
#define COUP_INVALIDE 2
#define CASE_VIDE 3
#define PARTIE_FINIE 4

char *itoa(int value, char *buffer, int base);
int coupValide(int *plateau, int coup, int joueur);
void afficherPlateau(int *plateau, int *sock, int scoreJoueur1, int scoreJoueur2, char *pseudoJoueur1, char *pseudoJoueur2);
void initPlateau(int *score1, int *score2, int *plateau);
int prendreGraine(int caseFin, int joueur, int *plateau);
void jouerCoup(int *plateau, int coup, int joueur, int *score1, int *score2);
bool finDeJeu(int *plateau, int joueur, int scoreJoueur1, int scoreJoueur2);
char finDePartie(int scoreJoueur1, int scoreJoueur2);
