/**
 * @file main.c
 * @brief Programme d'un jeu de snake.
 * @author Dhennin Elouan
 * @version 2
 * @date 29/10/24
 *
 * Le programme va utiliser différentes fonctions pour faire un jeu de snake.
 * Tout est basé sur un tableau à deux dimensions et un serpent.
 * Le serpent ne se déplace pas dans le tableau, mais dessus.
 * Et avec des variable et différents calculs il va réussir à définir
 * sa position sur le tableau.
 * Pour le moment, la condition d'arrêt se limite à appuyer sur 'a' ou que le serpent
 * arrive au bout du tableau.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <termio.h>

/*Constante du joueur*/
#define TAILLE_TABLEAU 40
#define POS_INITIAL_JOUEUR 20
#define LONGEUR_MAX 100

#define DECALAGE_TABLEAU_Y 5 // Décalage du tableau en X pour le placer dans la console
#define DECALAGE_TABLEAU_X 5 // Décalage du tableau en Y pour le placer dans la console
#define CACHER_CURSEUR 70	 // Valeur absurde pour être sur d'être coller en bas de la console

#define DELAI_IMAGES 90000 // micro_sec

#define TAILLE_INITIAL_DU_JOUEUR 10 // Taille de base du joueur (en réalité 10, car la boucle for commence à 0)
#define POSITION_MINIMUM 1			// Position minimum d'apparition du joueur sur le tableau

/*Les constantes du jeu*/
const char FERMER_JEU = 'a';		// Condition d'arrêt
const char TETE_JOUEUR = 'O';		// Tête du joueur
const char CORP_JOUEUR = 'X';		// Corp du joueur
const char CARACTERE_EFFACER = ' '; // Pour effacer un élément
const char POMME = '0';

/*Les constantes de direction*/
const char HAUT = 'z';
const char BAS = 's';
const char DROITE = 'd';
const char GAUCHE = 'q';

// Définir le type du tableau pour le joueur
typedef int corp_longeur[LONGEUR_MAX];

// Définir le type du tableau à deux dimensions
typedef char type_tableau_2d[TAILLE_TABLEAU][TAILLE_TABLEAU];

/* Initialisations des fonctions et des procédure */
int kbhit();						  // équivalent d'un INKEYS en BASIC
int dans_tableau(int eltx, int elty); // Vérifie que l'élément se trouve dans le tableau pour le dessiner
void goto_x_y(int x, int y);		  // équivalent d'un LOCATE en Basic
void afficher(int x, int y, char c);  // Affiche en lien avec goto_x_y un caractère à l'écran
void print_tableau(type_tableau_2d tableau);
void progresser(corp_longeur Les_x, corp_longeur Les_y, char direction, int taille_joueur);
void dessiner_serpent(corp_longeur Les_x, corp_longeur Les_y, int taille_joueur);
void effacer_serpent(corp_longeur Les_x, corp_longeur Les_y, int taille_joueur);
void effacer(int x, int y);
void creation_du_serpent(int x, int y, corp_longeur Les_x, corp_longeur Les_y);
void enable_echo();
void disable_echo();
char calcul_direction(char direction, char lettre, int *bloque_x, int *bloque_y);
char lire_entrer();

int kbhit()
{
	// la fonction retourne :
	// 1 si un caractere est present
	// 0 si pas de caractere present

	int un_caractere = 0;
	struct termios oldt, newt;
	int ch;
	int oldf;

	// mettre le terminal en mode non bloquant
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

	ch = getchar();

	// restaurer le mode du terminal
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);

	if (ch != EOF)
	{
		ungetc(ch, stdin);
		un_caractere = 1;
	}
	return un_caractere;
}

void goto_x_y(int x, int y)
{
	/**
	 * @brief Fonction qui déplace le curseur dans la console
	 * @param x position x.
	 * @param y position y.
	 */
	printf("\033[%d;%df", y, x);
}

void afficher(int x, int y, char c)
{
	/**
	 * @brief affiche un caractère à une position donner
	 * @param x position x.
	 * @param y position y.
	 * @param c caractère a afficher aux cordonnées donner.
	 */

	goto_x_y(x, y);
	printf("%c", c);
	goto_x_y(1, CACHER_CURSEUR); // cacher le curseur pour une meilleur lisibilité

	fflush(stdout);
}

void print_tableau(type_tableau_2d tableau)
{
	/**
	 * @brief Permet de dessiner le tableau de jeu. Mais aussi l'interface autour !
	 * @param tableau : tableau à deux dimension qui représente le tableau de jeu
	 */

	int lig = 0;
	int col = 0;
	int i = 0;

	// Dessiner le contour du tableau
	// Valeur un peu arbitraire pour avoir quelque chose
	// d'estetique

	// Dessine le bord haut et droit. +1 pour le décalage (1 plus
	//  grand que le tabelau de jeu)

	for (i = 0; i <= TAILLE_TABLEAU + 1; i++)
	{
		afficher(i + DECALAGE_TABLEAU_X - 1, DECALAGE_TABLEAU_Y - 1, '_');
		afficher(i + DECALAGE_TABLEAU_X - 1, TAILLE_TABLEAU + DECALAGE_TABLEAU_Y, '_');
	}

	// Dessine le bord droit et gauche. +1 pour le décalage (*
	// et +1 pour repasser sur la dernière ligne par esthetisme)
	for (i = 0; i <= TAILLE_TABLEAU; i++)
	{
		afficher(DECALAGE_TABLEAU_X - 1, i + DECALAGE_TABLEAU_Y, '|');
		afficher(TAILLE_TABLEAU + DECALAGE_TABLEAU_X, i + DECALAGE_TABLEAU_Y, '|');
	}

	// Double boucle qui permet de dessiner les colonne et les ligne du tableau
	for (lig = 0; lig < TAILLE_TABLEAU; lig++)
	{

		for (col = 0; col < TAILLE_TABLEAU; col++)
		{
			tableau[lig][col] = ' ';
			afficher(col + DECALAGE_TABLEAU_X, lig + DECALAGE_TABLEAU_Y, tableau[lig][col]);
		}

		col = 0;
	}

	// Dessiner le titre
	// Décalage du tableau + la moitié de la taille du tableau - la moitié de la taille
	// de la chaine pour le placer en haut au centre de l'écran
	goto_x_y(DECALAGE_TABLEAU_X + (TAILLE_TABLEAU / 2) - 5, DECALAGE_TABLEAU_Y - 2);
	printf("Snake v2");
}

void progresser(corp_longeur Les_x, corp_longeur Les_y, char direction, int taille_joueur)
{
	/**
	 * @brief Calcule la nouvelle position du serpent quand il avance sans intervention du joueur
	 * @param Les_x liste des valeurs en x
	 * @param Les_y liste des valeurs en y
	 * @param direction variable qui permet de connaitre la direction du joueur
	 */

	int i = 0;
	int tampon_x;
	int tampon_y;
	int precedent_x = Les_x[0];
	int precedent_y = Les_y[0];

	/*
	Ce petit algo permet de permuter les valeurs du corps du joueur
	L'élement 1 donne ces valeur au second, qui donne ces valeur au troisième...
	Pour ce faire schématisons :

	x1 = 12
	x2 = 13
	x3 = 14

	Pour les échanger faisons comme ceci :

	val_précédent = x1
	tampon = x2
	x2 = val_précédent
	val_précédent = tampon

	et ceci dans un boucle permet de décaler toute les valeur vers x-1 !
	La x[0] n'est pas dans la boucle car il est le premier, et l'on va par la suite
	calculer une nouvelle valeur pour lui.
	*/

	for (i = 1; i < taille_joueur; i++)
	{
		tampon_x = Les_x[i];
		tampon_y = Les_y[i];

		Les_x[i] = precedent_x;
		Les_y[i] = precedent_y;

		precedent_x = tampon_x;
		precedent_y = tampon_y;
	}

	if (direction == HAUT)
	{
		Les_y[0]--; // Changer la trajectoire vers le haut
	}
	else if (direction == GAUCHE)
	{
		Les_x[0]--; // Changer la trajectoire vers la gauche
	}
	else if (direction == BAS)
	{
		Les_y[0]++; // Changer la trajectoire vers le bas
	}
	else if (direction == DROITE)
	{
		Les_x[0]++; // Changer la trajectoire vers la droite
	}
}

int dans_tableau(int eltx, int elty)
{
	/**
	 * @brief va calculer si un éléments se trouve dans le tableau
	 * @param eltx : position de l'élément en x
	 * @param elty : position de l'élément en y
	 * @return 1 si l'élément est dans le tableaun. 0 S'il se trouve en dehors
	 */
	if ((eltx < TAILLE_TABLEAU + DECALAGE_TABLEAU_X // Si le joueur touche le mur droit
		 && eltx >= DECALAGE_TABLEAU_X)				// Si le joueur touche le mur gauche

		&&

		(elty < TAILLE_TABLEAU + DECALAGE_TABLEAU_Y // Si le joueur touche le mur bas
		 && elty >= DECALAGE_TABLEAU_Y))			// Si le joueur touche le mur haut
	{
		return 0;
	}
	return 1;
}

void dessiner_serpent(corp_longeur Les_x, corp_longeur Les_y, int taille_joueur)
{
	/**
	 * @brief Dessine le joueur en premier, et fait une boucle pour dessiner les enfants
	 * (le corps) du joueur en fonction de la taille du joueur.
	 * @param Les_x liste des valeurs en x
	 * @param Les_y liste des valeurs en y
	 */

	int i;
	printf("\033[1;33m");
	for (i = 1; i < taille_joueur; i++)
	{

		if (dans_tableau(Les_x[i], Les_y[i]) == 0) // Vérifier si l'élément est dans le cardre du tableau
		{
			if (i % 2 == 0)
				printf("\033[1;33m");

			else 
				printf("\033[1;34m");

			afficher(Les_x[i], Les_y[i], CORP_JOUEUR);
		}
	}

	if (dans_tableau(Les_x[0], Les_y[0]) == 0) // Même chose pour le tête
	{
		printf("\033[1;36m");
		afficher(Les_x[0], Les_y[0], TETE_JOUEUR);
	}
}

void effacer(int x, int y)
{
	/**
	 * @brief effacer un caractère à une position voulu
	 * @param x position x
	 * @param y position y
	 */
	afficher(x, y, CARACTERE_EFFACER);
}

void effacer_serpent(corp_longeur Les_x, corp_longeur Les_y, int taille_serpent)
{
	/**
	 * @brief Va de la fin jusqu'à la tête du joueur pour remplacer avec un caractère vide
	 * @param Les_x liste des valeurs en x
	 * @param Les_y liste des valeurs en y
	 */

	int i;

	// taille + 1 pour prendre en compte la tête du joueur
	for (i = 0; i < taille_serpent; i++)
	{
		if (dans_tableau(Les_x[i], Les_y[i]) == 0) // Ne pas effacer un éléments du serpent en dehors du tableau (risque d'effacer les bordure)
		{
			effacer(Les_x[i], Les_y[i]);
		}
	}
}

char lire_entrer()
{
	/**
	 * @brief va lire les entrer de l'utilisateur
	 * @return retourne la lettre qui a été entrer par l'utilisateur
	 */

	int entrer;
	char lettre = ' ';
	entrer = kbhit();
	if (entrer == 1)
	{						  // Vérifie à chaque "tour de jeu" si quelque chose est rentré
		scanf("%c", &lettre); // Va regarder dans le buffer la lettre à rechercher
		return lettre;
	}
	return lettre;
}

void creation_du_serpent(int x, int y, corp_longeur Les_x, corp_longeur Les_y)
{
	/**
	 * @brief Fonction qui viens crée le serpent la premièrer fois.
	 * @param x position initial en x
	 * @param y position initial en y
	 * @param Les_x tableau des position en x
	 * @param Les_y tableau des position en y
	 *
	 */
	int i = 0;

	for (i = 0; i < TAILLE_INITIAL_DU_JOUEUR; i++)
	{
		Les_x[i] = x - i;
		Les_y[i] = y;
	}
}

void disable_echo()
{
	struct termios tty;

	// Obtenir les attributs du terminal
	if (tcgetattr(STDIN_FILENO, &tty) == -1)
	{
		perror("tcgetattr");
		exit(EXIT_FAILURE);
	}

	// Desactiver le flag ECHO
	tty.c_lflag &= ~ECHO;

	// Appliquer les nouvelles configurations
	if (tcsetattr(STDIN_FILENO, TCSANOW, &tty) == -1)
	{
		perror("tcsetattr");
		exit(EXIT_FAILURE);
	}
}

void enable_echo()
{
	struct termios tty;

	// Obtenir les attributs du terminal
	if (tcgetattr(STDIN_FILENO, &tty) == -1)
	{
		perror("tcgetattr");
		exit(EXIT_FAILURE);
	}

	// Reactiver le flag ECHO
	tty.c_lflag |= ECHO;

	// Appliquer les nouvelles configurations
	if (tcsetattr(STDIN_FILENO, TCSANOW, &tty) == -1)
	{
		perror("tcsetattr");
		exit(EXIT_FAILURE);
	}
}

char calcul_direction(char direction, char lettre, int *bloque_x, int *bloque_y)
{
	/**
	 * @brief Calcule la direction du joueur
	 * @param direction : la direction du joueur, qui sera changé ou non
	 * @param lettre : la lettre qui a été rentré par le joueur
	 * @param bloque_x : variable de blockage sur les x (pour ne pas tourner sur lui même)
	 * @param bloque_y : variable de blockage sur les y (pour ne pas tourner sur lui même)
	 * @return direction : changer ou non, l'on retourne la valeur actuel de la direction du joueur
	 */

	/*Utilisation d'un switch pour gérer les différentes directions*/
	switch (lettre)
	{
	case 'z':
		if (*bloque_y != 2)
		{
			*bloque_x = 0; // l'axe des x est disponible
			*bloque_y = 1; // l'axe des y est bloqué pour le bas
			direction = HAUT;
		}
		break;

	case 's':
		if (*bloque_y != 1)
		{
			*bloque_x = 0; // l'axe des x est disponible
			*bloque_y = 2; // l'axe des y est bloqué pour le haut
			direction = BAS;
		}
		break;

	case 'q':
		if (*bloque_x != 1)
		{
			*bloque_y = 0; // l'axe des y est disponible
			*bloque_x = 2; // l'axe des x est bloqué pour le droite
			direction = GAUCHE;
		}
		break;

	case 'd':
		if (*bloque_x != 2)
		{
			*bloque_y = 0; // l'axe des y est disponible
			*bloque_x = 1; // l'axe des x est bloqué pour le gauche
			direction = DROITE;
		}
		break;
	}

	return direction;
}
void positionner_pomme(int *x, int *y, corp_longeur les_x, corp_longeur les_y, int taille_joueur)
{
	int conf;
	int i;

	printf("\033[1;31m");

	do
	{
		*x = rand() % TAILLE_TABLEAU + DECALAGE_TABLEAU_X;
		*y = rand() % TAILLE_TABLEAU + DECALAGE_TABLEAU_Y;

		for (i = 0; i < taille_joueur; i++)
		{ // Vérifier si la pomme n'as pas été mis dans le joueur même
			if (les_x[i] == *x || les_y[i] == *y)
			{
				conf = 0;
			}
		}
		if (conf == 0)
			conf = -1;
		else
			conf = 1;

	} while (conf != 1);

	afficher(*x, *y, POMME);
}

int collision_avec_pomme(int x_tete, int y_tete, int x_pomme, int y_pomme)
{
	if ((x_tete == x_pomme) && (y_tete == y_pomme))
	{
		system("paplay mange.wav &");
		return 1;
	}
	return 0;
}

int collision_avec_lui_meme(corp_longeur les_x, corp_longeur les_y, int taille_joueur)
{
	int i;
	for (i = 1; i < taille_joueur; i++)
	{
		if ((les_x[0] == les_x[i]) && (les_y[0] == les_y[i]))
		{
			return 1;
		}
	}
	return 0;
}

int main()
{

	char lettre = ' '; // Valeur du caractère entrer

	type_tableau_2d plateau; // Plateau de jeu
	corp_longeur Les_x;		 // Position du corps en X
	corp_longeur Les_y;		 // Position du corps en Y

	corp_longeur Les_x_tampon;
	corp_longeur Les_y_tampon;

	int taille_joueur = TAILLE_INITIAL_DU_JOUEUR;

	char direction = DROITE; // DIrection du joueur, avec la direction initial à droite.

	/*
	Ces deux valeurs permettent de bloquer le serpent s'il veut tourner sur lui même.
	Pour mieux comprendre comment il fonctionne, aller calcule direction.
	*/
	int bloque_x = 1; // 1 = droite, 2 = gauche
	int bloque_y = 0; // 1 = haut, 2 = basZ

	/*
		Pomme
	*/
	int toucher_pomme = 0;
	int x_pomme = 0;
	int y_pomme = 0;

	int col_avec_lui_meme = 0;
	int col_avec_mur = 0;

	system("clear");

	// apparaître à 20 sur le tableau et non sur la console (20 + décalage du tableau dans la console)

	creation_du_serpent(POS_INITIAL_JOUEUR + DECALAGE_TABLEAU_X,
						POS_INITIAL_JOUEUR + DECALAGE_TABLEAU_Y, Les_x, Les_y);

	print_tableau(plateau);

	positionner_pomme(&x_pomme, &y_pomme, Les_x, Les_y, taille_joueur);

	disable_echo();

	
	srand(time(NULL));

	while (lettre != FERMER_JEU && col_avec_lui_meme != 1 && col_avec_mur != 1)
	{

		dessiner_serpent(Les_x, Les_y, taille_joueur);
		usleep(DELAI_IMAGES);
		effacer_serpent(Les_x, Les_y, taille_joueur);
		for (int i = 0; i < taille_joueur; i++)
		{
			Les_x_tampon[i] = Les_x[i];
			Les_y_tampon[i] = Les_y[i];
		}
		// Lire les entrer au clavier
		lettre = lire_entrer();

		// Calcule de la direction
		direction = calcul_direction(direction, lettre, &bloque_x, &bloque_y);

		progresser(Les_x, Les_y, direction, taille_joueur);

		if (toucher_pomme == 1)
		{
			positionner_pomme(&x_pomme, &y_pomme, Les_x, Les_y, taille_joueur);
			toucher_pomme = 0;
			taille_joueur++;
		}

		toucher_pomme = collision_avec_pomme(Les_x[0], Les_y[0], x_pomme, y_pomme);
		col_avec_lui_meme = collision_avec_lui_meme(Les_x, Les_y, taille_joueur);
		col_avec_mur = dans_tableau(Les_x[0], Les_y[0]);

		goto_x_y(5, TAILLE_TABLEAU + 10);
		printf("Score : %d", taille_joueur - 10);
	}

	if (lettre != FERMER_JEU){
		system("paplay mort.wav &");
	}

	goto_x_y(1, CACHER_CURSEUR);
	dessiner_serpent(Les_x_tampon, Les_y_tampon, taille_joueur);
	enable_echo();

	return EXIT_SUCCESS;
}