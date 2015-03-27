/* GIRAUDET Alexis - HALLEREAU François */

#include <glpk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <time.h>

struct timeval start_utime, stop_utime;

enum error_code
{
    ERROR_ARGC,
    ERROR_FOPEN,
    ERROR_MALLOC,
    ERROR_SCANF,
    ERROR_REALLOC
};

int n,
    nbcreux,
    nbcontr,
    nbvar,
    min_sub_loop_len,
    * min_sub_loop,
    * c,
    * x,
    * ia,
    * ja;

double * ar;

glp_prob * prob;

void crono_start(void)
{
    struct rusage rusage;

    getrusage(RUSAGE_SELF, &rusage);
    start_utime = rusage.ru_utime;
}

void crono_stop(void)
{
    struct rusage rusage;

    getrusage(RUSAGE_SELF, &rusage);
    stop_utime = rusage.ru_utime;
}

double crono_ms(void)
{
    return (stop_utime.tv_sec - start_utime.tv_sec) * 1000 +
           (stop_utime.tv_usec - start_utime.tv_usec) / 1000 ;
}

void exit_error(enum error_code err)
{
    switch(err)
    {
    case ERROR_ARGC:
        fputs("Usage: robots [DATA FILE]\n", stderr);
        break;
    case ERROR_MALLOC:
        fputs("Error: malloc\n", stderr);
        break;
    case ERROR_FOPEN:
        fputs("Error: fopen\n", stderr);
        break;
    case ERROR_SCANF:
        fputs("Error: scanf\n", stderr);
        break;
    case ERROR_REALLOC:
        fputs("Error: realloc\n", stderr);
        break;
    }

    exit(EXIT_FAILURE);
}

void allocate_memory(void)
{
    c = malloc((n * n + n + 3 * n) * sizeof(int));
    x = c + n * n;
    min_sub_loop = x + n;
    prob = glp_create_prob();
    ia = malloc((1 + nbcreux) * sizeof(int));
    ja = malloc((1 + nbcreux) * sizeof(int));
    ar = malloc((1 + nbcreux) * sizeof(double));

    if((c == NULL) || (prob == NULL) || (ia == NULL) || (ja == NULL) || (ar == NULL))
        exit_error(ERROR_MALLOC);
}

void free_memory(void)
{
    if(prob != NULL)
        glp_delete_prob(prob);

    if(c != NULL)
        free(c);

    if(ia != NULL)
        free(ia);

    if(ja != NULL)
        free(ja);

    if(ar != NULL)
        free(ar);
}

void reallocate_memory(void)
{
    static int fib0 = 0,
               fib1 = 2;
}

void read_data(char * data_file)
{

    FILE *fin; /* fichier à lire */
    int i,j; /* indices */
    int val, res; /* entier lu */

    fin = fopen(data_file, "r"); /* ouverture du fichier en lecture */
    if(fin == NULL)
        exit_error(ERROR_FOPEN);

    /* lecture et initialisation de n */
    res = fscanf(fin, "%d", &val);
    if(res != 1)
        exit_error(ERROR_SCANF);

    n = val;
    nbcreux = 2*n*n - 2*n;
    nbcontr = 2*n;
    nbvar = n*n;

    allocate_memory();

    /* allocation et remplissage de la matrice des distances */
    for(i=0; i<n; ++i)
    {
        for(j=0; j<n; ++j)
        {
            res = fscanf(fin,"%d",&val);
            if(res != 1)
                exit_error(ERROR_SCANF);

            c[i*n+j] = val;
        }
    }

    /* fermeture du fichier */
    fclose(fin);
}

int is_marked(int id_node, int * ar_marked_node, int * nb_marked_node)
{
    int i;

    for(i=0; i<*nb_marked_node; ++i)
    {
        if(ar_marked_node[i] == id_node)
        {
            return 1;
        }
    }

    ar_marked_node[*nb_marked_node] = id_node;
    ++*nb_marked_node;

    return 0;
}

void update_min_sub_loop(void)
{
    int node, nxt_node,
        i, j,
        * tmp_sub_loop,
        tmp_sub_loop_len,
        * ar_marked_node,
        nb_marked_node;

    tmp_sub_loop = min_sub_loop + n;
    ar_marked_node = tmp_sub_loop + n;

    min_sub_loop_len = n;
    nb_marked_node = 0;

    node = 0;
    while(nb_marked_node < n)
    {
        if(!is_marked(node, ar_marked_node, &nb_marked_node))
        {
            tmp_sub_loop_len = 0;

            tmp_sub_loop[tmp_sub_loop_len] = node;
            ++tmp_sub_loop_len;

            nxt_node = x[node];
            while(!is_marked(nxt_node, ar_marked_node, &nb_marked_node))
            {
                tmp_sub_loop[tmp_sub_loop_len] = nxt_node;
                ++tmp_sub_loop_len;

                nxt_node = x[nxt_node];
            }

            if((tmp_sub_loop_len < min_sub_loop_len) || (tmp_sub_loop_len == n))
            {
                min_sub_loop_len = tmp_sub_loop_len;
                for(j=0; j<tmp_sub_loop_len; ++j)
                {
                    min_sub_loop[j] = tmp_sub_loop[j];
                }
            }
        }

        ++node;
    }
}

void extract_permutations(void)
{
    int i;

    for(i = 0; i < nbvar; ++i)
    {
        if(glp_mip_col_val(prob, i+1) > 0.0)
        {
            x[i/n] = i%n;
        }
    }
}

void resolve_prob(void)
{
    glp_load_matrix(prob, nbcreux, ia, ja, ar);

    glp_write_lp(prob, NULL, "robots.lp");

    glp_simplex(prob, NULL);
    glp_intopt(prob, NULL);

    extract_permutations();

    update_min_sub_loop();
}

int main(int argc, char *argv[])
{
    int i,
        j,
        pos,
        nbsol;

    double temps,
           z;

    c = x = ia = ja = NULL;
    ar = NULL;
    prob = NULL;
    min_sub_loop_len = 0;
    nbsol = 0;

    atexit(free_memory);

    /* test de l'argument */
    if(argc != 2)
        exit_error(ERROR_ARGC);

    read_data(argv[1]);

    crono_start(); /* Lancement du compteur (juste après le chargement des données à partir d'un fichier */

    glp_set_prob_name(prob, "robots"); /* affectation d'un nom */
    glp_set_obj_dir(prob, GLP_MIN); /* Il s'agit d'un problème de minimisation */

    glp_add_rows(prob, nbcontr);
    for(i=1; i<=nbcontr; ++i)
    {
        glp_set_row_bnds(prob, i, GLP_FX, 1.0, 1.0);
    }

    glp_add_cols(prob, nbvar);
    for(i=1; i<=nbvar; ++i)
    {
        glp_set_col_bnds(prob, i, GLP_DB, 0.0, 1.0);
        glp_set_col_kind(prob, i, GLP_BV);
    }

    pos = 1;

    for(i=0; i<n; ++i)
    {
        for(j=0; j<n; ++j)
        {
            glp_set_obj_coef(prob, pos, c[i*n+j]);
            ++pos;
        }
    }

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
                ++pos;

                ia[pos] = i + n + 1;
                ja[pos] = j * n + 1 + i;
                ar[pos] = 1.0;
                ++pos;
            }
        }
    }

    resolve_prob();

    while(min_sub_loop_len < n)
    {
        nbcontr = glp_add_rows(prob, 1);
        glp_set_row_bnds(prob, nbcontr, GLP_UP, 1.0, 1.0);

        nbcreux += min_sub_loop_len;

        ia = realloc(ia, (1 + nbcreux) * sizeof(int));
        ja = realloc(ja, (1 + nbcreux) * sizeof(int));
        ar = realloc(ar, (1 + nbcreux) * sizeof(double));

        for(i=0; i<min_sub_loop_len; ++i)
        {
            ia[pos] = nbcontr;
            ja[pos] = min_sub_loop[i] * n + min_sub_loop[(i+1)%min_sub_loop_len] + 1;
            ar[pos] = 1.0;
            ++pos;
        }

        resolve_prob();
    }

    z = glp_mip_obj_val(prob);

    printf("z = %lf\n",z);
    for(i=0; i<n; ++i)
    {
        printf("%d -> %d\n", i+1, x[i]+1);
    }

    printf("min = %d\n", j);
    for(i=0; i<j; ++i)
    {
        printf("%d ", min_sub_loop[i]+1);
    }
    puts("\n");

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

    return EXIT_SUCCESS;
}
