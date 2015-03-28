/* GIRAUDET Alexis - HALLEREAU François */

#include <glpk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <time.h>

#define INIT_NBCREUX_AVAILABLE 2

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
    nbsol,
    nbrealloc,
    min_sub_loop_len,
    * min_sub_loop,
    * buf_sub_loop,
    * buf_marked_node,
    * c,
    * x,
    * ia,
    * ja;

double * ar;

glp_prob * prob;

glp_smcp parm;

glp_iocp parmip;

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
    buf_sub_loop = min_sub_loop + n;
    buf_marked_node = buf_sub_loop + n;
    prob = glp_create_prob();
    ia = malloc((1 + nbcreux + INIT_NBCREUX_AVAILABLE) * sizeof(int));
    ja = malloc((1 + nbcreux + INIT_NBCREUX_AVAILABLE) * sizeof(int));
    ar = malloc((1 + nbcreux + INIT_NBCREUX_AVAILABLE) * sizeof(double));

    if((c == NULL) || (prob == NULL) || (ia == NULL) || (ja == NULL) || (ar == NULL))
        exit_error(ERROR_MALLOC);
}

void free_memory(void)
{
    if(prob != NULL)
        glp_delete_prob(prob);
        glp_free_env();

    if(c != NULL)
        free(c);

    if(ia != NULL)
        free(ia);

    if(ja != NULL)
        free(ja);

    if(ar != NULL)
        free(ar);
}

void reallocate_memory(int required)
{
    int tmp;
    static int fib0 = 0,
               fib1 = INIT_NBCREUX_AVAILABLE,
               available = INIT_NBCREUX_AVAILABLE;

/*printf("\nDEBUG required = %d\n", required);
printf("DEBUG fib0 = %d\n", fib0);
printf("DEBUG fib1 = %d\n", fib1);
printf("DEBUG available = %d\n", available);*/

    if(available < required)
    {
        tmp = fib0;
        fib0 = fib1;
        fib1 += tmp;

        if(fib1 < required)
        {
            fib0 += required;
            fib1 += required;
        }

        ia = realloc(ia, (1 + nbcreux + fib1) * sizeof(int));
        ja = realloc(ja, (1 + nbcreux + fib1) * sizeof(int));
        ar = realloc(ar, (1 + nbcreux + fib1) * sizeof(double));

        if((ia == NULL) || (ja == NULL) || (ar == NULL))
            exit_error(ERROR_REALLOC);

        available += fib1;
        ++nbrealloc;
    }

    available -= required;

/*printf("DEBUG\nDEBUG required = %d\n", required);
printf("DEBUG fib0 = %d\n", fib0);
printf("DEBUG fib1 = %d\n", fib1);
printf("DEBUG available = %d\n", available);
printf("DEBUG nbrealloc = %d\n\n", nbrealloc);*/

}

void read_data(char * data_file)
{

    FILE *fin;
    int i, val, res;

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
    for(i=0; i<n*n; ++i)
    {
        res = fscanf(fin,"%d",&val);
        if(res != 1)
            exit_error(ERROR_SCANF);

        c[i] = val;
    }

    /* fermeture du fichier */
    fclose(fin);
}

int is_marked(int id_node, int * nb_marked_node)
{
    int i;

    for(i=0; i<*nb_marked_node; ++i)
    {
        if(buf_marked_node[i] == id_node)
        {
            return 1;
        }
    }

    buf_marked_node[*nb_marked_node] = id_node;
    ++*nb_marked_node;

    return 0;
}

void update_min_sub_loop(void)
{
    int node, nxt_node,
        i,
        sub_loop_len,
        nb_marked_node;

    puts("Cycles:");

    min_sub_loop_len = n;
    nb_marked_node = 0;

    node = 0;
    while(nb_marked_node < n)
    {
        if(!is_marked(node, &nb_marked_node))
        {
            printf("(%d", node+1);

            sub_loop_len = 0;

            buf_sub_loop[sub_loop_len] = node;
            ++sub_loop_len;

            nxt_node = x[node];
            while(!is_marked(nxt_node, &nb_marked_node))
            {
                printf(" %d", nxt_node+1);

                buf_sub_loop[sub_loop_len] = nxt_node;
                ++sub_loop_len;

                nxt_node = x[nxt_node];
            }

            puts(")");

            if((sub_loop_len < min_sub_loop_len) || (sub_loop_len == n))
            {
                min_sub_loop_len = sub_loop_len;
                for(i=0; i<sub_loop_len; ++i)
                {
                    min_sub_loop[i] = buf_sub_loop[i];
                }
            }
        }

        ++node;
    }
    putc('\n', stdout);
}

void extract_permutations(void)
{
    int i, node, nxt_node;

    puts("Permutations:");

    for(i = 0; i < nbvar; ++i)
    {
        if(glp_mip_col_val(prob, i+1) > 0.0)
        {
            node = i/n;
            nxt_node = i%n;

            printf("%d -> %d\n", node+1, nxt_node+1);

            x[node] = nxt_node;
        }
    }

    putc('\n', stdout);
}

void resolve_prob(void)
{
    ++nbsol;
    puts("################");
    printf("Résolution %d\n\n", nbsol);

    glp_load_matrix(prob, nbcreux, ia, ja, ar);

    glp_simplex(prob, &parm);
    glp_intopt(prob, &parmip);

    extract_permutations();

    update_min_sub_loop();
}

int main(int argc, char *argv[])
{
    int i,
        j,
        pos;

    double temps,
           z;

    c = x = ia = ja = NULL;
    ar = NULL;
    prob = NULL;
    min_sub_loop_len = nbsol = nbrealloc = 0;

    atexit(free_memory);

    /* test de l'argument */
    if(argc != 2)
        exit_error(ERROR_ARGC);

    read_data(argv[1]);

    crono_start(); /* Lancement du compteur (juste après le chargement des données à partir d'un fichier */

    glp_set_prob_name(prob, "robots"); /* affectation d'un nom */
    glp_set_obj_dir(prob, GLP_MIN); /* Il s'agit d'un problème de minimisation */

    glp_init_smcp(&parm);
    parm.msg_lev = GLP_MSG_OFF;

    glp_init_iocp(&parmip);
    parmip.msg_lev = GLP_MSG_OFF;

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
        glp_set_row_bnds(prob, nbcontr, GLP_UP, min_sub_loop_len-1.0, min_sub_loop_len-1.0);

        nbcreux += min_sub_loop_len;
        /*ia = realloc(ia, (1 + nbcreux) * sizeof(int));
        ja = realloc(ja, (1 + nbcreux) * sizeof(int));
        ar = realloc(ar, (1 + nbcreux) * sizeof(double));
        ++nbrealloc;*/
        reallocate_memory(min_sub_loop_len);

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

    printf("z = %g\n\n", z);

    /* Résolution achevée, arrêt du compteur de temps et affichage des résultats */
    crono_stop();
    temps = crono_ms()/1000,0;

    puts("Résultat :");
    puts("-----------");
    /* Affichage de la solution sous la forme d'un cycle avec sa longueur à ajouter */
    printf("Temps : %f\n", temps);
    printf("Nombre d'appels à GPLK : %d\n", nbsol);
    printf("Nombre d'appels à realloc : %d (*3)\n", nbrealloc);
    printf("Nombre de contraintes ajoutées : %d\n", nbcontr);

    glp_write_lp(prob, NULL, "robots.lp");

    return EXIT_SUCCESS;
}
