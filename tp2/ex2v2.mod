/*
 * Recherche Opérationnelle
 * TP2 : GNU MathProg - Utilisation d'une matrice creuse
 *
 * Exercice 2
 * (TD 2 Exercice 6)
 * Modèle (avec matrice creuse)
 *
 * Alexis Giraudet
 * François Hallereau
 * 602C
 */

# Declaration des donnees du probleme

    param nbClients; # nombre de clients
    param nbEntrepots; # nombre d'entrepots

    set indClient := 1..nbClients; # indices des clients
    set indEntrepot := 1..nbEntrepots; # indices des entrepots

    set indEntrepotClient within indEntrepot cross indClient; # ensemble des indices de la matrice creuse

    param tabCout{indEntrepot}; # tableau des couts
    param tabCapacite{indEntrepot}; # tableau des capacites
    param tabDemande{indClient}; # tableau des demandes

    param matCoutLivr{(i,j) in indEntrepotClient}; # Matrice creuse des couts par livraison



# Declaration d'un tableau de variables binaires

    var y{indEntrepot} binary; # 1 si on construit un entrepot sur le site i, 0 sinon
    var x{indEntrepot, indClient} >= 0 integer; # part de satisfaction de la demande des clients par l'entrepot

# Fonction objectif

    minimize cout : sum{k in indEntrepot}( y[k]*tabCout[k] + sum{(i,j) in indEntrepotClient}( x[i,j]*matCoutLivr[i,j]));

# Contraintes

    s.t. st1{j in indClient} : sum{i in indEntrepot} x[i,j] == 1;
    s.t. st2{i in indEntrepot} : sum{j in indClient} tabDemande[j]*x[i,j] <= tabCapacite[i]*y[i];

# Resolution (qui est ajoutee en fin de fichier si on ne le precise pas)

    solve;

# Affichage des resultats

    #display : x;
    display : y;
    display : "objectif : ", (sum{i in indEntrepot} y[i]*tabCout[i]) + (sum{(i,j) in indEntrepotClient} x[i,j]*matCoutLivr[i,j]);

end;
