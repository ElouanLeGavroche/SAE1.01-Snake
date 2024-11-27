/**
 * @file main.c
 * @brief Programme d'un jeu de snake.
 * @author Dhennin Elouan
 * @version 4
 * @date 18/11/24
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termio.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>

// CONSTANTE DU JEU

/*Constante du joueur*/
#define TETE_JOUEUR 'O' // Tête du joueur
#define CORP_JOUEUR 'X' // Corp du joueur
#define POS_INITIAL_JOUEUR_X 40
#define POS_INITIAL_JOUEUR_Y 20
#define LONGEUR_MAX 20
#define ZONE_DE_PROTECTION_X 15
#define ZONE_DE_PROTECTION_Y 5
#define TAILLE_INITIAL_DU_JOUEUR 10 // Taille de base du joueur (en réalité 10, car la boucle for commence à 0)
int taille_joueur = 10;

/*Les constantes du jeu*/
#define CONDITION_VICTOIRE 20 // Score requis pour que le jeu se finisse
#define CACHER_CURSEUR 70	  // Valeur absurde pour être sur d'être coller en bas de la console
#define TAILLE_PAVE 5
#define NOMBRE_PAVE 4
#define TAILLE_TABLEAU_Y 40
#define TAILLE_TABLEAU_X 80

/*Constantes des éléments graphique hors joueur*/
#define CARACTERE_EFFACER ' ' // Pour effacer un élément
#define MUR '#'
#define POMME '6'
#define VITESSE_DEPART 70000 // micro_sec
#define ACCELERATION 1.05

/*Les constantes de direction & entrer*/
#define HAUT 'z'
#define BAS 's'
#define DROITE 'd'
#define GAUCHE 'q'
#define FERMER_JEU 'a' // Condition d'arrêt

// Définir les types tableau
// + 1 car la dernière valeur sert au tampon du dernier élément,
// pour l'effacer correctement
typedef int corp_longeur[LONGEUR_MAX + 1];

// Définir le type du tableau à deux dimensions
typedef char type_tableau_2d[TAILLE_TABLEAU_Y][TAILLE_TABLEAU_X];
typedef int conteneur[NOMBRE_PAVE][TAILLE_PAVE][TAILLE_PAVE];

/* Initialisations des fonctions et des procédure */
int kbhit();						  // équivalent d'un INKEYS en BASIC
int dans_tableau(int eltx, int elty); // Vérifie que l'élément se trouve dans le tableau pour le dessiner
int collision_pave(conteneur block_x, conteneur block_y, int tete_x, int tete_y);
int collision_avec_lui_meme(corp_longeur les_x, corp_longeur les_y, int tete_x, int tete_y); // évite que le joueuer rentre en collisiona vec lui même
int verification_des_collision(int tete_x_ou_y, corp_longeur les_x, corp_longeur les_y, char dir, conteneur block_x, conteneur block_y);
int confirmer_position(int indice, corp_longeur liste, int nouvelle_position);
int collision_avec_pomme(int x_tete, int y_tete, int x_pomme, int y_pomme);

void positionner_pomme(int *x, int *y, corp_longeur les_x, corp_longeur les_y, conteneur block_x, conteneur block_y);
void teleportation(int *tete_x, int *tete_y);
void goto_x_y(int x, int y);		 // équivalent d'un LOCATE en Basic
void afficher(int x, int y, char c); // Affiche en lien avec goto_x_y un caractère à l'écran
void init_plateau(type_tableau_2d tableau);
void dessiner_serpent(corp_longeur les_x, corp_longeur les_y);
void effacer_serpent(corp_longeur les_x, corp_longeur les_y);
void effacer(int x, int y);
void progresser(corp_longeur les_x, corp_longeur les_y, char direction, bool *collision_joueur, bool *manger_pomme);
void creation_du_serpent(int x, int y, corp_longeur les_x, corp_longeur les_y);
void enable_echo();
void disable_echo();
void deposer_pave(conteneur pos_des_paves_x, conteneur pos_des_paves_y);
void dessiner_plateau(type_tableau_2d plateau);
void message_defaite();
void victoire(double temps_de_jeu);

char calcul_direction(char direction, char lettre);
char lire_entrer();

float agrandir_joueur(float vitesse_jeu);

int main()
{
	system("clear");

	int vitesse_jeu = VITESSE_DEPART;
	char lettre = CARACTERE_EFFACER; // Valeur du caractère entrer

	type_tableau_2d plateau; // Plateau de jeu
	corp_longeur les_x;		 // Position du corps en X
	corp_longeur les_y;		 // Position du corps en Y

	char direction = DROITE; // DIrection du joueur, avec la direction initial à droite.
	bool collision_joueur = false;

	conteneur pos_des_paves_x; // Contient les pavés dans deux tableau à 3 dimensions
	conteneur pos_des_paves_y;

	/*variable lié à la pomme*/
	int pomme_x;
	int pomme_y;
	bool pomme_ramasser;

	// apparaître à 20 sur le tableau et non sur la console (20 + décalage du tableau dans la console)

	creation_du_serpent(POS_INITIAL_JOUEUR_X + 1, POS_INITIAL_JOUEUR_Y + 1, les_x, les_y);

	init_plateau(plateau);
	dessiner_plateau(plateau);
	deposer_pave(pos_des_paves_x, pos_des_paves_y);

	positionner_pomme(&pomme_x, &pomme_y, les_x, les_y, pos_des_paves_x, pos_des_paves_y);

	disable_echo();

	double tour_jeu = 0; // Pour calculer le temps de jeu

	do
	{

		// Lire les entrer au clavier
		lettre = lire_entrer();

		// Calcule de la direction
		direction = calcul_direction(direction, lettre);

		/*
			Cette condition est ici pour éviter d'effacer le bout du serpent
			alors qu'il n'y a eu aucun déplacement à la fin du jeu.
		*/

		progresser(les_x, les_y, direction, &collision_joueur,
				   &pomme_ramasser);

		/*Collision et gestion du jeu avec la pomme*/
		pomme_ramasser = collision_avec_pomme(les_x[0], les_y[0], pomme_x, pomme_y);

		collision_joueur = verification_des_collision(les_x[0], les_x,
													  les_y, 'x', pos_des_paves_x, pos_des_paves_y);

		if (pomme_ramasser == true)
		{
			vitesse_jeu = agrandir_joueur(vitesse_jeu);

			if (taille_joueur == CONDITION_VICTOIRE)
				collision_joueur = true;

			else
				positionner_pomme(&pomme_x, &pomme_y, les_x, les_y,
								  pos_des_paves_x, pos_des_paves_y);
		}

		// Si le joueur obtien la dernière pomme, on veux qu'ils l'efface
		// Alors malgré le fait qu'il aie fini, on actualise ça position pour
		// Montrer la pomme manger
		if (collision_joueur != 1 || taille_joueur == CONDITION_VICTOIRE)
		{
			effacer_serpent(les_x, les_y);
			dessiner_serpent(les_x, les_y);
			usleep(vitesse_jeu);
		}

		/*
		Le temps de jeu étant dérisoire, nous prenons en compte uniquement le temps
		D'attente entre deux image. Qui est finalement la majorité du temps de jeu
		du joueur. Attendre...
		*/
		tour_jeu = tour_jeu + vitesse_jeu;
	} while ((lettre != FERMER_JEU) && (collision_joueur == false));

	enable_echo();

	if (taille_joueur == CONDITION_VICTOIRE)
	{
		// Si le joueur a gagner, calcule de sont score et de sont temps de jeu
		double temps_de_jeu = (tour_jeu / CLOCKS_PER_SEC);
		victoire(temps_de_jeu);
	}
	else
	{
		message_defaite();
	}

	goto_x_y(1, CACHER_CURSEUR);

	return EXIT_SUCCESS;
}

float agrandir_joueur(float vitesse_jeu)
{
	/**
	 * @brief agrandi le joueur, et l'accélère
	 * @param vitesse_jeu l'attente entre deux image en ms
	 * @return vitesse_jeu l'attente réduite avec la nouvelle taille
	 */
	taille_joueur++;						  // Agrandir joueur
	vitesse_jeu = vitesse_jeu / ACCELERATION; // Accéléré joueur
	return vitesse_jeu;
}

void victoire(double temps_de_jeu)
{
	/**
	 * @brief gère un genre de menu de fin de jeu pour montrer le "résumer" de la
	 * 			partie du joueur.
	 * @param temps_de_jeu : est le temps qu'a pris le joueur a
	 * 			attendre entre deux image.
	 */

	char nom[25];
	goto_x_y(2, TAILLE_TABLEAU_Y - 3);
	printf("FIN DU JEU !!!");
	goto_x_y(2, TAILLE_TABLEAU_Y - 2);
	printf("Entrez votre nom : ");

	scanf("%s", nom);

	goto_x_y(2, TAILLE_TABLEAU_Y - 1);
	printf("%s à fini le jeu en %2f secondes !!", nom, temps_de_jeu);
}

void message_defaite()
{
	/**
	 * @brief Message signalent au joueur ça défaite
	 */

	goto_x_y(TAILLE_TABLEAU_X / 2 - 8, TAILLE_TABLEAU_Y / 2);
	printf("/---------------\\");
	goto_x_y(TAILLE_TABLEAU_X / 2 - 8, TAILLE_TABLEAU_Y / 2 + 1);
	printf("|Vous avez Perdu|");
	goto_x_y(TAILLE_TABLEAU_X / 2 - 8, TAILLE_TABLEAU_Y / 2 + 2);
	printf("\\---------------/");
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
}

void init_plateau(type_tableau_2d tableau)
{
	/**
	 * @brief Permet de dessiner le tableau de jeu. Mais aussi l'interface autour !
	 * @param tableau : tableau à deux dimension qui représente le tableau de jeu
	 */

	int lig = 0;
	int col = 0;

	for (lig = 0; lig < TAILLE_TABLEAU_Y; lig++)
	{

		for (col = 0; col < TAILLE_TABLEAU_X; col++)
		{

			if (col == TAILLE_TABLEAU_X / 2 ||
				lig == TAILLE_TABLEAU_Y / 2 ||
				(col == TAILLE_TABLEAU_Y - 1) / 2 ||
				(lig == TAILLE_TABLEAU_Y - 1) / TAILLE_TABLEAU_Y / 2)
			{
				tableau[lig][col] = CARACTERE_EFFACER;
			}
			else if (lig == 0 || col == 0 ||
					 lig == TAILLE_TABLEAU_Y - 1 || col == TAILLE_TABLEAU_X - 1)
			{
				tableau[lig][col] = MUR;
			}

			else
			{
				tableau[lig][col] = CARACTERE_EFFACER;
			}
		}
	}
}

void dessiner_plateau(type_tableau_2d plateau)
{
	/**
	 * @brief va faire apparaître au lancement du jeu les bordure du tableau
	 * @param plateau : tableau à deux dimensions déjà initialiser
	 */
	int i, y;
	for (i = 0; i < TAILLE_TABLEAU_Y; i++)
	{
		for (y = 0; y < TAILLE_TABLEAU_X; y++)
		{
			printf("\033[44m"); // Un peu de couleur
			afficher(y + 1, i + 1, plateau[i][y]);
		}
	}
	fflush(stdout);

	goto_x_y((TAILLE_TABLEAU_X / 2) - 5, TAILLE_TABLEAU_Y + 1); // Centrer le titre
	printf("\033[0;36m");
	printf("\033[44m");
	printf("Snake 3.00");
}

int collision_pave(conteneur block_x, conteneur block_y, int tete_x, int tete_y)
{
	/**
	 * @brief regarde s'il y a collision avec les différent pavé du jeu
	 * @param block_x liste à 3 dimensions des x des pavés
	 * @param block_y liste à 3 dimensions des y des pavés
	 * @param tete_x position de la tête en x
	 * @param tete_x position de la tête en y
	 */

	int i, y, z;
	bool collision = 0;

	for (z = 0; z <= NOMBRE_PAVE; z++)
	{
		for (i = 0; i <= TAILLE_PAVE; i++)
		{
			for (y = 0; y <= TAILLE_PAVE; y++)

			{

				if (block_x[z][i][y] == tete_x && tete_y == block_y[z][i][y])
				{
					collision = 1;
				}
			}
		}
	}

	return collision;
}

int verification_des_collision(int tete_x_ou_y, corp_longeur les_x, corp_longeur les_y, char dir, conteneur block_x, conteneur block_y)
{
	/**
	 * @brief Vérifie les collisions pouvant mettre fin à la partie.
	 * @param tete_x_ou_y position x ou y de la tête
	 * @param les_x positions du corps en x
	 * @param les_y positions du corps en y
	 * @param dir indique si la vérification concerne la position x ou y
	 * @param block_x tableau à 3 dimensions des pavés en x
	 * @param block_y tableau à 3 dimensions des pavés en y
	 * @return 1 en cas de collision, sinon 0
	 */

	int position_tete_x = (dir == 'x') ? tete_x_ou_y : les_x[0];
	int position_tete_y = (dir == 'x') ? les_y[0] : tete_x_ou_y;
	return (dans_tableau(position_tete_x, position_tete_y) == 0 ||

			collision_pave(block_x, block_y, position_tete_x, position_tete_y) == 1 ||

			collision_avec_lui_meme(les_x, les_y, position_tete_x,
									position_tete_y) == 1);
}

void teleportation(int *tete_x, int *tete_y)
{
	/**
	 * @brief téléporte le joueur d'un coté à l'autre du monde avec les "portail"
	 * @param *tete_x tête du joueur en x en sortie
	 * @param *tete_y tête du joueur en y en sortie
	 */

	if (*tete_x > TAILLE_TABLEAU_X && *tete_y == TAILLE_TABLEAU_Y / 2 + 1)
		*tete_x = 1; // Téléporteur à droite du monde

	else if (*tete_x < 1 && *tete_y == TAILLE_TABLEAU_Y / 2 + 1)
		*tete_x = TAILLE_TABLEAU_X; // Téléporteur à gauche du monde

	else if ((*tete_y > TAILLE_TABLEAU_Y) && (*tete_y == TAILLE_TABLEAU_X / 2 + 1))
		*tete_y = 1; // Téléporteur en bas du monde

	else if ((*tete_y < 1) && (*tete_x == TAILLE_TABLEAU_X / 2 + 1))
		*tete_y = TAILLE_TABLEAU_Y; // Téléporteur en haut du monde
}

void progresser(corp_longeur les_x, corp_longeur les_y, char direction, bool *collision_joueur, bool *pomme_manger)
{
	/**
	 * @brief Calcule la nouvelle position du serpent quand il avance sans intervention du joueur
	 * @param les_x liste des valeurs en x
	 * @param les_y liste des valeurs en y
	 * @param *collision_joueur vérifie s'il y a une collision 0 si non, 1 si oui
	 */

	if (*pomme_manger == true)
	{
		*pomme_manger = false;
		goto_x_y(1, TAILLE_TABLEAU_Y + 1);
		printf("score : %d", taille_joueur - TAILLE_INITIAL_DU_JOUEUR);
	}

	if (*collision_joueur == false)
	{

		int i = 0;

		les_x[taille_joueur + 1] = les_x[taille_joueur];
		les_y[taille_joueur + 1] = les_y[taille_joueur];

		for (i = taille_joueur; i >= 1; i--)
		{
			les_x[i] = les_x[i - 1];
			les_y[i] = les_y[i - 1];
		}

		/* Utilisation d'opération Ternaire pour séléctionner la direction*/
		if (direction == HAUT || direction == BAS)
			les_y[0] = (direction == HAUT) ? les_y[0] - 1 : les_y[0] + 1;
		else if (direction == DROITE || direction == GAUCHE)
			les_x[0] = (direction == GAUCHE) ? les_x[0] - 1 : les_x[0] + 1;

		teleportation(&les_x[0], &les_y[0]);
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
	int dedans = 0;
	// Gestion des téléporteur au quatre coin du niveau
	if (((eltx >= TAILLE_TABLEAU_X) && (elty == TAILLE_TABLEAU_Y / 2 + 1)) ||
		((eltx <= 1) && (elty == TAILLE_TABLEAU_Y / 2 + 1)) ||
		((elty >= TAILLE_TABLEAU_Y) && (eltx == TAILLE_TABLEAU_X / 2 + 1)) ||
		((elty <= 1) && (eltx == TAILLE_TABLEAU_X / 2 + 1)))
	{
		dedans = 1;
	}
	else if ((eltx < TAILLE_TABLEAU_X	 // Si le joueur touche le mur droit
			  && eltx >= 2)				 // Si le joueur touche le mur gauche
			 && (elty < TAILLE_TABLEAU_Y // Si le joueur touche le mur bas
				 && elty >= 2))			 // Si le joueur touche le mur haut
	{
		dedans = 1;
	}
	return dedans;
}

void dessiner_serpent(corp_longeur les_x, corp_longeur les_y)
{
	/**
	 * @brief Dessine le joueur en premier, et fait une boucle pour dessiner les enfants
	 * (le corps) du joueur en fonction de la taille du joueur.
	 * @param les_x liste des valeurs en x
	 * @param les_y liste des valeurs en y
	 */

	int i;
	for (i = 1; i < taille_joueur; i++)
	{

		printf("\033[0;35m");
		printf("\033[44m");
		afficher(les_x[i], les_y[i], CORP_JOUEUR);
	}

	printf("\033[0;35m");
	printf("\033[44m");
	afficher(les_x[0], les_y[0], TETE_JOUEUR);

	fflush(stdout);
}

void effacer(int x, int y)
{
	/**
	 * @brief effacer un caractère à une position voulu
	 * @param x position x
	 * @param y position y
	 */
	afficher(x, y, CARACTERE_EFFACER);
	fflush(stdout);
}

void effacer_serpent(corp_longeur les_x, corp_longeur les_y)
{
	/**
	 * @brief Va de la fin jusqu'à la tête du joueur pour remplacer avec un caractère vide
	 * @param les_x liste des valeurs en x
	 * @param les_y liste des valeurs en y
	 */

	// Efface uniquement le dernier éléments du joueura
	effacer(les_x[taille_joueur], les_y[taille_joueur]);
}

char lire_entrer()
{
	/**
	 * @brief va lire les entrer de l'utilisateur
	 * @return retourne la lettre qui a été entrer par l'utilisateur
	 */

	int entrer;
	char lettre = CARACTERE_EFFACER;
	entrer = kbhit();
	if (entrer == 1)
	{						// Vérifie à chaque "tour de jeu" si quelque chose est rentré
		lettre = getchar(); // Va regarder dans le buffer la lettre à rechercher
	}
	return lettre;
}

void creation_du_serpent(int x, int y, corp_longeur les_x, corp_longeur les_y)
{
	/**
	 * @brief Fonction qui viens crée le serpent la premièrer fois.
	 * @param x position initial en x
	 * @param y position initial en y
	 * @param les_x tableau des position en x
	 * @param les_y tableau des position en y
	 *
	 */
	int i = 0;

	for (i = 0; i < taille_joueur; i++)
	{
		les_x[i] = x - i;
		les_y[i] = y;
	}
}

char calcul_direction(char direction, char lettre)
{
	/**
	 * @brief Calcule la direction du joueur
	 * @param direction : la direction du joueur, qui sera changé ou non
	 * @param lettre : la lettre qui a été rentré par le joueur
	 * @return direction : changer ou non, l'on retourne la valeur actuel de la direction du joueur
	 */

	/* Utilisation de conditions pour gérer les différentes directions */
	if (lettre == HAUT && direction != BAS)
		direction = HAUT;
	else if (lettre == BAS && direction != HAUT)
		direction = BAS;
	else if (lettre == GAUCHE && direction != DROITE)
		direction = GAUCHE;
	else if (lettre == DROITE && direction != GAUCHE)
		direction = DROITE;

	return direction;
}

int collision_avec_pomme(int x_tete, int y_tete, int x_pomme, int y_pomme)
{
	/**
	 * @brief véfifie si le joueur est en collision avec la pomme
	 * @param x_tete tête du joueur en x
	 * @param y_tete tête du joueur en y
	 * @param x_pomme position de la pomme en x
	 * @param y_pomme position de la pomme en y
	 * @return
	 */

	int collision = 0;
	if ((x_tete == x_pomme) && (y_tete == y_pomme))
	{
		collision = 1;
	}
	return collision;
}

int collision_avec_lui_meme(corp_longeur les_x, corp_longeur les_y, int tete_x, int tete_y)
{
	/**
	 * @brief regarde si la tête du joueur rentre dans le corps du joueur
	 * @param les_x corps en x du joueur
	 * @param les_y corps en y du joueur
	 * @param tete_x
	 * @param tete_y
	 * @return 0 si pas de collision, 1 si collision
	 */
	int i;
	bool collision = false;
	for (i = 1; i < taille_joueur; i++)
	{
		if ((tete_x == les_x[i]) && (tete_y == les_y[i]))
		{
			collision = true;
		}
	}
	return collision;
}

int confirmer_position(int indice, corp_longeur liste, int nouvelle_position)
{
	/**
	 * @brief permet de savoir si la position voulu par le pavé, n'as pas déjà été prise par un autre block
	 * @param indice est le n eme pavé poser
	 * @param liste liste des position déjà utilisé par les pavé précédent
	 * @param nouvelle_position la nouvelle position choisit par le pavé
	 * @return 0 si la position est bonne, 1 si elle est déjà utiliser par un autre pavé
	 */
	int i;
	int identique = 0;

	for (i = 0; i < indice; i++)
	{
		if (liste[i] == nouvelle_position)
		{
			identique = 1;
		}
	}
	return identique;
}

void deposer_pave(conteneur position_x, conteneur position_y)
{
	/**
	 * @brief dépose les pavés dans le monde
	 * @param position_x position des x des différent pavés
	 * @param position_y position des y des différent pavés
	 */
	int pos_x, pos_y, i, y, z, tampon;

	// Ce sont les variable requis pour connaitre où en est la boucle dans le positionnement
	// Des pavés. Et d'y inscrire les position interdite par ceux-ci
	int indice = 0;
	corp_longeur position_interdite_x = {}; // Initialiser toutes les valeurs du tableau à 0
	corp_longeur position_interdite_y = {};

	srand(time(NULL)); // Rendre l'aléatoire plus aléatoire

	for (z = 0; z < NOMBRE_PAVE; z++)
	{

		// Génération aléatoire en x et en y des pavé, en evitant qu'ils apparraissent sur le joueur
		// En créant une protection de 10 block autour de la tête (devant, derrière, haut et en bas)
		do
		{
			pos_x = rand() % (TAILLE_TABLEAU_X - TAILLE_PAVE - 3) + 3;
			tampon = pos_x;
			pos_y = rand() % (TAILLE_TABLEAU_Y - TAILLE_PAVE - 3) + 3;

		} while (

			// Vérification du positionnement dans le tableau
			(pos_x >= POS_INITIAL_JOUEUR_X - ZONE_DE_PROTECTION_X && pos_x <= POS_INITIAL_JOUEUR_X + ZONE_DE_PROTECTION_X &&
			 pos_y >= POS_INITIAL_JOUEUR_Y - ZONE_DE_PROTECTION_Y && pos_y <= POS_INITIAL_JOUEUR_Y + ZONE_DE_PROTECTION_Y)

			// Vérification du positionnement les uns sur les autres
			|| (confirmer_position(indice, position_interdite_x, pos_x) == 1 && confirmer_position(indice, position_interdite_y, pos_y) == 1));

		position_interdite_x[indice] = pos_x;
		position_interdite_y[indice] = pos_y;
		indice++;

		for (i = 0; i < TAILLE_PAVE; i++)
		{
			for (y = 0; y < TAILLE_PAVE; y++)
			{
				printf("\033[0;31m");
				printf("\033[0;44m");

				afficher(pos_x, pos_y, MUR);
				position_x[z][i][y] = pos_x;
				position_y[z][i][y] = pos_y;

				pos_x++;
			}
			pos_x = tampon;
			pos_y++;
		}
	}
	fflush(stdout);
}

void positionner_pomme(int *x, int *y, corp_longeur les_x, corp_longeur les_y, conteneur block_x, conteneur block_y)
{
	/**
	 * @brief pose la pomme de manière aléatoire sur le tableau avec certaine contrainte
	 * @param *x position en sortie de la pomme en x
	 * @param *y position en sortie de la pomme en y
	 * @param les_x corps du joueur en x
	 * @param les_y corps du joueur en y
	 * @param block_x position en x de tout les pavés
	 * @param block_y position en y de tout les pavés
	 */

	int conf;
	int i;
	int j, f, z;

	printf("\033[1;31m");

	do
	{
		/* Génère un nombre restant dans le tableau*/
		*x = rand() % (TAILLE_TABLEAU_X - 2) + 2;
		*y = rand() % (TAILLE_TABLEAU_Y - 2) + 2;

		// Vérifier si la pomme n'as pas été mis dans le joueur même
		for (i = 0; i < taille_joueur; i++)
		{
			if (les_x[i] == *x || les_y[i] == *y)
			{
				conf = 0;
			}
		}

		// Vérifie que la pomme n'est pas en collision avec un pavé
		for (j = 0; j < NOMBRE_PAVE; j++)
		{
			for (f = 0; f < TAILLE_PAVE; f++)
			{
				for (z = 0; z < TAILLE_PAVE; z++)
				{
					if (block_x[j][f][z] == *x && block_y[j][f][z] == *y)
					{
						conf = 0;
					}
				}
			}
		}

		if (conf == 0)
			conf = -1;
		else
			conf = 1;

	} while (conf != 1);

	afficher(*x, *y, POMME); // Afficher la pomme
	fflush(stdout);
}

/*		FONCTION DONNER PAR LES INSTRUCTIONS       */

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
