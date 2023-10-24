#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

// Constantes de jeu

#define MAUVAISE_SAISIE 0
#define COUP_VALIDE -1
#define COUP_INVALIDE -2
#define CASE_VIDE -3
#define PARTIE_FINIE -4

int coupValide(int coup, int joueur);
void afficherPlateau(int * plateau, int *sock, int scoreJoueur1, int scoreJoueur2, char * pseudoJoueur1, char * pseudoJoueur2);
void initPlateau(int *score1, int *score2, int * plateau);
int prendreGraine(int caseFin, int joueur);
void jouerCoup(int coup, int joueur);
bool finDeJeu(int joueur);
void finDePartie();
