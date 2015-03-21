/* GIRAUDET Alexis - HALLEREAU François */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glpk.h>

/* Déclarations pour le compteur de temps CPU */
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>

struct timeval start_utime, stop_utime;

struct data {
    int n; /* nombre de sites */
    int * c; /* matrice des distances */
};

int * data_get_ptr(int i, int j, struct data * d)
{
    return d->c + (i % d->n) * d->n + (j % d->n);
}

void crono_start()
{
    struct rusage rusage;

    getrusage(RUSAGE_SELF, &rusage);
    start_utime = rusage.ru_utime;
}

void crono_stop()
{
    struct rusage rusage;

    getrusage(RUSAGE_SELF, &rusage);
    stop_utime = rusage.ru_utime;
}

double crono_ms()
{
    return (stop_utime.tv_sec - start_utime.tv_sec) * 1000 +
           (stop_utime.tv_usec - start_utime.tv_usec) / 1000 ;
}

void lecture_data(char *file, struct data *p)
{

    FILE *fin; /* fichier à lire */
    int i,j; /* indices */
    int val, res; /* entier lu */

    fin = fopen(file,"r"); /* ouverture du fichier en lecture */

    /* lecture et initialisation de n */
    res = fscanf(fin, "%d", &val);
    p->n = val;

    /* allocation et remplissage de la matrice des distances */
    p->c = (int *) malloc (sizeof(int*) * p->n * p->n);
    for(i=0; i<p->n; i++)
    {
        for(j=0; j<p->n; j++)
        {
            res = fscanf(fin,"%d",&val);
            *data_get_ptr(i, j, p) = val;
        }
    }

    /* fermeture du fichier */
    fclose(fin);
}


int main(int argc, char *argv[])
{
    struct data p;
    int n, i, j, pos, nbcreux, nbcontr, nbvar;
    double temps, z;
    int nbsol = 0; /* Compteur du nombre d'appels au solveur GLPK */
    int * ia;
    int * ja;
    double * ar;
    double * x;


    /* test de l'argument */
    if(argc != 2)
    {
        fprintf(stderr, "Usage: robots [DATA FILE]\n");
        return EXIT_FAILURE;
    }

    lecture_data(argv[1], &p);

    printf("%d\n", p.n);
    for (i = 0; i < p.n; i++)
    {
        for (j = 0; j < p.n; j++)
        {
            printf("%d ", *data_get_ptr(i, j, &p));
        }
        printf("\n");
    }
    printf("\n");

    /* Déclarations à compléter... */

    crono_start(); /* Lancement du compteur (juste après le chargement des données à partir d'un fichier */

    glp_prob *prob; /* déclaration du pointeur sur le problème */
    prob = glp_create_prob(); /* allocation mémoire pour le problème */
    glp_set_prob_name(prob, "robots"); /* affectation d'un nom */
    glp_set_obj_dir(prob, GLP_MIN); /* Il s'agit d'un problème de minimisation */

    glp_smcp parm;
    glp_init_smcp(&parm);
    parm.msg_lev = GLP_MSG_OFF; /* Paramètre de GLPK dans la résolution d'un PL en variables continues afin de couper les affichages (dans lesquels on se noierait) */

    glp_iocp parmip;
    glp_init_iocp(&parmip);
    parmip.msg_lev = GLP_MSG_OFF; /* Paramètre de GLPK dans la résolution d'un PL en variables entières afin de couper les affichages (dans lesquels on se noierait) */

    n = p.n;
    nbcontr = 2*n;
    nbvar = n*n;
    nbcreux = 2*n*n - 2*n;

    glp_add_rows(prob, nbcontr);
    for(i=1; i<=nbcontr; i++)
    {
        glp_set_row_bnds(prob, i, GLP_FX, 1.0, 1.0);
    }

    glp_add_cols(prob, nbvar);
    for(i=1; i<=nbvar; i++)
    {
        glp_set_col_bnds(prob, i, GLP_DB, 0.0, 1.0);
        glp_set_col_kind(prob, i, GLP_BV);
    }

    pos = 1;

    for(i=0; i<n; i++)
    {
        for(j=0; j<n; j++)
        {
            glp_set_obj_coef(prob, pos, *data_get_ptr(i, j, &p));
            pos++;
        }
    }

    ia = (int *) malloc ((1 + nbcreux) * sizeof(int));
    ja = (int *) malloc ((1 + nbcreux) * sizeof(int));
    ar = (double *) malloc ((1 + nbcreux) * sizeof(double));

    pos = 1;

    for(i=0; i<n; ++i)
    {
        for(j=0; j<n; ++j)
        {
            if(i != j)
            {
                ia[pos] = i + 1;
                ja[pos] = j + 1 + i * n;
                ar[pos] = 1.0;
                pos++;

                ia[pos] = i + n + 1;
                ja[pos] = j * n + 1 + i;
                ar[pos] = 1.0;
                pos++;
            }
        }
    }

    for(i=1; i<pos; i++)
    {
        printf("ia[%d] = %d; ja[%d] = %d; ar[%d] = %f;\n", i, ia[i], i, ja[i], i, ar[i]);
    }

    /* Les appels glp_simplex(prob,NULL); et gpl_intopt(prob,NULL); (correspondant aux paramètres par défaut) seront ensuite remplacés par glp_simplex(prob,&parm); et glp_intopt(prob,&parmip); */

    glp_load_matrix(prob, nbcreux, ia, ja, ar);

    glp_write_lp(prob,NULL,"robots.lp");

    glp_simplex(prob,NULL);
    glp_intopt(prob,NULL); /* Résolution */
    z = glp_mip_obj_val(prob); /* Récupération de la valeur optimale. Dans le cas d'un problème en variables continues, l'appel est différent : z = glp_get_obj_val(prob);*/
    x = (double *) malloc (nbvar * sizeof(double));
    for(i = 0; i < nbvar; i++) x[i] = glp_mip_col_val(prob,i+1); /* Récupération de la valeur des variables, Appel différent dans le cas d'un problème en variables continues : for(i = 0;i < p.nbvar;i++) x[i] = glp_get_col_prim(prob,i+1);	*/

    printf("z = %lf\n",z);
    printf(" ");
    for(i=0; i<n; i++)printf(" %d", i+1);
    printf("\n");
    pos = 0;
    for(i=0; i<n; i++)
    {
        printf("%d", i+1);
        for(j=0; j<n; j++)
        {
            printf(" %d", (int)x[pos]);
            pos++;
        }
        printf("\n");
    }
    printf("\n");
    /* A compléter ...
            .
            .
            .
    */

    /* Résolution achevée, arrêt du compteur de temps et affichage des résultats */
    crono_stop();
    temps = crono_ms()/1000,0;

    printf("\n");
    puts("Résultat :");
    puts("-----------");
    /* Affichage de la solution sous la forme d'un cycle avec sa longueur à ajouter */
    printf("Temps : %f\n", temps);
    printf("Nombre d'appels à GPLK : %d\n", nbsol);
    printf("Nombre de contraintes ajoutées : %d\n", nbcontr);

    glp_delete_prob(prob);
    free(p.c);
    free(ia);
    free(ja);
    free(ar);
    free(x);

    return EXIT_SUCCESS;
}
