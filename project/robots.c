/* NOM1 Prénom1 - NOM2 Prénom2 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glpk.h>

/* Déclarations pour le compteur de temps CPU */
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>

struct timeval start_utime, stop_utime;

typedef struct {
    int n; /* nombre de sites */
    int ** c; /* matrice des distances */
} donnees;

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

void lecture_data(char *file, donnees *p)
{

    FILE *fin; /* fichier à lire */
    int i,j; /* indices */
    int val, res; /* entier lu */

    fin = fopen(file,"r"); /* ouverture du fichier en lecture */

    /* lecture et initialisation de n */
    res = fscanf(fin,"%d",&val);
    p->n = val;

    /* allocation et remplissage de la matrice des distances */
    p->c = (int **) malloc (p->n * sizeof(int*));
    for(i=0; i<p->n; i++)
    {
    	p->c[i] = (int*) malloc(p->n * sizeof(int));
    	for(j=0; j<p->n; j++)
    	{
    		res = fscanf(fin,"%d",&val);
    		p->c[i][j] = val;
    	}
    }

    /* fermeture du fichier */
    fclose(fin);
}


int main(int argc, char *argv[])
{
	donnees p;
	int n, i, j, pos, nbcreux;
	double temps;
	int nbsol = 0; /* Compteur du nombre d'appels au solveur GLPK */ 
	int nbcontr = 0; /* Compteur du nombre de contraintes ajoutées pour obtenir une solution composée d'un unique cycle */
	int * ia;
	int * ja;
	double * ar;


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
    		printf("%d ", p.c[i][j]);
    	}
    	printf("\n");
    }
    printf("\n");

	/* Déclarations à compléter... */

	crono_start(); /* Lancement du compteur (juste après le chargement des données à partir d'un fichier */

	glp_prob *prob; /* déclaration du pointeur sur le problème */
	prob = glp_create_prob(); /* allocation mémoire pour le problème */ 
	glp_set_prob_name(prob, "Robot"); /* affectation d'un nom */
	glp_set_obj_dir(prob, GLP_MIN); /* Il s'agit d'un problème de minimisation */

	glp_smcp parm;
	glp_init_smcp(&parm);
	parm.msg_lev = GLP_MSG_OFF; /* Paramètre de GLPK dans la résolution d'un PL en variables continues afin de couper les affichages (dans lesquels on se noierait) */

	glp_iocp parmip;
	glp_init_iocp(&parmip);
	parmip.msg_lev = GLP_MSG_OFF; /* Paramètre de GLPK dans la résolution d'un PL en variables entières afin de couper les affichages (dans lesquels on se noierait) */
	
	n = 3;

	nbcreux = 2*n*(n*n+1);

	ia = (int *) malloc ((1 + nbcreux) * sizeof(int));
	ja = (int *) malloc ((1 + nbcreux) * sizeof(int));
	ar = (double *) malloc ((1 + nbcreux) * sizeof(double));

	pos = 1;

	for(i=0; i<n; i++)
	{
		for(j=0; j<n*n; j+=n)
		{
			ia[pos] = i + 1;
			ja[pos] = j + i + 1;
			ar[pos] = 1.0;
			pos++;
		}
		ia[pos] = i + 1;
		ja[pos] = n*n + 1;
		ar[pos] = 1.0;
		pos++;
	}

	for(i=n; i<2*n; i++)
	{
		for(j=n*(i-n); j<n*(i-n)+n; j++)
		{
			ia[pos] = i + 1;
			ja[pos] = j + 1;
			ar[pos] = 1.0;
			pos++;
		}
		ia[pos] = i + 1;
		ja[pos] = n*n + 1;
		ar[pos] = 1.0;
		pos++;
	}

	for(i=1; i<pos; i++)
	{
		printf("ia[%d] = %d; ja[%d] = %d; ar[%d] = %f;\n", i, ia[i], i, ja[i], i, ar[i]);
	}

	/* Les appels glp_simplex(prob,NULL); et gpl_intopt(prob,NULL); (correspondant aux paramètres par défaut) seront ensuite remplacés par glp_simplex(prob,&parm); et glp_intopt(prob,&parmip); */

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
	printf("Temps : %f\n",temps);	
	printf("Nombre d'appels à GPLK : %d\n",nbsol);
	printf("Nombre de contraintes ajoutées : %d\n",nbcontr);


	free(ia);
	free(ja);
	free(ar);
}
