/* GIRAUDET Alexis - HALLEREAU François */

#include <glpk.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <time.h>

/* Constantes pour l'affichage des noms des contraintes du problème */
const char * const ct1 = "(1')";
const char * const ct2 = "(2')";
const char * const ct3 = "(3)";
/* Suffixe du nom du fichier de sortie du problème */
const char * const out_file_suffix = ".lp";

struct timeval start_utime, stop_utime;

/* Énumération des codes d'erreur */
enum error_code
{
    ERROR_ARGC, /* argument invalide */
    ERROR_FOPEN, /* fichier invalide */
    ERROR_MALLOC, /* problème d'allocation de mémoire */
    ERROR_SCANF, /* problème de formatage du fichier */
    ERROR_REALLOC /* problème de réallocation de mémoire */
};

char * argv0, /* nom du fichier du programme exécuté */
     * argv1, /* premier argument passé au programme */
     * var_name, /* noms des variables du problème, de la forme: x_i,j */
     * out_file; /* nom de fichier de sortie du problème */

/* taille maximale du nom des variables du problème */
size_t var_size;

int n, /* nombre de lignes/colonnes du distancier */
    nbcreux, /* nombre d'éléments de la matrice creuse */
    nbcreux_available, /* nombre d'éléments de la matrice creuse disponibles */
    nbcontr, /* nombre de contraintes */
    nbvar, /* nombre de variables (x_i,j) */
    nbsol, /* nombre d'appels à glpk */
    nbrealloc, /* nombre d'appels à la fonction realloc (/3) */
    min_sub_loop_len, /* taille du plus petit sous-tour */
    fib0, /* Fibonacci(n-1): utilisé pour calculer nbcreux_available */
    fib1, /* Fibonacci(n): utilisé pour calculer nbcreux_available */
    * min_sub_loop, /* plus petit sous-tour */
    * buf_sub_loop, /* buffer utilisé pour le calcul du plus petit sous-tour */
    * buf_marked_node, /* buffer utilisé pour le calcul du plus petit sous-tour */
    * c, /* distancier */
    * x, /* tableau des permutations */
    * ia, /* indices des contraintes de la matrice creuse */
    * ja; /* indices des variables de la matrice creuse */

/* coefficients des variables de la matrice creuse */
double * ar;

/* problème glpk */
glp_prob * prob;

/* paramètres à passer au problème glpk pour une résolution silencieuse */
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

/* Affichage d'un message d'erreur et terminaison du programme en cas d'erreur */
void exit_error(enum error_code err)
{
    switch(err)
    {
    case ERROR_ARGC:
        fprintf(stderr, "Utilisation: %s [FICHIER .DAT]\n", argv0);
        break;
    case ERROR_MALLOC:
        fputs("Erreur: malloc\n", stderr);
        break;
    case ERROR_FOPEN:
        fputs("Erreur: Impossibe d'accéder au fichcier\n", stderr);
        break;
    case ERROR_SCANF:
        fputs("Erreur: Format de fichier incorrect\n", stderr);
        break;
    case ERROR_REALLOC:
        fputs("Erreur: realloc\n", stderr);
        break;
    }

    exit(EXIT_FAILURE);
}

/* Allocation de la mémoire */
void allocate_memory(void)
{
    c = malloc((n * n + n + 3 * n) * sizeof(int));
    x = c + n * n;
    min_sub_loop = x + n;
    buf_sub_loop = min_sub_loop + n;
    buf_marked_node = buf_sub_loop + n;
    var_name = malloc((n * n * var_size + strlen(argv1) + strlen(out_file_suffix) + 1) * sizeof(char));
    out_file = var_name + n * n * var_size;
    prob = glp_create_prob();
    ia = malloc((1 + nbcreux + nbcreux_available) * sizeof(int));
    ja = malloc((1 + nbcreux + nbcreux_available) * sizeof(int));
    ar = malloc((1 + nbcreux + nbcreux_available) * sizeof(double));

    /* vérification des allocations */
    if((c == NULL) || (prob == NULL) || (ia == NULL) || (ja == NULL) || (ar == NULL) || (var_name == NULL))
        exit_error(ERROR_MALLOC);
}

/* libération de la mémoire */
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

    if(var_name != NULL)
        free(var_name);
}

/* réallocation automatique de la mémoire pour la matrice creuse
 * nbcreux_required: taille nécessaire (taille minimale à allouer)
 */
void reallocate_memory(int nbcreux_required)
{
    int tmp;

    /* teste si on a besoin de réallouer la mémoire */
    if(nbcreux_available < nbcreux_required)
    {
        /* incrémentation de la suite de Fibonacci */
        tmp = fib0;
        fib0 = fib1;
        fib1 += tmp;

        /* teste si la nouvelle valeur de la suite de Fibonacci répond aux besoins */
        if(fib1 < nbcreux_required)
        {
            /* gonflement artificiel des valeurs de la suite */
            fib0 += nbcreux_required;
            fib1 += nbcreux_required;
        }

        /* réallocation */
        ia = realloc(ia, (1 + nbcreux + fib1) * sizeof(int));
        ja = realloc(ja, (1 + nbcreux + fib1) * sizeof(int));
        ar = realloc(ar, (1 + nbcreux + fib1) * sizeof(double));

        /* teste la réallocation */
        if((ia == NULL) || (ja == NULL) || (ar == NULL))
            exit_error(ERROR_REALLOC);

        /* calcul du nouveau nombre d'éléments de la matrice creuse disponibles */
        nbcreux_available += fib1;
        ++nbrealloc;
    }

    /* calcul du nouveau nombre d'éléments de la matrice creuse disponibles */
    nbcreux_available -= nbcreux_required;
}

/* lecture du distancier depuis un fichier
 * data_file: nom du fichier du distancier
 */
void read_data(char * data_file)
{

    FILE *fin; /* fichier du distancier */
    int i, /* indice */
        val, /* valeur lue (scanf) */
        res; /* résultat de la lecture (scanf) */

    fin = fopen(data_file, "r"); /* ouverture du fichier en lecture */
    
    /* teste l'ouverture du fichier */
    if(fin == NULL)
        exit_error(ERROR_FOPEN);

    /* lecture et initialisation de n */
    res = fscanf(fin, "%d", &val);
    if(res != 1)
        exit_error(ERROR_SCANF);

    /*@GLOBAL_INIT*/
    n = val;
    nbcreux = 2*n*n - 2*n;
    nbcontr = 2*n;
    nbvar = n*n;
    nbcreux_available = fib1 = n; /* marge de n*/
    var_size = (2 * (log10(n) + 1) + 4); /* 2*: deux indice, 4: 'x' + '_' + ',' + '\0' */

    /* allocation de la mémoire */
    allocate_memory();

    /* lecture des valeurs du distancier */
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

/* teste si un noeud est marqué et le marque si il ne l'est pas
 * id_node: le noeud à tester
 * nb_marked_node: le nombre de noeuds déjà marqués
 * retourne vrai si le noeud est déjà marqué sinon faux
 */
int is_marked(int id_node, int * nb_marked_node)
{
    int i; /* indice */

    /* parcours des noeuds marqués */
    for(i=0; i<*nb_marked_node; ++i)
    {
        /* teste si le noeud est marqué */
        if(buf_marked_node[i] == id_node)
        {
            return 1;
        }
    }

    /* ajoute le noeud au tableau des noeuds marqués */
    buf_marked_node[*nb_marked_node] = id_node;
    /* et incrémente le nombre de noeuds marqués */
    ++*nb_marked_node;

    return 0;
}

/* met à jour le plus petit sous-tour à partir du tableau des permutations */
void update_min_sub_loop(void)
{
    int node, nxt_node, /* noeud -> noeud suivant */
        i, /* indice */
        sub_loop_len, /* taille du sous-tour temporaire */
        nb_marked_node; /* nombre de noeuds marqués */

    /*@PRINT*/
    puts("Cycles:");

    min_sub_loop_len = n;
    nb_marked_node = 0;

    /* tant que l'on a pas marqué tous les noeuds */
    node = 0;
    while(nb_marked_node < n)
    {
        /* si le noeud n'est pas marqué alors on calcule le cycle auquel il appartient */
        if(!is_marked(node, &nb_marked_node))
        {
            /*@PRINT*/
            printf("(%d", node+1);

            /* on initialise le buffer contenant le cycle avec le premier noeud du cycle */
            buf_sub_loop[0] = node;
            sub_loop_len = 1;

            /* tant qu'on ne tombe pas sur un noeud marqué (le premier noeud du cycle), on parcourt le cycle */
            nxt_node = x[node];
            while(!is_marked(nxt_node, &nb_marked_node))
            {
                /*@PRINT*/
                printf(" %d", nxt_node+1);

                /* on ajoute le nouveau noeud au cycle */
                buf_sub_loop[sub_loop_len] = nxt_node;
                ++sub_loop_len;

                /* on passe au noeud suivant */
                nxt_node = x[nxt_node];
            }

            /*@PRINT*/
            puts(")");

            /* si le nouceau cycle est plus petit que l'ancient ou bien qu'il n'y à pas de sous-tour */
            if((sub_loop_len < min_sub_loop_len) || (sub_loop_len == n))
            {
                /* on met à jour le plus petit sous-tour ou bien la solution */
                min_sub_loop_len = sub_loop_len;
                for(i=0; i<sub_loop_len; ++i)
                {
                    min_sub_loop[i] = buf_sub_loop[i];
                }
            }
        }

        /* on passe au noeud suivant */
        ++node;
    }

    /*@PRINT*/
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

void write_prob(void)
{
    int i;
    char * current_index;

    for(i=0; i<n; ++i)
    {
        glp_set_row_name(prob, i+1, ct1);
        glp_set_row_name(prob, i+n+1, ct2);
    }

    for(i=2*n; i<nbcontr; ++i)
        glp_set_row_name(prob, i+1, ct3);

    i = 0;
    for(current_index=var_name; current_index<var_name+n*n*var_size; current_index+=var_size)
    {
        sprintf(current_index, /*var_size,*/ "x_%d,%d", i/n+1, i%n+1);
        glp_set_col_name(prob, i+1, current_index);
        ++i;
    }

    strcpy(out_file, argv1);
    strcpy(out_file+strlen(argv1), out_file_suffix);

    glp_write_lp(prob, NULL, out_file);
}

int main(int argc, char *argv[])
{
    int i,
        j,
        pos;

    double temps,
           z;

    argv0 = argv[0];
    argv1 = var_name = out_file = NULL;
    c = x = ia = ja = NULL;
    ar = NULL;
    prob = NULL;
    min_sub_loop_len = nbsol = nbrealloc = fib0 = 0;

    atexit(free_memory);

    /* test de l'argument */
    if(argc != 2)
        exit_error(ERROR_ARGC);

    argv1 = argv[1];

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
        reallocate_memory(min_sub_loop_len);

        puts("Cycle à casser:");
        printf("(%d", min_sub_loop[0] + 1);

        ia[pos] = nbcontr;
        ja[pos] = min_sub_loop[min_sub_loop_len-1] * n + min_sub_loop[0] + 1;
        ar[pos] = 1.0;
        ++pos;

        for(i=0; i<min_sub_loop_len-1; ++i)
        {
            printf(" %d", min_sub_loop[i+1]+1);

            ia[pos] = nbcontr;
            ja[pos] = min_sub_loop[i] * n + min_sub_loop[i+1] + 1;
            ar[pos] = 1.0;
            ++pos;
        }

        puts(")\n");

        resolve_prob();
    }

    z = glp_mip_obj_val(prob);

    printf("z = %g\n\n", z);

    /* Résolution achevée, arrêt du compteur de temps et affichage des résultats */
    crono_stop();
    temps = crono_ms()/1000.0;

    puts("Résultat :");
    puts("-----------");
    /* Affichage de la solution sous la forme d'un cycle avec sa longueur à ajouter */
    printf("Temps (secondes) : %.3lf\n", temps);
    printf("Nombre d'appels à GPLK : %d\n", nbsol);
    printf("Nombre d'appels à realloc (/3) : %d\n", nbrealloc);
    printf("Nombre de contraintes ajoutées : %d\n", nbcontr);

    write_prob();

    return EXIT_SUCCESS;
}
