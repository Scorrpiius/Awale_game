#include "awale.h"

int plateau[12];
int scoreJoueur1;
int scoreJoueur2;

// Fonction pour échanger deux nombres
void swap(char *x, char *y)
{
    char t = *x;
    *x = *y;
    *y = t;
}

// Fonction pour inverser `buffer[i…j]`
char *reverse(char *buffer, int i, int j)
{
    while (i < j)
    {
        swap(&buffer[i++], &buffer[j--]);
    }

    return buffer;
}

// Fonction itérative pour implémenter la fonction `itoa()` en C
char *itoa(int value, char *buffer, int base)
{
    // entrée invalide
    if (base < 2 || base > 32)
    {
        return buffer;
    }

    // considère la valeur absolue du nombre
    int n = abs(value);

    int i = 0;
    while (n)
    {
        int r = n % base;

        if (r >= 10)
        {
            buffer[i++] = 65 + (r - 10);
        }
        else
        {
            buffer[i++] = 48 + r;
        }

        n = n / base;
    }

    // si le nombre est 0
    if (i == 0)
    {
        buffer[i++] = '0';
    }

    // Si la base est 10 et la valeur est négative, la string résultante
    // est précédé d'un signe moins (-)
    // Avec toute autre base, la valeur est toujours considérée comme non signée
    if (value < 0 && base == 10)
    {
        buffer[i++] = '-';
    }

    buffer[i] = '\0'; // string de fin nulle

    // inverse la string et la renvoie
    return reverse(buffer, 0, i - 1);
}

int coupValide(int *plateau, int coup, int joueur) {

    if (coup < 1 || coup > 6) { return MAUVAISE_SAISIE; }

    int nbrGraines = (joueur == 1) ? plateau[coup - 1] : plateau[12 - coup];
    if (nbrGraines == 0) { return CASE_VIDE; }

    if (joueur == 1){
        int vraiCoup = coup - 1;

        for (int i = 11; i >= 6; i--) { /* On vérifie si le plateau adverse est vide */
            if (plateau[i] != 0) {return COUP_VALIDE;}
        }

        if (plateau[vraiCoup] + vraiCoup >= 6){ /* Le plateau adverse est vide on vérifie si le coup du joueur nourrit le plateau du joueur adverse */
            return COUP_VALIDE;
        } else {
            /* Vérifier si il existe un coup tel qu'il nourrit le joueur adverse
                si oui alors il faut le jouer
                si non la partie est finie
                */
            for (int i = 0; i < 6; i++) {
                if (plateau[i] + i >= 6) {return COUP_INVALIDE;}
            }
        }
        return PARTIE_FINIE;

    } else if (joueur == 2) { /* Même chose pour le joueur 2 */
        int vraiCoup = 12 - coup;

        for (int i = 0; i < 6; i++) {
            if (plateau[i] != 0) { return COUP_VALIDE; }
        }

        if (plateau[vraiCoup] - coup >= 0) {
            return COUP_VALIDE;
        } else {
            for (int i = 11; i >= 6; i--) {
                if (plateau[i] + i >= 6) { return COUP_INVALIDE;}
            }
        }
        return PARTIE_FINIE;
    }
}

void afficherPlateau(int *plateau, int *sock, int scoreJoueur1, int scoreJoueur2, char *pseudoJoueur1, char *pseudoJoueur2) {

    char affichagePlateau[4096] = "\n\n\t\t\t\x1b[90m  1   2   3   4   5   6\x1b[0m\n\t\t\t╔═══╦═══╦═══╦═══╦═══╦═══╗\n\033[31m";
    strcat(affichagePlateau, pseudoJoueur1);

    /* L'affichage est différent si le pseudo d'un joueur est trop petit */
    if ((int)strlen(pseudoJoueur1) <= 4) {

        strcat(affichagePlateau, "\x1b[0m\t: \t\x1b[32m\x1b[1m");

    } else {

        strcat(affichagePlateau, "\x1b[0m : \t\x1b[32m\x1b[1m");

    }

    char scoreJ1[3];
    char scoreJ2[3];
    char casePlateau[2];

    itoa(scoreJoueur1, scoreJ1, 10);
    itoa(scoreJoueur2, scoreJ2, 10);
    strcat(affichagePlateau, scoreJ1);
    strcat(affichagePlateau, "\x1b[0m pts\t║ ");

    for (int i = 0; i < 6; i++){

        itoa(plateau[i], casePlateau, 10);
        if (plateau[i] >= 10) {

            strcat(affichagePlateau, casePlateau);
            strcat(affichagePlateau, "║ ");
        } else {

            strcat(affichagePlateau, casePlateau);
            strcat(affichagePlateau, " ║ ");

        }
    }

    /* L'affichage est différent si le pseudo d'un joueur est trop petit */
    strcat(affichagePlateau, "\n\t\t\t╠═══╬═══╬═══╬═══╬═══╬═══╣\n\033[34m");
    strcat(affichagePlateau, pseudoJoueur2);
    
    if ((int)strlen(pseudoJoueur1) <= 4) {

        strcat(affichagePlateau, "\x1b[0m\t: \t\x1b[32m\x1b[1m");

    } else {

        strcat(affichagePlateau, "\x1b[0m : \t\x1b[32m\x1b[1m");

    }

    strcat(affichagePlateau, scoreJ2);
    strcat(affichagePlateau, "\x1b[0m pts\t║ ");

    for (int i = 11; i >= 6; i--) {

        itoa(plateau[i], casePlateau, 10);

        if (plateau[i] >= 10) {

            strcat(affichagePlateau, casePlateau);
            strcat(affichagePlateau, "║ ");

        } else {

            strcat(affichagePlateau, casePlateau);
            strcat(affichagePlateau, " ║ ");

        }
    }

    strcat(affichagePlateau, "\n\t\t\t╚═══╩═══╩═══╩═══╩═══╩═══╝\n");
    send(*sock, affichagePlateau, strlen(affichagePlateau), 0);

}

void initPlateau(int *scoreJoueur1, int *scoreJoueur2, int *plateau) {

    *scoreJoueur1 = 0;
    *scoreJoueur2 = 0;
    for (int i = 0; i < 12; i++) { plateau[i] = 4; }

}

int prendreGraine(int caseFin, int joueur, int *plateau) {

    int totalGraines = 0;
    bool verification = false;
    while (!verification) {

        if (joueur == 1) {

            if (caseFin < 6) {

                verification = true;
                break;

            }

            if (plateau[caseFin] == 2 || plateau[caseFin] == 3) {

                totalGraines += plateau[caseFin];
                plateau[caseFin] = 0;
                (caseFin--);
                caseFin = caseFin % 12;

            } else {

                verification = true;
                break;

            }
        }

        if (joueur == 2) {

            if (caseFin > 5) {

                verification = true;
                break;

            }

            if (plateau[caseFin] == 2 || plateau[caseFin] == 3) {

                totalGraines += plateau[caseFin];
                plateau[caseFin] = 0;
                (caseFin--);
                caseFin = caseFin % 12;

            } else {

                verification = true;
                break;
                
            }
        }
    }
    return totalGraines;
}

void jouerCoup(int *plateau, int coup, int joueur, int *scoreJoueur1, int *scoreJoueur2) {

    int nbGraines;
    if (joueur == 1) {

        coup = coup - 1;
        int caseInit = coup;
        nbGraines = plateau[coup];
        plateau[coup] = 0;

        for (int i = coup + 1; i <= coup + nbGraines; i++) {

            if (!(i % 12 == caseInit)) {
                
                plateau[i % 12]++;

            } else {

                coup++;

            }
        }

        int caseFin = (nbGraines + coup) % 12;
        *scoreJoueur1 += prendreGraine(caseFin, joueur, plateau);

    } else {

        coup = 12 - coup;
        int caseInit = coup;
        nbGraines = plateau[coup];
        plateau[coup] = 0;

        for (int i = coup + 1; i <= coup + nbGraines; i++) {

            if (!(i % 12 == caseInit)) {

                plateau[i % 12]++;

            } else {

                coup++;
                
            }
        }

        int caseFin = (nbGraines + coup) % 12;
        *scoreJoueur2 += prendreGraine(caseFin, joueur, plateau);
    }
}

bool finDeJeu(int *plateau, int joueur, int scoreJoueur1, int scoreJoueur2) {

    bool fini = false;
    if (scoreJoueur1 >= 25 || scoreJoueur2 >= 25) { fini = true; }
    return fini;
}

char finDePartie(int scoreJoueur1, int scoreJoueur2) {

    if (scoreJoueur1 > scoreJoueur2) {

        return '1';

    } else if (scoreJoueur2 > scoreJoueur1) {

        return '2';

    } else {

        return '3';

    }
}
