/* Utilisation de GLPK en mode bibliothèque */
/* Il s'agit de l'exercice 2.2 des feuilles de TD, qui a servi à illustrer l'utilisation d'une matrice creuse avec GNUMathProg */
/* Ici, toutes les données sont saisies "en dur" dans le code, et nous nous permettons donc des allocations statiques. 
   En cas de lecture des données dans un fichier, des allocations dynamiques seraient nécessaires. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glpk.h> /* Nous allons utiliser la bibliothèque de fonctions de GLPK */

#define NBVAR 16 /* nombre de caisses */
#define NBCONTR 3 /* nombre de wagons */
#define NBCREUX 3*(NBVAR+1)+3*NBVAR


int main(int argc, char *argv[])
{	
	/* structures de données propres à GLPK */
	
	glp_prob *prob; // déclaration d'un pointeur sur le problème
	int ia[1 + NBCREUX];
	int ja[1 + NBCREUX];
	double ar[1 + NBCREUX]; // déclaration des 3 tableaux servant à définir la partie creuse de la matrice des contraintes

	/* variables récupérant les résultats de la résolution du problème (fonction objectif et valeur des variables) */

	int i;
	double z; 
	double x[NBVAR]; 
	
	/* Les déclarations suivantes sont optionnelles, leur but est de donner des noms aux variables et aux contraintes.
	   Cela permet de lire plus facilement le modèle saisi si on en demande un affichage à GLPK, ce qui est souvent utile pour détecter une erreur! */
	
	char nomcontr[NBCONTR][8]; /* ici, les contraintes seront nommées "salle1", "salle2",... */
	char numero[NBCONTR][3]; /* pour un nombre à deux chiffres */	
	char nomvar[NBVAR][3]; /* "xA", "xB", ... */
	
	/* Création d'un problème (initialement vide) */
	
	prob = glp_create_prob(); /* allocation mémoire pour le problème */ 
	glp_set_prob_name(prob, "wagon"); /* affectation d'un nom (on pourrait mettre NULL) */
	glp_set_obj_dir(prob, GLP_MIN); /* Il s'agit d'un problème de minimisation, on utiliserait la constante GLP_MAX dans le cas contraire */
	
	/* Déclaration du nombre de contraintes (nombre de lignes de la matrice des contraintes) : NBCONTR */
	
	glp_add_rows(prob, NBCONTR); 

	/* On commence par préciser les bornes sur les constrainte, les indices des contraintes commencent à 1 (!) dans GLPK */

	for(i = 1;i <= NBCONTR;i++)
	{
		/* partie optionnelle : donner un nom aux contraintes */
		strcpy(nomcontr[i-1], "salle");
		sprintf(numero[i-1], "%d", i);
		strcat(nomcontr[i-1], numero[i-1]); /* Les contraintes sont nommés "salle1", "salle2"... */		
		glp_set_row_name(prob, i, nomcontr[i-1]); /* Affectation du nom à la contrainte i */
		
		/* partie indispensable : les bornes sur les contraintes */
		if (i == 4) glp_set_row_bnds(prob, i, GLP_LO, 2.0, 0.0);
		else glp_set_row_bnds(prob, i, GLP_LO, 1.0, 0.0);
		/* Avec GLPK, on peut définir simultanément deux contraintes, si par exemple, on a pour une contrainte i : "\sum x_i >= 0" et "\sum x_i <= 5",
		   on écrit alors : glp_set_row_bnds(prob, i, GLP_DB, 0.0, 5.0); la constante GLP_DB signifie qu'il y a deux bornes sur "\sum x_i" qui sont ensuite données.
		   Ici, nous n'avons qu'une seule contrainte du type "\sum x_i >= 1" (ou "\sum x_i >= 2" pour la contrainte 4) soit une borne inférieure sur "\sum x_i", on écrit donc glp_set_row_bnds(prob, i, GLP_LO, 1.0, 0.0); le paramètre "0.0" est ignoré. 
		   Les autres constantes sont GLP_UP (borne supérieure sur le membre de gauche de la contrainte) et GLP_FX (contrainte d'égalité).   
		 Remarque : les membres de gauches des contraintes "\sum x_i ne sont pas encore saisis, les variables n'étant pas encore déclarées dans GLPK */ 
	}	

	/* Déclaration du nombre de variables : NBVAR */
	
	glp_add_cols(prob, NBVAR); 
	
	/* On précise le type des variables, les indices commencent à 1 également pour les variables! */
	
	for(i = 1;i <= NBVAR;i++)
	{
		/* partie optionnelle : donner un nom aux variables */
		sprintf(nomvar[i-1],"x%c",'B'+i-1);
		glp_set_col_name(prob, i , nomvar[i-1]); /* Les variables sont nommées "xA", "xB"... afin de respecter les noms de variables de l'exercice 2.2 */
		
		/* partie obligatoire : bornes éventuelles sur les variables, et type */
		glp_set_col_bnds(prob, i, GLP_DB, 0.0, 1.0); /* bornes sur les variables, comme sur les contraintes */
		glp_set_col_kind(prob, i, GLP_BV);	/* les variables sont par défaut continues, nous précisons ici qu'elles sont binaires avec la constante GLP_BV, on utiliserait GLP_IV pour des variables entières */	
	} 

	/* définition des coefficients des variables dans la fonction objectif */

	for(i = 1;i <= NBVAR;i++) glp_set_obj_coef(prob,i,1.0); // Tous les coûts sont ici à 1! 
	
	/* Définition des coefficients non-nuls dans la matrice des contraintes, autrement dit les coefficients de la matrice creuse */
	/* Les indices commencent également à 1 ! */
	
	/**/
	ia[1] = 1; ja[1] = 1; ar[1] = 1.0;
    ia[2] = 1; ja[2] = 2; ar[2] = 1.0;
    ia[3] = 1; ja[3] = 3; ar[3] = 1.0;
	
	ia[4] = 2; ja[4] = 4; ar[4] = 1.0;
    ia[5] = 2; ja[5] = 5; ar[5] = 1.0;
    ia[6] = 2; ja[6] = 6; ar[6] = 1.0;

	ia[7] = 3; ja[7] = 7; ar[7] = 1.0;
    ia[8] = 3; ja[8] = 8; ar[8] = 1.0;
    ia[9] = 3; ja[9] = 9; ar[9] = 1.0;

	ia[10] = 4; ja[10] = 10; ar[10] = 1.0;
    ia[11] = 4; ja[11] = 11; ar[11] = 1.0;
    ia[12] = 4; ja[12] = 12; ar[12] = 1.0;

	ia[13] = 5; ja[13] = 13; ar[13] = 1.0;
    ia[14] = 5; ja[14] = 14; ar[14] = 1.0;
    ia[15] = 5; ja[15] = 15; ar[15] = 1.0;

	ia[16] = 6; ja[16] = 16; ar[16] = 1.0;
    ia[17] = 6; ja[17] = 17; ar[17] = 1.0;
    ia[18] = 6; ja[18] = 18; ar[18] = 1.0;

	ia[19] = 7; ja[19] = 19; ar[19] = 1.0;
    ia[20] = 7; ja[20] = 20; ar[20] = 1.0;
    ia[21] = 7; ja[21] = 21; ar[21] = 1.0;

	ia[22] = 8; ja[22] = 22; ar[22] = 1.0;
    ia[23] = 8; ja[23] = 23; ar[23] = 1.0;
    ia[24] = 8; ja[24] = 24; ar[24] = 1.0;

	ia[25] = 9; ja[25] = 25; ar[25] = 1.0;
    ia[26] = 9; ja[26] = 26; ar[26] = 1.0;
    ia[27] = 9; ja[27] = 27; ar[27] = 1.0;

	ia[28] = 10; ja[28] = 28; ar[28] = 1.0;
    ia[29] = 10; ja[29] = 29; ar[29] = 1.0;
    ia[30] = 10; ja[30] = 30; ar[30] = 1.0;

	ia[31] = 11; ja[31] = 31; ar[31] = 1.0;
    ia[32] = 11; ja[32] = 32; ar[32] = 1.0;
    ia[33] = 11; ja[33] = 33; ar[33] = 1.0;

	ia[34] = 12; ja[34] = 34; ar[34] = 1.0;
    ia[35] = 12; ja[35] = 35; ar[35] = 1.0;
    ia[36] = 12; ja[36] = 36; ar[36] = 1.0;

	ia[37] = 13; ja[37] = 37; ar[37] = 1.0;
    ia[38] = 13; ja[38] = 38; ar[38] = 1.0;
    ia[39] = 13; ja[39] = 39; ar[39] = 1.0;

	ia[40] = 14; ja[40] = 40; ar[40] = 1.0;
    ia[41] = 14; ja[41] = 41; ar[41] = 1.0;
    ia[42] = 14; ja[42] = 42; ar[42] = 1.0;

	ia[43] = 15; ja[43] = 43; ar[43] = 1.0;
    ia[44] = 15; ja[44] = 44; ar[44] = 1.0;
    ia[45] = 15; ja[45] = 45; ar[45] = 1.0;

	ia[46] = 16; ja[46] = 46; ar[46] = 1.0;
    ia[47] = 16; ja[47] = 47; ar[47] = 1.0;
    ia[48] = 16; ja[48] = 48; ar[48] = 1.0;

    /**/
	ia[49] = 17; ja[49] = 1; ar[49] = 34.0;
    ia[50] = 17; ja[50] = 4; ar[50] = 6.0;
    ia[51] = 17; ja[51] = 7; ar[51] = 8.0;
	ia[52] = 17; ja[52] = 10; ar[52] = 17.0;
    ia[53] = 17; ja[53] = 13; ar[53] = 16.0;
    ia[54] = 17; ja[54] = 16; ar[54] = 5.0;
	ia[55] = 17; ja[55] = 19; ar[55] = 13.0;
    ia[56] = 17; ja[56] = 22; ar[56] = 21.0;
    ia[57] = 17; ja[57] = 25; ar[57] = 25.0;
	ia[58] = 17; ja[58] = 28; ar[58] = 31.0;
    ia[59] = 17; ja[59] = 31; ar[59] = 14.0;
    ia[60] = 17; ja[60] = 34; ar[60] = 13.0;
	ia[61] = 17; ja[61] = 37; ar[61] = 33.0;
    ia[62] = 17; ja[62] = 40; ar[62] = 9.0;
    ia[63] = 17; ja[63] = 43; ar[63] = 25.0;
    ia[64] = 17; ja[64] = 46; ar[64] = 25.0;
    ia[65] = 17; ja[65] = 49; ar[65] = -1.0;

	ia[49] = 18; ja[49] = 2; ar[49] = 34.0;
    ia[50] = 18; ja[50] = 5; ar[50] = 6.0;
    ia[51] = 18; ja[51] = 8; ar[51] = 8.0;
	ia[52] = 18; ja[52] = 11; ar[52] = 17.0;
    ia[53] = 18; ja[53] = 14; ar[53] = 16.0;
    ia[54] = 18; ja[54] = 17; ar[54] = 5.0;
	ia[55] = 18; ja[55] = 20; ar[55] = 13.0;
    ia[56] = 18; ja[56] = 23; ar[56] = 21.0;
    ia[57] = 18; ja[57] = 26; ar[57] = 25.0;
	ia[58] = 18; ja[58] = 29; ar[58] = 31.0;
    ia[59] = 18; ja[59] = 32; ar[59] = 14.0;
    ia[60] = 18; ja[60] = 35; ar[60] = 13.0;
	ia[61] = 18; ja[61] = 38; ar[61] = 33.0;
    ia[62] = 18; ja[62] = 41; ar[62] = 9.0;
    ia[63] = 18; ja[63] = 44; ar[63] = 25.0;
    ia[64] = 18; ja[64] = 47; ar[64] = 25.0;
    ia[65] = 18; ja[65] = 50; ar[65] = -1.0;

	ia[49] = 19; ja[49] = 3; ar[49] = 34.0;
    ia[50] = 19; ja[50] = 6; ar[50] = 6.0;
    ia[51] = 19; ja[51] = 9; ar[51] = 8.0;
	ia[52] = 19; ja[52] = 12; ar[52] = 17.0;
    ia[53] = 19; ja[53] = 15; ar[53] = 16.0;
    ia[54] = 19; ja[54] = 18; ar[54] = 5.0;
	ia[55] = 19; ja[55] = 21; ar[55] = 13.0;
    ia[56] = 19; ja[56] = 24; ar[56] = 21.0;
    ia[57] = 19; ja[57] = 27; ar[57] = 25.0;
	ia[58] = 19; ja[58] = 30; ar[58] = 31.0;
    ia[59] = 19; ja[59] = 33; ar[59] = 14.0;
    ia[60] = 19; ja[60] = 36; ar[60] = 13.0;
	ia[61] = 19; ja[61] = 39; ar[61] = 33.0;
    ia[62] = 19; ja[62] = 42; ar[62] = 9.0;
    ia[63] = 19; ja[63] = 45; ar[63] = 25.0;
    ia[64] = 19; ja[64] = 48; ar[64] = 25.0;
    ia[65] = 19; ja[65] = 51; ar[65] = -1.0;

	/* chargement de la matrice dans le problème */
	
	glp_load_matrix(prob,NBCREUX,ia,ja,ar); 
	
	/* Optionnel : écriture de la modélisation dans un fichier (TRES utile pour debugger!) */

	glp_write_lp(prob,NULL,"wagon.lp");

	/* Résolution, puis lecture des résultats */
	
	glp_simplex(prob,NULL);	glp_intopt(prob,NULL); /* Résolution */
	z = glp_mip_obj_val(prob); /* Récupération de la valeur optimale. Dans le cas d'un problème en variables continues, l'appel est différent : z = glp_get_obj_val(prob); */
	for(i = 0;i < NBVAR; i++) x[i] = glp_mip_col_val(prob,i+1); /* Récupération de la valeur des variables, Appel différent dans le cas d'un problème en variables continues : for(i = 0;i < p.nbvar;i++) x[i] = glp_get_col_prim(prob,i+1); */

	printf("z = %lf\n",z);
	for(i = 0;i < NBVAR;i++) printf("x%c = %d, ",'B'+i,(int)(x[i] + 0.5)); /* un cast est ajouté, x[i] pourrait être égal à 0.99999... */ 
	puts("");

	/* libération mémoire */
	glp_delete_prob(prob); 

	/* J'adore qu'un plan se déroule sans accroc! */
	return 0;
}
