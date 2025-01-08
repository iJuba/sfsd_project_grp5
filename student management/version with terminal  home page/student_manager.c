#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ENREGISTREMENTS 10  /// Nombre d'enregistrements par bloc
#define MAX_LINE_LENGTH 256     /// Longueur maximale d'une ligne dans le fichier texte

/// Structure d'un enregistrement
typedef struct {
    int numero_inscription;     /// Num�ro d'inscription de l'�tudiant
    char nom[50];               /// Nom de l'�tudiant
    char prenom[50];            /// Pr�nom de l'�tudiant
    int annee_naissance;        /// Ann�e de naissance de l'�tudiant
    char groupe[10];            /// Groupe de l'�tudiant
    float notes[4];             /// Notes de l'�tudiant
    int coefficients[4];        /// Coefficients des notes
    float moyenne_ponderee;     /// Moyenne pond�r�e des notes
    int indicateur_suppression; /// Indicateur de suppression (0 = actif, 1 = supprim�)
} Etudiant;

/// Structure d'un bloc
typedef struct {
    Etudiant etudiants[MAX_ENREGISTREMENTS]; /// Tableau d'�tudiants
    int nb_enregistrements;                  /// Nombre d'enregistrements dans ce bloc
} Bloc;

/// Structure de l'ent�te
typedef struct {
    int nb_blocs_utilises;    /// Nombre total de blocs utilis�s
    int compteur_inserts;     /// Nombre total d'enregistrements ins�r�s
} Entete;

/// Structure repr�sentant le fichier TOF avec son ent�te
typedef struct {
    FILE *fichier;   /// Pointeur vers le fichier
    Entete entete;   /// Ent�te du fichier
} FichierTOF;

/// Fonction pour lire l'ent�te du fichier
void lire_entete(FichierTOF *f) {
    rewind(f->fichier); /// Retour au d�but du fichier
    fread(&(f->entete), sizeof(Entete), 1, f->fichier); /// Lecture de l'ent�te
}

/// Fonction pour mettre � jour l'ent�te du fichier
void mettre_a_jour_entete(FichierTOF *f) {
    rewind(f->fichier); /// Retour au d�but du fichier
    fwrite(&(f->entete), sizeof(Entete), 1, f->fichier); /// �criture de l'ent�te
}

/// Fonction pour initialiser le fichier TOF
void initialiser_fichier(FichierTOF *f, const char *nom_fichier) {
    f->fichier = fopen(nom_fichier, "wb"); /// Cr�ation et ouverture du fichier en mode �criture binaire
    if (f->fichier == NULL) {
        printf("Erreur lors de l'ouverture du fichier");
        exit(-1);
    }

    /// Initialiser l'ent�te
    f->entete.nb_blocs_utilises = 0;
    f->entete.compteur_inserts = 0;

    /// �crire l'ent�te dans le fichier
    fwrite(&(f->entete), sizeof(Entete), 1, f->fichier);
}

/** la fonction lire_ligne permet de recuper� les information contenue par une ligne
et les rang� dans une strecture Etudiant d�finie au paravant */
int lire_ligne(char *ligne, Etudiant *etudiant) {
    char *info;
    int i;

    /// Lire le num�ro d'inscription
    info = strtok(ligne, ";");
    etudiant->numero_inscription = atoi(info);

    /** strtok divise la cha�ne ligne en jetons s�par�s par des points-virgules (;)
     La fonction atoi convertit une cha�ne de caract�res en un entier */

    /// Lire le nom et le prenom
    info = strtok(NULL, ";");
    sscanf(info, "%49s %49s", etudiant->nom, etudiant->prenom);

    /** Chaque appel suivant � strtok avec NULL comme premier argument continue � diviser la cha�ne initiale
     La fonction sscanf lit des donn�es format�es � partir d'une cha�ne de caract�res
     le format uttilis� "%s %s" car le nom et le prenom sont s�par� par un espace */

    /// Lire l'ann�e de naissance
    info = strtok(NULL, ";");
    etudiant->annee_naissance = atoi(info);

    /// Lire le groupe
    info = strtok(NULL, ";");
    strcpy(etudiant->groupe, info);

    /// Lire les notes et coefficients
    for (i = 0; i < 4; i++) {
        info = strtok(NULL, ";");
        sscanf(info, "%f,%d", &etudiant->notes[i], &etudiant->coefficients[i]);
    }

    //// Lire la moyenne pond�r�e
    info = strtok(NULL, ";");
    etudiant->moyenne_ponderee = atof(info);

    /// La fonction atof convertit une cha�ne de caract�res en un nombre � virgule flottante (float)

    /// Lire l'indicateur de suppression
    info = strtok(NULL, ";");
    etudiant->indicateur_suppression = atoi(info);
    return 0;
}

/// Fonction pour charger initialement les donn�es depuis un fichier texte vers un fichier binaire
int chargement_initial(const char *fichier_txt, const char *fichier_bin, float T) {
    /// T est un r�el compris entre 0 et 1 et d�signe le taux de chargement voulu au d�part
    FILE *F1;
    F1 = fopen(fichier_txt, "r");
    if (F1 == NULL) {
        printf("Erreur d'ouverture du ficher txt");
        return -9999;
    }

    FichierTOF *F2 = (FichierTOF *)malloc(sizeof(FichierTOF));/// Allocation d'espace pour F2
    initialiser_fichier(F2, fichier_bin);

    Etudiant etudiant;/// etudiant c'est le buffer utilis� pour remplire les bloc
    Bloc Buf;/// Buf est le buffer utilis� pour ecrire sur /lire du fichier binaire

    int j = 0;  /// num d'enreg dans le bloc
    int i = 1;  /// pour calculer le nombre de bloc
    int k = 0;  /// pour calculer le nombre d'enregistrement

    char ligne[MAX_LINE_LENGTH];
    while (fgets(ligne, sizeof(ligne), F1) != NULL) {
        lire_ligne(ligne, &etudiant);
        k++;
        if (j < T * MAX_ENREGISTREMENTS) {///verifier si le bloc a remplire n'est pas plein a (T*100)%
            Buf.etudiants[j] = etudiant;
            j++;
        } else {/// le bloc est plein a (T*100)%
            Buf.nb_enregistrements = j;
            fwrite(&Buf, sizeof(Bloc), 1, F2->fichier);/// Ecriture du bloc dans le fichier
            Buf.etudiants[0] = etudiant; /// remplissage d'un nouveau bloc avec l'element lus pour ne pas le perdre
            i++;
            j = 1;
        }
    }
    Buf.nb_enregistrements = j;
    fwrite(&Buf, sizeof(Bloc), 1, F2->fichier);/// ecriture du dernier bloc
    F2->entete.nb_blocs_utilises = i;
    F2->entete.compteur_inserts = k;
    mettre_a_jour_entete(F2);
    fclose(F1);
    fclose(F2->fichier);
    free(F2);/// liberer l'espace deja allou�
    return 0;
}

/// Fonction pour r�organiser le fichier binaire en supprimant les enregistrements marqu�s comme supprim�s
int Reorganisation(const char *fichier_bin, float T) {
    FichierTOF *F = (FichierTOF *)malloc(sizeof(FichierTOF));
    F->fichier = fopen(fichier_bin, "rb");
    if (F == NULL) {
        printf("Erreur d'ouverture du ficher %s", fichier_bin);
        return -9999;
    }
    lire_entete(F);/// la lecture de l'entete nous permet de positionn� la tete de lecture/ecriture sur le premier bloc
    FichierTOF *F1 = (FichierTOF *)malloc(sizeof(FichierTOF));
    initialiser_fichier(F1, "new_file.bin");

    Bloc Buf, Buf1; /// Buf represente le buffer du fichier initial __ Buf1 represente le buffer du nouveau fichier reorganis�
    int nb_enr = 0; /// pour calculer le nombre d'enregistrement dans le fichier
    int nb_blc = 1; /// pour calculer le nombre de bloc dans le fichier
    int j;          /// pour mettre le nombre d'enregistrement dans le bloc
    int k;          /// indice pour parcourire un bloc d'enregistremment dans le fichier initial
    int q = 0;      /// indice pour parcourire un bloc d'enregistremment dans le fichier reorganis�

    while (fread(&Buf, sizeof(Buf), 1, F->fichier) == 1) { /// lire bloc par bloc
        j = Buf.nb_enregistrements;
        k = 0;
        while (k < j) { /// tanque tous les enregistremment du bloc ne sont pas entierement lus
            if (Buf.etudiants[k].indicateur_suppression == 0) {///verifier l'indicateur de supression pour savoir si l'etudiant est actif
                nb_enr++;
                ///Remlissage des bloc du fichier reorganis� � (T*100)%
                if (q < T * MAX_ENREGISTREMENTS) {
                    Buf1.etudiants[q] = Buf.etudiants[k];
                    q++;
                } else {
                    Buf1.nb_enregistrements = q;
                    fwrite(&Buf1, sizeof(Bloc), 1, F1->fichier);
                    Buf1.etudiants[0] = Buf.etudiants[k];
                    q = 1;
                    nb_blc++;
                }
            }
            k++; /// passer au prochain enregistrement
        }
    }
    /** A la fin de la boucle on doit �crire le dernier bloc
     m�me si tous les enregistrements sont supprim�s le fichier contiendrait un bloc vide */
    Buf1.nb_enregistrements = q;
    fwrite(&Buf1, sizeof(Bloc), 1, F1->fichier);

    F1->entete.compteur_inserts = nb_enr;
    F1->entete.nb_blocs_utilises = nb_blc;
    mettre_a_jour_entete(F1);

    fclose(F->fichier);
    fclose(F1->fichier);
    free(F);
    free(F1);
    /// Remplacer l'ancient fichier par le nouveau reorganis�
    remove(fichier_bin); /// Supprimer l'ancien fichier binaire
    rename("new_file.bin", fichier_bin); /// Renommer le nouveau fichier binaire

    return 0;
}

/// Fonction de recherche dichotomique conforme � la logique fournie
int recherche_dichotomique(const char *fichier_bin, int numero_inscription, int *Trouv, int *i, int *j) {
    FILE *fichier = fopen(fichier_bin, "rb");
    if (fichier == NULL) {
        printf("Erreur lors de l'ouverture du fichier\n");
        return -1;
    }

    /// Lire l'ent�te pour conna�tre le nombre de blocs utilis�s
    Entete entete;
    fread(&entete, sizeof(Entete), 1, fichier);

    /// Initialisation des variables
    int bi = 1; /// Borne inf�rieure (premier bloc)
    int bs = entete.nb_blocs_utilises; /// Borne sup�rieure (dernier bloc)
    *Trouv = 0; /// Indicateur de trouv� (faux par d�faut)
    int stop = 0; /// Indicateur d'arr�t
    *j = 1; /// Position initiale dans le bloc

    /// Recherche externe (entre les blocs)
    while (bi <= bs && !(*Trouv) && !stop) {
        *i = (bi + bs) / 2; /// Bloc du milieu
        fseek(fichier, sizeof(Entete) + (*i - 1) * sizeof(Bloc), SEEK_SET); /// Se positionner sur le bloc *i
        Bloc buf;
        fread(&buf, sizeof(Bloc), 1, fichier); /// Lire le bloc

        /// V�rifier si la cl� est dans l'intervalle du bloc courant
        if (numero_inscription >= buf.etudiants[0].numero_inscription &&
            numero_inscription <= buf.etudiants[buf.nb_enregistrements - 1].numero_inscription) {
            /// Recherche interne (dans le bloc)
            int inf = 0; /// Borne inf�rieure dans le bloc
            int sup = buf.nb_enregistrements - 1; /// Borne sup�rieure dans le bloc
            while (inf <= sup && !(*Trouv)) {
                *j = (inf + sup) / 2; /// Position du milieu dans le bloc
                if (numero_inscription == buf.etudiants[*j].numero_inscription) {
                    *Trouv = 1; /// �tudiant trouv�
                } else if (numero_inscription < buf.etudiants[*j].numero_inscription) {
                    sup = *j - 1; /// Rechercher dans la moiti� gauche
                } else {
                    inf = *j + 1; /// Rechercher dans la moiti� droite
                }
            }

            /// Si l'�tudiant n'est pas trouv�, d�terminer o� il devrait se trouver
            if (!(*Trouv)) {
                *j = inf; /// Position o� l'�tudiant devrait �tre ins�r�
            }
            stop = 1; /// Arr�ter la recherche externe
        } else if (numero_inscription < buf.etudiants[0].numero_inscription) {
            bs = *i - 1; /// Rechercher dans la moiti� gauche des blocs
        } else {
            bi = *i + 1; /// Rechercher dans la moiti� droite des blocs
        }
    }

    /// Si la recherche externe termine sans trouver l'�tudiant
    if (bi > bs) {
        *i = bi; /// Bloc o� l'�tudiant devrait �tre ins�r�
        *j = 1;  /// Position dans le bloc
    }

    fclose(fichier);
    return 0;
}
int suppression_logique(const char *fichier_bin, int numero_inscription) {
    int Trouv, i, j;

    /// Rechercher l'�tudiant � supprimer
    recherche_dichotomique(fichier_bin, numero_inscription, &Trouv, &i, &j);

    if (Trouv) {
        /// Ouvrir le fichier en mode lecture/�criture binaire
        FILE *fichier = fopen(fichier_bin, "r+b");
        if (fichier == NULL) {
            printf("Erreur lors de l'ouverture du fichier\n");
            return-1;
        }

        /// Se positionner sur le bloc contenant l'�tudiant � supprimer
        fseek(fichier, sizeof(Entete) + (i - 1) * sizeof(Bloc), SEEK_SET);
        Bloc buf;
        fread(&buf, sizeof(Bloc), 1, fichier); /// Lire le bloc

        /// Marquer l'�tudiant comme supprim�
        buf.etudiants[j].indicateur_suppression = 1;

        /// R��crire le bloc modifi� dans le fichier
        fseek(fichier, sizeof(Entete) + (i - 1) * sizeof(Bloc), SEEK_SET);
        fwrite(&buf, sizeof(Bloc), 1, fichier);

        fclose(fichier);
        printf("L'�tudiant avec le num�ro d'inscription %d a �t� marqu� comme supprim�.\n", numero_inscription);
    } else {
        printf("Aucun �tudiant trouv� avec le num�ro d'inscription %d.\n", numero_inscription);
    }
    return 0;
}



/// Cette fonction nous permet de visualiser le contenue du fichier binaire
int afficher_contenu_fichier_binaire(const char *nom_fichier) {
    FILE *fichier = fopen(nom_fichier, "rb"); // Ouverture en mode lecture binaire
    if (fichier == NULL) {
        printf("Erreur lors de l'ouverture du fichier\n");
        return -1;
    }

    // Lire l'ent�te du fichier
    Entete entete;
    fread(&entete, sizeof(Entete), 1, fichier);

    // Afficher les informations g�n�rales du fichier
    printf("Nombre total d'�tudiants: %d\n", entete.compteur_inserts);
    printf("Nombre de blocs utilis�s: %d\n", entete.nb_blocs_utilises);
    printf("\n");

    Bloc bloc; // Pour stocker un bloc d'�tudiants

    // Parcourir tous les blocs du fichier
    for (int i = 0; i < entete.nb_blocs_utilises; i++) {
        fread(&bloc, sizeof(Bloc), 1, fichier);

        // Parcourir tous les �tudiants dans le bloc courant
        for (int j = 0; j < bloc.nb_enregistrements; j++) {
            Etudiant etudiant = bloc.etudiants[j];

            // Afficher les informations de l'�tudiant en colonnes
            printf("+---------------------+---------------------+\n");
            printf("| %-20s | %-20s |\n", "Num�ro d'inscription", "Nom");
            printf("+---------------------+---------------------+\n");
            printf("| %-20d | %-20s |\n", etudiant.numero_inscription, etudiant.nom);
            printf("+---------------------+---------------------+\n");
            printf("| %-20s | %-20s |\n", "Pr�nom", "Ann�e de naissance");
            printf("+---------------------+---------------------+\n");
            printf("| %-20s | %-20d |\n", etudiant.prenom, etudiant.annee_naissance);
            printf("+---------------------+---------------------+\n");
            printf("| %-20s | %-20s |\n", "Groupe", "Moyenne pond�r�e");
            printf("+---------------------+---------------------+\n");
            printf("| %-20s | %-20.2f |\n", etudiant.groupe, etudiant.moyenne_ponderee);
            printf("+---------------------+---------------------+\n");
            printf("| %-20s | %-20s |\n", "Indicateur suppression", etudiant.indicateur_suppression ? "Supprim�" : "Actif");
            printf("+---------------------+---------------------+\n");

            // Afficher les notes et coefficients
            printf("| %-20s | %-20s |\n", "Module", "Note (Coefficient)");
            printf("+---------------------+---------------------+\n");
            for (int k = 0; k < 4; k++) {
                printf("| %-20d | %-5.2f (%-2d)          |\n", k + 1, etudiant.notes[k], etudiant.coefficients[k]);
            }
            printf("+---------------------+---------------------+\n");
            printf("\n"); // Espace entre les �tudiants
        }
    }

    fclose(fichier);
    return 0;
}

int modifier_etudiant(const char *fichier_bin, int numero_inscription) {
    int Trouv, i, j;
    recherche_dichotomique(fichier_bin, numero_inscription, &Trouv, &i, &j);

    if (Trouv) {
        FILE *fichier = fopen(fichier_bin, "r+b");
        if (fichier == NULL) {
            printf("Erreur lors de l'ouverture du fichier\n");
            return -1;
        }

        /// Se positionner sur le bloc contenant l'�tudiant
        fseek(fichier, sizeof(Entete) + (i - 1) * sizeof(Bloc), SEEK_SET);
        Bloc buf;
        fread(&buf, sizeof(Bloc), 1, fichier);

        Etudiant *etudiant = &buf.etudiants[j];
        int continuer = 1; /// Indicateur pour continuer ou non la modification

        while (continuer) {
            /// Afficher les informations actuelles de l'�tudiant
            printf("\nInformations actuelles de l'�tudiant :\n");
            printf("1. Nom : %s\n", etudiant->nom);
            printf("2. Pr�nom : %s\n", etudiant->prenom);
            printf("3. Ann�e de naissance : %d\n", etudiant->annee_naissance);
            printf("4. Groupe : %s\n", etudiant->groupe);
            printf("5. Notes et coefficients :\n");
            for (int k = 0; k < 4; k++) {
                printf("   Module %d : Note = %.2f, Coefficient = %d\n", k + 1, etudiant->notes[k], etudiant->coefficients[k]);
            }
            printf("6. Quitter\n");

            /// Demander � l'utilisateur quel champ modifier
            int choix;
            printf("\nEntrez le num�ro du champ � modifier (1-6) : ");
            scanf("%d", &choix);

            /// Modifier le champ s�lectionn�
            switch (choix) {
                case 1:
                    printf("Nouveau nom : ");
                    scanf("%s", etudiant->nom);
                    break;
                case 2:
                    printf("Nouveau pr�nom : ");
                    scanf("%s", etudiant->prenom);
                    break;
                case 3:
                    printf("Nouvelle ann�e de naissance : ");
                    scanf("%d", &etudiant->annee_naissance);
                    break;
                case 4:
                    printf("Nouveau groupe : ");
                    scanf("%s", etudiant->groupe);
                    break;
                case 5:
                    printf("Nouvelles notes et coefficients :\n");
                    for (int k = 0; k < 4; k++) {
                        printf("Module %d :\n", k + 1);
                        printf("   Nouvelle note : ");
                        scanf("%f", &etudiant->notes[k]);
                        printf("   Nouveau coefficient : ");
                        scanf("%d", &etudiant->coefficients[k]);
                    }
                    /// Recalculer la moyenne pond�r�e
                    float somme_notes_ponderees = 0;
                    int somme_coefficients = 0;
                    for (int k = 0; k < 4; k++) {
                        somme_notes_ponderees += etudiant->notes[k] * etudiant->coefficients[k];
                        somme_coefficients += etudiant->coefficients[k];
                    }
                    etudiant->moyenne_ponderee = somme_notes_ponderees / somme_coefficients;
                    break;
                case 6:
                    continuer = 0; /// Quitter la boucle
                    break;
                default:
                    printf("Choix invalide. Veuillez r�essayer.\n");
                    break;
            }

            /// R��crire le bloc modifi� dans le fichier apr�s chaque modification
            fseek(fichier, sizeof(Entete) + (i - 1) * sizeof(Bloc), SEEK_SET);
            fwrite(&buf, sizeof(Bloc), 1, fichier);
        }

        fclose(fichier);
        printf("\nLes modifications ont �t� enregistr�es.\n");
    } else {
        printf("Aucun �tudiant trouv� avec le num�ro d'inscription %d.\n", numero_inscription);
    }
    return 0;
}

int ajouter_etudiant(const char *fichier_bin) {
    Etudiant nouvel_etudiant; // Structure pour stocker les informations du nouvel �tudiant
    int Trouv, i, j; // Variables pour la recherche dichotomique

    /// �tape 1 : Saisie du num�ro d'inscription
    printf("Ajout d'un nouvel �tudiant :\n");
    printf("Num�ro d'inscription : ");
    if (scanf("%d", &nouvel_etudiant.numero_inscription) != 1) {
        printf("Erreur: Num�ro d'inscription invalide.\n");
        return -1;
    }

    /// �tape 2 : V�rifier si l'�tudiant existe d�j�
    if (recherche_dichotomique(fichier_bin, nouvel_etudiant.numero_inscription, &Trouv, &i, &j) != 0) {
        printf("Erreur lors de la recherche de l'�tudiant.\n");
        return -1;
    }

    if (Trouv) {
        // Si l'�tudiant existe d�j�, afficher un message d'erreur et arr�ter la fonction
        printf("Un �tudiant avec ce num�ro d'inscription existe d�j�.\n");
        return -1;
    }

    /// �tape 3 : Saisie des autres informations de l'�tudiant
    printf("Nom : ");
    if (scanf("%49s", nouvel_etudiant.nom) != 1) {
        printf("Erreur: Nom invalide.\n");
        return -1;
    }

    printf("Pr�nom : ");
    if (scanf("%49s", nouvel_etudiant.prenom) != 1) {
        printf("Erreur: Pr�nom invalide.\n");
        return -1;
    }

    printf("Ann�e de naissance : ");
    if (scanf("%d", &nouvel_etudiant.annee_naissance) != 1) {
        printf("Erreur: Ann�e de naissance invalide.\n");
        return -1;
    }

    printf("Groupe : ");
    if (scanf("%9s", nouvel_etudiant.groupe) != 1) {
        printf("Erreur: Groupe invalide.\n");
        return -1;
    }

    printf("Notes et coefficients :\n");
    for (int k = 0; k < 4; k++) {
        printf("Module %d :\n", k + 1);
        printf("   Note : ");
        if (scanf("%f", &nouvel_etudiant.notes[k]) != 1 || nouvel_etudiant.notes[k] < 0 || nouvel_etudiant.notes[k] > 20) {
            printf("Erreur: Note invalide (doit �tre entre 0 et 20).\n");
            return -1;
        }

        printf("   Coefficient : ");
        if (scanf("%d", &nouvel_etudiant.coefficients[k]) != 1 || nouvel_etudiant.coefficients[k] <= 0) {
            printf("Erreur: Coefficient invalide (doit �tre un entier positif).\n");
            return -1;
        }
    }

    /// �tape 4 : Calcul de la moyenne pond�r�e
    float somme_notes_ponderees = 0; // Somme des notes pond�r�es
    int somme_coefficients = 0; // Somme des coefficients
    for (int k = 0; k < 4; k++) {
        somme_notes_ponderees += nouvel_etudiant.notes[k] * nouvel_etudiant.coefficients[k];
        somme_coefficients += nouvel_etudiant.coefficients[k];
    }
    // Calcul de la moyenne pond�r�e
    nouvel_etudiant.moyenne_ponderee = somme_notes_ponderees / somme_coefficients;

    // Positionner l'indice de suppression � 0
    nouvel_etudiant.indicateur_suppression = 0;

    /// �tape 5 : Recherche de la position d'insertion
    if (recherche_dichotomique(fichier_bin, nouvel_etudiant.numero_inscription, &Trouv, &i, &j) != 0) {
        printf("Erreur lors de la recherche de la position d'insertion.\n");
        return -1;
    }

    if (!Trouv) {
        // Ouvrir le fichier en mode lecture/�criture binaire
        FILE *fichier = fopen(fichier_bin, "r+b");
        if (fichier == NULL) {
            printf("Erreur lors de l'ouverture du fichier.\n");
            return -1;
        }

        // Lire l'ent�te du fichier
        FichierTOF F;
        F.fichier = fichier;
        lire_entete(&F);

        Bloc buf; // Buffer pour stocker un bloc d'�tudiants
        Etudiant x; // Variable temporaire pour stocker un �tudiant lors des d�calages
        int continu = 1; // Indicateur pour continuer ou non les d�calages

        /// �tape 6 : D�calage des enregistrements et insertion
        while (continu && i <= F.entete.nb_blocs_utilises) {
            // Lire le bloc courant
            fseek(fichier, sizeof(Entete) + (i - 1) * sizeof(Bloc), SEEK_SET);
            if (fread(&buf, sizeof(Bloc), 1, fichier) != 1) {
                printf("Erreur lors de la lecture du bloc.\n");
                fclose(fichier);
                return -1;
            }

            // Sauvegarder le dernier enregistrement du bloc courant
            x = buf.etudiants[buf.nb_enregistrements - 1];

            // D�caler les enregistrements vers le bas pour faire de la place
            for (int k = buf.nb_enregistrements; k > j; k--) {
                buf.etudiants[k] = buf.etudiants[k - 1];
            }

            // Ins�rer le nouvel �tudiant � la position j
            buf.etudiants[j] = nouvel_etudiant;

            // Si le bloc n'est pas plein, ins�rer x � la fin et arr�ter
            if (buf.nb_enregistrements < MAX_ENREGISTREMENTS) {
                buf.nb_enregistrements++; // Augmenter le nombre d'enregistrements dans le bloc
                buf.etudiants[buf.nb_enregistrements - 1] = x; // Ins�rer x � la fin
                fseek(fichier, sizeof(Entete) + (i - 1) * sizeof(Bloc), SEEK_SET);
                if (fwrite(&buf, sizeof(Bloc), 1, fichier) != 1) {
                    printf("Erreur lors de l'�criture du bloc.\n");
                    fclose(fichier);
                    return -1;
                }
                continu = 0; // Arr�ter la boucle
            } else {
                // Si le bloc est plein, �crire le bloc et passer au suivant
                fseek(fichier, sizeof(Entete) + (i - 1) * sizeof(Bloc), SEEK_SET);
                if (fwrite(&buf, sizeof(Bloc), 1, fichier) != 1) {
                    printf("Erreur lors de l'�criture du bloc.\n");
                    fclose(fichier);
                    return -1;
                }
                i++; // Passer au bloc suivant
                j = 0; // Position d'insertion dans le nouveau bloc
                nouvel_etudiant = x; // x devient le nouvel �tudiant � ins�rer
            }
        }

        /// �tape 7 : Si on d�passe la fin du fichier, ajouter un nouveau bloc
        if (i > F.entete.nb_blocs_utilises) {
            buf.etudiants[0] = nouvel_etudiant; // Ins�rer l'�tudiant dans le nouveau bloc
            buf.nb_enregistrements = 1; // Le nouveau bloc contient un seul enregistrement
            fseek(fichier, sizeof(Entete) + (i - 1) * sizeof(Bloc), SEEK_SET);
            if (fwrite(&buf, sizeof(Bloc), 1, fichier) != 1) {
                printf("Erreur lors de l'�criture du nouveau bloc.\n");
                fclose(fichier);
                return -1;
            }
            F.entete.nb_blocs_utilises++; // Mettre � jour le nombre de blocs utilis�s
        }

        /// �tape 8 : Mettre � jour l'ent�te du fichier
        F.entete.compteur_inserts++; // Incr�menter le compteur d'insertions
        mettre_a_jour_entete(&F); // R��crire l'ent�te dans le fichier

        // Fermer le fichier
        fclose(fichier);
        printf("L'�tudiant a �t� ajout� avec succ�s.\n");
    }

    return 0;
}

int fichier_existe(const char *nom_fichier) {
    FILE *fichier = fopen(nom_fichier, "r");
    if (fichier) {
        fclose(fichier);
        return 1; // Le fichier existe
    }
    return 0; // Le fichier n'existe pas
}

// Fonction pour v�rifier si un taux de chargement est valide (entre 0 et 1)
int taux_chargement_valide(float taux) {
    return (taux >= 0.0 && taux <= 1.0);
}

void extractByClass(const char *fichier_bin, const char *class_name) {
    FILE *fichier = fopen(fichier_bin, "rb");
    if (fichier == NULL) {
        printf("Erreur: Impossible d'ouvrir le fichier %s.\n", fichier_bin);
        return;
    }

    // Lire l'ent�te du fichier
    Entete entete;
    fread(&entete, sizeof(Entete), 1, fichier);

    Bloc bloc;
    Etudiant filtered[MAX_ENREGISTREMENTS * entete.nb_blocs_utilises]; // Tableau pour stocker les �tudiants filtr�s
    int filtered_count = 0; // Compteur pour les �tudiants filtr�s

    // Parcourir tous les blocs du fichier
    for (int i = 0; i < entete.nb_blocs_utilises; i++) {
        fread(&bloc, sizeof(Bloc), 1, fichier);
        for (int j = 0; j < bloc.nb_enregistrements; j++) {
            Etudiant etudiant = bloc.etudiants[j];
            // V�rifier si l'�tudiant appartient � la classe sp�cifi�e et n'est pas supprim�
            if (strcmp(etudiant.groupe, class_name) == 0 && etudiant.indicateur_suppression == 0) {
                if (filtered_count < MAX_ENREGISTREMENTS * entete.nb_blocs_utilises) {
                    filtered[filtered_count++] = etudiant;
                } else {
                    printf("Attention: Trop d'�tudiants � filtrer. Certains ont �t� ignor�s.\n");
                }
            }
        }
    }
    fclose(fichier);

    if (filtered_count == 0) {
        printf("Aucun �tudiant trouv� pour la classe %s.\n", class_name);
        return;
    }

    // Trier les �tudiants par moyenne pond�r�e (ordre d�croissant)
    for (int i = 0; i < filtered_count - 1; i++) {
        for (int j = 0; j < filtered_count - i - 1; j++) {
            if (filtered[j].moyenne_ponderee < filtered[j + 1].moyenne_ponderee) {
                Etudiant temp = filtered[j];
                filtered[j] = filtered[j + 1];
                filtered[j + 1] = temp;
            }
        }
    }

    // Afficher les �tudiants filtr�s et tri�s
    printf("\n�tudiants de la classe %s tri�s par moyenne pond�r�e (d�croissant) :\n", class_name);
    printf("=================================================================\n");
    printf("| %-5s | %-15s | %-15s | %-7s |\n", "ID", "Nom", "Pr�nom", "Moyenne");
    printf("=================================================================\n");

    for (int i = 0; i < filtered_count; i++) {
        printf("| %-5d | %-15s | %-15s | %-7.2f |\n",
               filtered[i].numero_inscription,
               filtered[i].nom,
               filtered[i].prenom,
               filtered[i].moyenne_ponderee);
    }
    printf("=================================================================\n");
}


int main() {
    int choix;
    char fichier_txt[100], fichier_bin[100];
    float taux_chargement;
    int numero_inscription;
    char class_name[10]; // Pour stocker le nom de la classe � extraire

    printf("Bienvenue dans le gestionnaire d'�tudiants!\n");

    while (1) {
        printf("\nMenu:\n");
        printf("1. Chargement initial des donn�es\n");
        printf("2. R�organisation du fichier binaire\n");
        printf("3. Suppression logique d'un �tudiant\n");
        printf("4. Afficher le contenu du fichier binaire\n");
        printf("5. Modifier les informations d'un �tudiant\n");
        printf("6. Ajouter un nouvel �tudiant\n");
        printf("7. Rechercher un �tudiant\n");
        printf("8. Extraire les �tudiants par classe\n");
        printf("9. Quitter\n");
        printf("Entrez votre choix: ");
        scanf("%d", &choix);

        switch (choix) {
            case 1: {
                // Chargement initial des donn�es
                printf("Entrez le nom du fichier texte: ");
                scanf("%s", fichier_txt);
                printf("Entrez le nom du fichier binaire: ");
                scanf("%s", fichier_bin);
                printf("Entrez le taux de chargement (entre 0 et 1): ");
                scanf("%f", &taux_chargement);

                // V�rifications
                if (!fichier_existe(fichier_txt)) {
                    printf("Erreur: Le fichier texte '%s' n'existe pas.\n", fichier_txt);
                    break;
                }
                if (!taux_chargement_valide(taux_chargement)) {
                    printf("Erreur: Le taux de chargement doit �tre compris entre 0 et 1.\n");
                    break;
                }

                // Appel de la fonction de chargement initial
                if (chargement_initial(fichier_txt, fichier_bin, taux_chargement) == 0) {
                    printf("Chargement initial r�ussi!\n");
                } else {
                    printf("Erreur lors du chargement initial.\n");
                }
                break;
            }

            case 2: {
                // R�organisation du fichier binaire
                printf("Entrez le nom du fichier binaire: ");
                scanf("%s", fichier_bin);
                printf("Entrez le taux de chargement (entre 0 et 1): ");
                scanf("%f", &taux_chargement);

                // V�rifications
                if (!fichier_existe(fichier_bin)) {
                    printf("Erreur: Le fichier binaire '%s' n'existe pas.\n", fichier_bin);
                    break;
                }
                if (!taux_chargement_valide(taux_chargement)) {
                    printf("Erreur: Le taux de chargement doit �tre compris entre 0 et 1.\n");
                    break;
                }

                // Appel de la fonction de r�organisation
                if (Reorganisation(fichier_bin, taux_chargement) == 0) {
                    printf("R�organisation r�ussie!\n");
                } else {
                    printf("Erreur lors de la r�organisation.\n");
                }
                break;
            }

            case 3: {
                // Suppression logique d'un �tudiant
                printf("Entrez le nom du fichier binaire: ");
                scanf("%s", fichier_bin);
                printf("Entrez le num�ro d'inscription de l'�tudiant � supprimer: ");
                scanf("%d", &numero_inscription);

                // V�rifications
                if (!fichier_existe(fichier_bin)) {
                    printf("Erreur: Le fichier binaire '%s' n'existe pas.\n", fichier_bin);
                    break;
                }

                // Appel de la fonction de suppression logique
                if (suppression_logique(fichier_bin, numero_inscription) == 0) {
                    printf("Suppression logique r�ussie!\n");
                } else {
                    printf("Erreur lors de la suppression logique.\n");
                }
                break;
            }

            case 4: {
                // Afficher le contenu du fichier binaire
                printf("Entrez le nom du fichier binaire: ");
                scanf("%s", fichier_bin);

                // V�rifications
                if (!fichier_existe(fichier_bin)) {
                    printf("Erreur: Le fichier binaire '%s' n'existe pas.\n", fichier_bin);
                    break;
                }

                // Appel de la fonction d'affichage
                if (afficher_contenu_fichier_binaire(fichier_bin) == 0) {
                    printf("Affichage r�ussi!\n");
                } else {
                    printf("Erreur lors de l'affichage du contenu.\n");
                }
                break;
            }

            case 5: {
                // Modifier les informations d'un �tudiant
                printf("Entrez le nom du fichier binaire: ");
                scanf("%s", fichier_bin);
                printf("Entrez le num�ro d'inscription de l'�tudiant � modifier: ");
                scanf("%d", &numero_inscription);

                // V�rifications
                if (!fichier_existe(fichier_bin)) {
                    printf("Erreur: Le fichier binaire '%s' n'existe pas.\n", fichier_bin);
                    break;
                }

                // Appel de la fonction de modification
                if (modifier_etudiant(fichier_bin, numero_inscription) == 0) {
                    printf("Modification r�ussie!\n");
                } else {
                    printf("Erreur lors de la modification.\n");
                }
                break;
            }

            case 6: {
                // Ajouter un nouvel �tudiant
                printf("Entrez le nom du fichier binaire: ");
                scanf("%s", fichier_bin);

                // V�rifications
                if (!fichier_existe(fichier_bin)) {
                    printf("Erreur: Le fichier binaire '%s' n'existe pas.\n", fichier_bin);
                    break;
                }

                // Appel de la fonction d'ajout
                if (ajouter_etudiant(fichier_bin) == 0) {
                    printf("Ajout r�ussi!\n");
                } else {
                    printf("Erreur lors de l'ajout.\n");
                }
                break;
            }

            case 7: {
                // Rechercher un �tudiant
                printf("Entrez le nom du fichier binaire: ");
                scanf("%s", fichier_bin);
                printf("Entrez le num�ro d'inscription de l'�tudiant � rechercher: ");
                scanf("%d", &numero_inscription);

                // V�rifications
                if (!fichier_existe(fichier_bin)) {
                    printf("Erreur: Le fichier binaire '%s' n'existe pas.\n", fichier_bin);
                    break;
                }

                int Trouv, i, j;
                if (recherche_dichotomique(fichier_bin, numero_inscription, &Trouv, &i, &j) == 0) {
                    if (Trouv) {
                        // Ouvrir le fichier pour lire les informations de l'�tudiant trouv�
                        FILE *fichier = fopen(fichier_bin, "rb");
                        if (fichier == NULL) {
                            printf("Erreur lors de l'ouverture du fichier.\n");
                            break;
                        }

                        // Se positionner sur le bloc contenant l'�tudiant
                        fseek(fichier, sizeof(Entete) + (i - 1) * sizeof(Bloc), SEEK_SET);
                        Bloc buf;
                        fread(&buf, sizeof(Bloc), 1, fichier);

                        // Afficher les informations de l'�tudiant trouv�
                        Etudiant etudiant = buf.etudiants[j];
                        printf("\n�tudiant trouv� :\n");
                        printf("Num�ro d'inscription: %d\n", etudiant.numero_inscription);
                        printf("Nom: %s\n", etudiant.nom);
                        printf("Pr�nom: %s\n", etudiant.prenom);
                        printf("Ann�e de naissance: %d\n", etudiant.annee_naissance);
                        printf("Groupe: %s\n", etudiant.groupe);
                        printf("Notes: \n");
                        for (int k = 0; k < 4; k++) {
                            printf("Module %d: %.2f (Coefficient: %d)\n", k + 1, etudiant.notes[k], etudiant.coefficients[k]);
                        }
                        printf("Moyenne pond�r�e: %.2f\n", etudiant.moyenne_ponderee);
                        printf("Indicateur de suppression: %d\n", etudiant.indicateur_suppression);

                        fclose(fichier);
                    } else {
                        printf("Aucun �tudiant trouv� avec le num�ro d'inscription %d.\n", numero_inscription);
                    }
                } else {
                    printf("Erreur lors de la recherche.\n");
                }
                break;
            }

            case 8: {
                // Extraire les �tudiants par classe
                printf("Entrez le nom du fichier binaire: ");
                scanf("%s", fichier_bin);
                printf("Entrez le nom de la classe � extraire (ex: 3A): ");
                scanf("%9s", class_name);

                // V�rifications
                if (!fichier_existe(fichier_bin)) {
                    printf("Erreur: Le fichier binaire '%s' n'existe pas.\n", fichier_bin);
                    break;
                }

                // Appel de la fonction extractByClass
                extractByClass(fichier_bin, class_name);
                break;
            }

            case 9: {
                // Quitter le programme
                printf("Au revoir!\n");
                exit(0);
            }

            default: {
                // Choix invalide
                printf("Choix invalide. Veuillez r�essayer.\n");
                break;
            }
        }
    }

    return 0;
}
