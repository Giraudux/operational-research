
## Introduction

## Preuve de l'optimalité de la solution

Soit n le nombre de lieux à visiter, l’ensemble des solutions admissibles est donc l’ensemble des cycles élémentaires de taille n.
Rappelons que les contraintes (3’) cassent les sous-tours de taille 2 à n-1, ces contraintes n’affectent donc pas l’ensemble des solutions admissibles.
De même un sous-ensemble des contraintes (3’) n’affectera pas non plus l’ensemble des solutions admissibles.
La première solution admissible retournée sera donc optimale car on aura seulement supprimé des solutions non admissibles avec un sous-ensemble des contraintes (3’).

## Correspondance des indices

Soit n le nombre de lieux à visiter,
soit i et j ∈ {0,...,n-1}, les doubles indices des variables du problème (indicés à partir de 0),
soit k ∈ {1,...,n×n}, les indices utilisés dans GLPK (indicés à partir de 1),
alors
i = k / n
j = k % n
k = i * n + j

## Récupération de la solution optimale retournée par GLPK et sa traduction en permutations, puis en produit de cycles disjoints

On parcours les valeurs des variables après la résolution et on regarde seulement les valeurs non-nulles qui correspondent aux permutations (voir la fonction extract_permutations).

```c
    for(i = 0; i < nbvar; ++i)
    {
        /* si le coefficient de la variable n'est pas nul */
        if(glp_mip_col_val(prob, i+1) > 0.0)
        {
            /* on calcule les indices des noeuds */
            node = i/n;
            nxt_node = i%n;

            /* mise à jour de la permutation */
            x[node] = nxt_node;
        }
    }
```

On parcours ensuite les cycles disjoints à la volée pour déterminer le plus petit sous-tour (voir les fonctions update_min_sub_loop et is_marked).

```c
    min_sub_loop_len = n;
    nb_marked_node = 0;

    /* tant que l'on a pas marqué tous les noeuds */
    node = 0;
    while(nb_marked_node < n)
    {
        /* si le noeud n'est pas marqué alors on calcule le cycle auquel il appartient */
        if(!is_marked(node, &nb_marked_node))
        {
            /* on initialise le buffer contenant le cycle avec le premier noeud du cycle */
            buf_sub_loop[0] = node;
            sub_loop_len = 1;

            /* tant qu'on ne tombe pas sur un noeud marqué (le premier noeud du cycle), on parcourt le cycle */
            nxt_node = x[node];
            while(!is_marked(nxt_node, &nb_marked_node))
            {
                /* on ajoute le nouveau noeud au cycle */
                buf_sub_loop[sub_loop_len] = nxt_node;
                ++sub_loop_len;

                /* on passe au noeud suivant */
                nxt_node = x[nxt_node];
            }

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
```

## Analyse expérimentale

Fichier       | Temps   | Nombre de contraintes (3') ajoutées
---           | ---     | ---
plat10.dat    | 0.000   | 2
plat20.dat    | 0.024   | 11
plat30.dat    | 0.064   | 14
plat40.dat    | 1.116   | 20
plat50.dat    | 0.288   | 23
plat60.dat    | 4.268   | 27
plat70.dat    | 25.797  | 49
plat80.dat    | 8.052   | 38
plat90.dat    | 9.572   | 41
plat100.dat   | 76.880  | 54
plat110.dat   | 23.505  | 44
plat120.dat   | 90.033  | 59
plat130.dat   | 97.530  | 66
plat140.dat   | 226.482 | 69
plat150.dat   | 342.677 | 71
relief10.dat  | 0.000   | 2
relief20.dat  | 0.000   | 1
relief30.dat  | 0.004   | 0
relief40.dat  | 0.012   | 2
relief50.dat  | 0.364   | 8
relief60.dat  | 0.328   | 8
relief70.dat  | 0.212   | 5
relief80.dat  | 1.156   | 10
relief90.dat  | 0.920   | 6
relief100.dat | 5.700   | 13
relief110.dat | 1.112   | 4
relief120.dat | 3.984   | 10
relief130.dat | 0.492   | 2
relief140.dat | 20.021  | 20
relief150.dat | 2.188   | 10

On observe que le temps de résolution et le nombre de contraintes (3') ajoutées sont biens plus faibles pour les problèmes avec les données en relief.

## Améliorations

Un système de résolution heuristique serait peut-être plus intérressant.

## Conclusion
