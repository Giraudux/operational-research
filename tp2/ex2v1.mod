/*
 * Recherche Opérationnelle
 * TP2 : GNU MathProg - Utilisation d'une matrice creuse
 *
 * Exercice 2
 * (TD 2 Exercice 6)
 * Modèle
 *
 * Alexis Giraudet
 * François Hallereau
 * 602C
 */

# Declaration des donnees du probleme

    param nbClients; # nombre de contraintes (lignes) du probleme
    param nbEntrepots; # nombre de variables (colonnes) du probleme

    set indClient := 1..nbClients; # indices des contraintes
    set indEntrepot := 1..nbEntrepots; # indices des variables

    param tabCout{indEntrepot};
    param tabCapacite{indEntrepot};
    param tabDemande{indClient};

    param matCoutLivr{indEntrepot,indClient}; # Matrice des contraintes



# Declaration d'un tableau de variables binaires

    var y{indEntrepot} binary;
    var x{indEntrepot, indClient} >= 0;

# Fonction objectif

    minimize cout : sum{i in indEntrepot}(y[i]*tabCout[i] + sum{j in indClient}(x[i,j]*matCoutLivr[i,j]));

# Contraintes

    s.t. st1{j in indClient} : sum{i in indEntrepot} x[i,j] == 1;
    s.t. st2{i in indEntrepot} : sum{j in indClient} tabDemande[j]*x[i,j] <= tabCapacite[i]*y[i];

# Resolution (qui est ajoutee en fin de fichier si on ne le precise pas)

    solve;

# Affichage des resultats

    #display : x;
    display : y;
    display : "objectif : ", sum{i in indEntrepot}( y[i]*tabCout[i] + sum{j in indClient}( x[i,j]*matCoutLivr[i,j]));

end;
