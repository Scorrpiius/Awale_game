#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#define RED "\033[31m"

int plateau[12];
int scoreJoueur1;
int scoreJoueur2;

int coupValide(int coup, int joueur){
    // 0 mauvaise saisie
    // -1 coup valide
    // -2 coup invalide 
    // -3 case vide
    // -4 coup invalide et partie finie

    int valide = 0;
    bool plateauVide = true;
    if (coup < 1 || coup > 6){
        return 0;
    }
    int nbrGraines = (joueur == 1) ? plateau[coup-1] : plateau[12-coup];
    if (nbrGraines == 0){
        return -3;
    }
    if(joueur == 1){
        int vraiCoup = coup -1;
        // On vérifie si le plateau adverse est vide 
        for(int i = 11; i >= 6; i--){
            if (plateau[i] != 0){
                plateauVide = false;
            }
        }
        if (!plateauVide){
            return -1;
        }
        // Le plateau adverse est vide on vérifie si le coup du joueur nourrit le plateau du joueur adverse
        if (plateau[vraiCoup] + vraiCoup >= 6){
            return -1;
        } else { 
            /* Vérifier si il existe un coup tel qu'il nourrit le joueur adverse
                si oui alors il faut le jouer
                si non la partie est finie
                */
            for (int i = 0; i < 6; i++){
                if (plateau[i] + i >= 6){
                    return -2;
                }   

            }   
        }
        return -3;
    } else if (joueur == 2){
        int vraiCoup = 12 - coup;
        // Même chose pour le joueur 2
        for(int i = 0; i <6; i++){
            if (plateau[i] != 0){
                plateauVide = false;
            }
        }
        if (!plateauVide){
            return -1;
        }
        if (plateau[vraiCoup] - coup >= 0){
            return -1;
        } else { 
            for (int i = 11; i >= 6; i--){
                if (plateau[i] + i >= 6){
                    return -2;
                }   

            }   
        }
        return -4;
    }
    
}  

void afficherPlateau()
{

    printf("\n");

    printf("\n\t\t\t\x1b[90m  1   2   3   4   5   6\x1b[0m\n");
    printf("\t\t\t╔═══╦═══╦═══╦═══╦═══╦═══╗\n");

    printf("\033[31mJoueur 1\x1b[0m : \x1b[32m\x1b[1m%d\x1b[0m pts\t║ ", scoreJoueur1);
    for (int i = 0; i < 6; i++)
    {
        if (plateau[i] >= 10) {
            printf("%d║ ", plateau[i]);
        } else {
            printf("%d ║ ", plateau[i]);
        }
        
    }

    printf("\n\t\t\t╠═══╬═══╬═══╬═══╬═══╬═══╣\n");

    printf("\033[34mJoueur 2\x1b[0m : \x1b[32m\x1b[1m%d\x1b[0m pts\t║ ", scoreJoueur2);
    for (int i = 11; i >= 6; i--)
    {
        if (plateau[i] >= 10) {
            printf("%d║ ", plateau[i]);
        } else {
            printf("%d ║ ", plateau[i]);
        }
    }

    printf("\n");
    printf("\t\t\t╚═══╩═══╩═══╩═══╩═══╩═══╝\n");
}

void initPlateau()
{
    scoreJoueur1 = 0;
    scoreJoueur2 = 0;
    for (int i = 0; i < 12; i++)
    {
        plateau[i] = 4;
    }
}

int prendreGraine(int caseFin, int joueur)
{
    int totalGraines = 0;
    bool verification = false;
    while (!verification)
    {
        if (joueur == 1)
        {
            if (caseFin < 6)
            {
                verification = true;
                break;
            }
            if (plateau[caseFin] == 2 || plateau[caseFin] == 3)
            {
                totalGraines += plateau[caseFin];
                plateau[caseFin] = 0;
                (caseFin--);
                caseFin = caseFin % 12;
            }
            else
            {
                verification = true;
                break;
            }
        }
        if (joueur == 2)
        {
            if (caseFin > 5)
            {
                verification = true;
                break;
            }
            if (plateau[caseFin] == 2 || plateau[caseFin] == 3)
            {
                totalGraines += plateau[caseFin];
                plateau[caseFin] = 0;
                (caseFin--);
                caseFin = caseFin % 12;
            }
            else
            {
                verification = true;
                break;
            }
        }
    }
    return totalGraines;
}

void jouerCoup(int coup, int joueur)
{

    int nbGraines;
    if (joueur == 1 )
    {
        coup = coup - 1;
        nbGraines = plateau[coup];
        plateau[coup] = 0;
        for (int i = coup + 1; i <= coup + nbGraines; i++)
        {
            plateau[i % 12]++;
        }

        int caseFin = (nbGraines + coup) % 12;
        scoreJoueur1 += prendreGraine(caseFin, joueur);
    }
    else
    {
        coup = 12 - coup;
        nbGraines = plateau[coup];
        plateau[coup] = 0;

        for (int i = coup + 1; i <= coup + nbGraines; i++)
        {
            plateau[i % 12]++;
        }

        int caseFin = (nbGraines + coup) % 12;
        scoreJoueur2 += prendreGraine(caseFin, joueur);
    }
}

bool finDeJeu(int joueur)
{

    bool fini = true;
    if (joueur == 1)
    {
        for (int i = 0; i < 5; i++)
        {
            if (plateau[i] != 0)
            {
                fini = false;
            }
        }
    }
    else
    {
        for (int i = 6; i < 11; i++)
        {
            if (plateau[i] != 0)
            {
                fini = false;
            }
        }
    }

    int nbGrainesTotal = 0;
    for (int i = 0; i < 12; i++)
    {
        nbGrainesTotal += plateau[i];
    }
    if (abs(scoreJoueur1 - scoreJoueur2) < nbGrainesTotal)
    {
        fini = false;
    }

    return fini;
}

void finDePartie()
{

    if (scoreJoueur1 > scoreJoueur2)
    {
        printf("Félicitations au joueur 1 !!! Vous avez fumé le joueur 2 \n");
    }
    else if (scoreJoueur2 > scoreJoueur1)
    {
        printf("Félicitations au joueur 2 !!! Vous avez fumé le joueur 1 \n");
    }
    else
    {
        printf("Personne n'a fumé personne ! Quel dommage...\n");
    }
}

int main(int argc, char **argv)
{
    initPlateau();

    afficherPlateau();

    int tourJoueur = 1;
    int coup;
    while (!finDeJeu(tourJoueur))
    {
    char *couleur = (tourJoueur == 1) ? "\x1b[31m" : "\x1b[34m";

    do {
        printf("%s\n\x1b[1m\x1b[4mJoueur %d\x1b[0m : Quel coup souhaitez-vous jouer ? (de 1 à 6)  \x1b[32m\x1b[1m", couleur, tourJoueur);

        int verif = scanf("%d", &coup);
        int valide = coupValide(coup, tourJoueur);
        
        if (verif != 1 || valide == 0) {
            printf("\n\x1b[31mSaisie invalide. Veuillez saisir un nombre entre 1 et 6.\x1b[0m\n");
            while (getchar() != '\n'); // Nettoie le tampon d'entrée
        } else if (verif != 1 || valide == -3 ){
            printf("\n\x1b[31mVous ne pouvez pas choisir une case vide.\x1b[0m\n");
            while (getchar() != '\n'); 
        } else if (verif != 1 || valide == -2){
            printf("\n\x1b[31mVous devez nourrir le joueur adverse.\x1b[0m\n");
            while (getchar() != '\n'); 
        } else  {
            break; // Sort de la boucle si la saisie est valide

        }
    } while (true);

    printf("\n\x1b[0mVous avez joué le coup : \x1b[32m\x1b[1m%d\x1b[0m\n", coup);



        jouerCoup(coup, tourJoueur);
        printf("\n__________________________________________________________\n");
        afficherPlateau();
        tourJoueur = (tourJoueur == 1) ? 2 : 1;
    }
    finDePartie();
}
