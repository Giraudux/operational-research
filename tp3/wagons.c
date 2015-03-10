/*
 * Recherche Opérationnelle
 * TP3 : Utilisation de GLPK en tant que bibliothèque de fonctions
 *
 * Exercice 2
 * Modèle implicite
 *
 * Alexis Giraudet
 * François Hallereau
 * 602C
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glpk.h>

typedef struct {
    int m; /* nombre de wagons */
    int n; /* nombre de caisses */
    int *p; /* tableau des données du problème */
    int nbvar; /* Nombre de variables */
    int nbcontr; /* Nombre de contraintes */
} donnees;

void lecture_data(char *file, donnees *p)
{

    FILE *fin; /* fichier à lire */
    int i,j; /* indices */
    int val, res; /* entier lu */

    fin = fopen(file,"r"); /* ouverture du fichier en lecture */

    /* lecture et initialisation de m et n */
    res = fscanf(fin,"%d",&val);
    p->m = val;
    res = fscanf(fin,"%d",&val);
    p->n = val;

    /* calcul de nbvar et nbcontr */
    p->nbvar = p->m * p->n +1;
    p->nbcontr = p->m + p->n;

    /* allocation du tableau de données */
    p->p = (int *) malloc (p->n * sizeof(int));

    /* remplissage du tableau avec le poid des caisses */
    for(i=0; i < p->n; i++)
    {
        res = fscanf(fin,"%d",&val);
        p->p[i] = val;
    }

    /* fermeture du fichier */
    fclose(fin);
}


int main(int argc, char *argv[])
{
    /* données du problème */
    donnees p;

    /* structures de données propres à GLPK */
    glp_prob *prob;
    int *ia;
    int *ja;
    double *ar;

    /* variables récupérant les résultats de la résolution du problème (fonction objectif et valeur des variables) */
    double z;
    double *x;

    /* autres déclarations */
    int i,j;
    int nbcreux;
    int pos;

    /* test de l'argument */
    if(argc != 2)
    {
        fprintf(stderr, "Usage: wagons [DATA FILE]\n");
        return EXIT_FAILURE;
    }

    /* chargement des données à partir d'un fichier */
    lecture_data(argv[1],&p);

    printf("%s\n%d %d\n", argv[1], p.m, p.n);
    for(i=0; i<p.n; i++) printf("%d ", p.p[i]);
    printf("\n");

    /* transfert de ces données dans les structures utilisées par la bibliothèque GLPK */
    prob = glp_create_prob();
    glp_set_prob_name(prob, "wagons");
    glp_set_obj_dir(prob, GLP_MIN);

    /* déclaration du nombre de contraintes (nombre de lignes de la matrice des contraintes) : p.nbcontr */
    glp_add_rows(prob, p.nbcontr);

    /* On commence par préciser les bornes sur les constrainte, les indices commencent à 1 (!) dans GLPK */
    for(i=1; i<=p.nbcontr; i++)
    {
        if (i <= p.n) glp_set_row_bnds(prob, i, GLP_FX, 1.0, 1.0);
        else glp_set_row_bnds(prob, i, GLP_UP, 0.0, 0.0);
    }

    /* Déclaration du nombre de variables : p.nbvar */
    glp_add_cols(prob, p.nbvar);

    /* On précise le type des variables, les indices commencent à 1 également pour les variables! */
    for(i=1; i <= p.nbvar-1; i++)
    {
        glp_set_col_bnds(prob, i, GLP_DB, 0.0, 1.0);
        glp_set_col_kind(prob, i, GLP_BV);
    }
    glp_set_col_bnds(prob, p.nbvar, GLP_LO, 0.0, 0.0);

    /* définition des coefficients des variables dans la fonction objectif */
    for(i = 1; i <= p.nbvar-1; i++) glp_set_obj_coef(prob,i,0.0);
    glp_set_obj_coef(prob, p.nbvar, 1.0);

    /* Définition des coefficients non-nuls dans la matrice des contraintes, autrement dit les coefficients de la matrice creuse */
    nbcreux = 2 * p.m * p.n + p.m;

    /* allocation */
    ia = (int *) malloc ((1 + nbcreux) * sizeof(int));
    ja = (int *) malloc ((1 + nbcreux) * sizeof(int));
    ar = (double *) malloc ((1 + nbcreux) * sizeof(double));

    /* remplissage par lignes */
    pos = 1;
    for(i=0; i < p.n; i++)
    {
        for(j=i*p.m; j < i*p.m+p.m; j++)
        {
            ia[pos] = i + 1;
            ja[pos] = j + 1;
            ar[pos] = 1.0;
            pos++;
        }
    }

    for(i=p.n; i < p.n+p.m; i++)
    {
        for(j=0; j < p.n; j++)
        {
            ia[pos] = i + 1;
            ja[pos] = j * p.m + i - p.n + 1;
            ar[pos] = p.p[j];
            pos++;
        }

        ia[pos] = i + 1;
        ja[pos] = p.m * p.n + 1;
        ar[pos] = -1.0;
        pos++;
    }

    /* affichage pour debug */
    for(i=1; i<pos; i++)
    {
        printf("ia[%d] = %d; ja[%d] = %d; ar[%d] = %f;\n", i, ia[i], i, ja[i], i, ar[i]);
    }

    /* chargement de la matrice dans le problème */
    glp_load_matrix(prob,nbcreux,ia,ja,ar);

    /* Optionnel : écriture de la modélisation dans un fichier (utile pour debugger) */
    glp_write_lp(prob,NULL,"wagons.lp");

    /* Résolution, puis lecture des résultats */
    glp_simplex(prob,NULL);
    glp_intopt(prob,NULL);
    z = glp_mip_obj_val(prob);
    x = (double *) malloc (p.nbvar * sizeof(double));
    for(i = 0; i < p.nbvar; i++) x[i] = glp_mip_col_val(prob,i+1);

    printf("z = %lf\n",z);
    for(i = 0; i < p.nbvar; i++) printf("x%c = %d, ",'B'+i,(int)(x[i] + 0.5));
    puts("");

    /* libération mémoire */
    glp_delete_prob(prob);
    free(p.p);
    free(ia);
    free(ja);
    free(ar);
    free(x);

    /* "Les chiffres, c'est pas une science exacte figurez-vous!" Karadoc */
    return EXIT_SUCCESS;
}
