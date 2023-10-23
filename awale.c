#include "awale.h"


int plateau[12];
int scoreJoueur1;
int scoreJoueur2;

// Fonction pour échanger deux nombres
void swap(char *x, char *y) {
    char t = *x; *x = *y; *y = t;
}
 
// Fonction pour inverser `buffer[i…j]`
char* reverse(char *buffer, int i, int j)
{
    while (i < j) {
        swap(&buffer[i++], &buffer[j--]);
    }
 
    return buffer;
}
 
// Fonction itérative pour implémenter la fonction `itoa()` en C
char* itoa(int value, char* buffer, int base)
{
    // entrée invalide
    if (base < 2 || base > 32) {
        return buffer;
    }
 
    // considère la valeur absolue du nombre
    int n = abs(value);
 
    int i = 0;
    while (n)
    {
        int r = n % base;
 
        if (r >= 10) {
            buffer[i++] = 65 + (r - 10);
        }
        else {
            buffer[i++] = 48 + r;
        }
 
        n = n / base;
    }
 
    // si le nombre est 0
    if (i == 0) {
        buffer[i++] = '0';
    }
 
    // Si la base est 10 et la valeur est négative, la string résultante
    // est précédé d'un signe moins (-)
    // Avec toute autre base, la valeur est toujours considérée comme non signée
    if (value < 0 && base == 10) {
        buffer[i++] = '-';
    }
 
    buffer[i] = '\0'; // string de fin nulle
 
    // inverse la string et la renvoie
    return reverse(buffer, 0, i - 1);
}


int coupValide(int coup, int joueur){

    if (coup < 1 || coup > 6){
        return MAUVAISE_SAISIE;
    }

    int nbrGraines = (joueur == 1) ? plateau[coup-1] : plateau[12-coup];
    if (nbrGraines == 0){
        return CASE_VIDE;
    }

    if(joueur == 1){
        int vraiCoup = coup -1;
        
        for(int i = 11; i >= 6; i--){ // On vérifie si le plateau adverse est vide 
            if (plateau[i] != 0){
                return COUP_VALIDE;
            }
        }

        
        if (plateau[vraiCoup] + vraiCoup >= 6){ // Le plateau adverse est vide on vérifie si le coup du joueur nourrit le plateau du joueur adverse
            return COUP_VALIDE;
        } else { 
            /* Vérifier si il existe un coup tel qu'il nourrit le joueur adverse
                si oui alors il faut le jouer
                si non la partie est finie
                */
            for (int i = 0; i < 6; i++){
                if (plateau[i] + i >= 6){
                    return COUP_INVALIDE;
                }   

            }   
        }
        return PARTIE_FINIE;
    } else if (joueur == 2){ // Même chose pour le joueur 2
        int vraiCoup = 12 - coup;
       
        for(int i = 0; i <6; i++){
            if (plateau[i] != 0){
                return COUP_VALIDE;
            }
        }

        if (plateau[vraiCoup] - coup >= 0){
            return COUP_VALIDE;
        } else { 
            for (int i = 11; i >= 6; i--){
                if (plateau[i] + i >= 6){
                    return COUP_INVALIDE;
                }   

            }   
        }
        return PARTIE_FINIE;
    }
    
}  

void afficherPlateau(int * plateau, int *sock, int scoreJoueur1, int scoreJoueur2)
{
    //printf("Je suis entré dans afficahge palrazojrhireoae\n");
    char affichagePlateau[4096]="\n\n\t\t\t\x1b[90m  1   2   3   4   5   6\x1b[0m\n\t\t\t╔═══╦═══╦═══╦═══╦═══╦═══╗\n\033[31mJoueur 1\x1b[0m : \x1b[32m\x1b[1m";
    //printf("TEST N°1 %s", affichagePlateau);
    char scoreJ1[2];
    char scoreJ2[2];
    char casePlateau[2];


    itoa(scoreJoueur1, scoreJ1, 10);
    itoa(scoreJoueur2, scoreJ2, 10);
    strcat(affichagePlateau, scoreJ1);
    //printf("TEST N°2 %s", affichagePlateau);

    strcat(affichagePlateau, "\x1b[0m pts\t║ ");
    //printf("TEST N°3 %s", affichagePlateau);

    /*
     printf("\n");

     printf("\n\t\t\t\x1b[90m  1   2   3   4   5   6\x1b[0m\n");
     printf("\t\t\t╔═══╦═══╦═══╦═══╦═══╦═══╗\n");

     printf("\033[31mJoueur 1\x1b[0m : \x1b[32m\x1b[1m%d\x1b[0m pts\t║ ", scoreJoueur1);
     */

    for (int i = 0; i < 6; i++){
        itoa(plateau[i], casePlateau, 10);
        if (plateau[i] >= 10) {
        
            strcat(affichagePlateau, casePlateau);
            strcat(affichagePlateau, "║ ");
            //printf("%d║ ", plateau[i]);
        } else {
            strcat(affichagePlateau, casePlateau);
            strcat(affichagePlateau, " ║ ");
            //printf("%d ║ ", plateau[i]);
        }
        
    }
    //printf("TEST N°4 %s", affichagePlateau);

    strcat(affichagePlateau, "\n\t\t\t╠═══╬═══╬═══╬═══╬═══╬═══╣\n\033[34mJoueur 2\x1b[0m : \x1b[32m\x1b[1m");

    strcat(affichagePlateau, scoreJ2);
    strcat(affichagePlateau, "\x1b[0m pts\t║ ");

    //printf("\n\t\t\t╠═══╬═══╬═══╬═══╬═══╬═══╣\n");

    //printf("\033[34mJoueur 2\x1b[0m : \x1b[32m\x1b[1m%d\x1b[0m pts\t║ ", scoreJoueur2);
    for (int i = 11; i >= 6; i--){
    itoa(plateau[i], casePlateau, 10);
    if (plateau[i] >= 10) {
        strcat(affichagePlateau, casePlateau);
        strcat(affichagePlateau, "║ ");
        //printf("%d║ ", plateau[i]);
    } else {
        strcat(affichagePlateau, casePlateau);
        strcat(affichagePlateau, " ║ ");
        //printf("%d ║ ", plateau[i]);
    }
    }

    strcat(affichagePlateau, "\n\t\t\t╚═══╩═══╩═══╩═══╩═══╩═══╝\n");
    //printf("\n");
    //printf("\t\t\t╚═══╩═══╩═══╩═══╩═══╩═══╝\n");
    //printf("%s", affichagePlateau);
    send(*sock, affichagePlateau, strlen(affichagePlateau), 0);
    //send(*sock, affichagePlateau, strlen(affichagePlateau), 0);
}

void initPlateau(int* scoreJoueur1, int* scoreJoueur2, int* plateau)
{
    *scoreJoueur1 = 0;
    *scoreJoueur2 = 0;
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
        int caseInit = coup;
        nbGraines = plateau[coup];
        plateau[coup] = 0;
        for (int i = coup + 1; i <= coup + nbGraines; i++)
        {   
            if( !(i%12 == caseInit)){
                plateau[i % 12]++;

            } else { coup++; }
        }

        int caseFin = (nbGraines + coup) % 12;
        scoreJoueur1 += prendreGraine(caseFin, joueur);
    }
    else
    {
        coup = 12 - coup;
        int caseInit = coup;
        nbGraines = plateau[coup];
        plateau[coup] = 0;

        for (int i = coup + 1; i <= coup + nbGraines; i++)
        {
            if( !(i%12 == caseInit)){
                plateau[i % 12]++;

            } else { coup++; }
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

/*int main(int argc, char **argv)
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
        } else {
            break; // Sort de la boucle si la saisie est valide
void finDePartie();



        jouerCoup(coup, tourJoueur);
        printf("\n__________________________________________________________\n");
        afficherPlateau();
        tourJoueur = (tourJoueur == 1) ? 2 : 1;
    }
    finDePartie();
}*/
