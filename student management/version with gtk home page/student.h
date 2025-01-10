#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NAME 50
#define MAX_CLASSE 10
#define MAX_NOTES 4
#define MAX_STUDENTS_PER_BLOC 10
#define MAX_BLOCS 10
#define MAX_FILTERED_STUDENTS 100
#define MAX_ENREGISTREMENTS 10
#define MAX_LINE_LENGTH 256

const char *nom_fichier = "students.bin";


typedef struct {
    int numero_inscription;
    char nom[MAX_NAME];
    char prenom[MAX_NAME];
    int annee_naissance;
    char classe[MAX_CLASSE];
    float notes[MAX_NOTES];
    int coefficients[MAX_NOTES];
    float moyenne;
    int supprime; // 0 = actif, 1 = supprimé
} Etudiant;

// Function declarations
void ajouter_etudiant();
void afficher_contenu_fichier_binaire();
void recherche_dichotomique(int numero_inscription, int *Trouv, int *i, int *j);
int find();
void calculer_moyenne(Etudiant *etudiant);
void extractByClass();
void modifier_etudiant();

typedef struct {
    Etudiant etudiants[MAX_ENREGISTREMENTS]; /// Tableau d'étudiants
    int nb_enregistrements;                  /// Nombre d'enregistrements dans ce bloc
} Bloc;

/// Structure de l'entête
typedef struct {
    int nb_blocs_utilises;    /// Nombre total de blocs utilisés
    int compteur_inserts;     /// Nombre total d'enregistrements insérés
} Entete;

/// Structure représentant le fichier TOF avec son entête
typedef struct {
    FILE *fichier;   /// Pointeur vers le fichier
    Entete entete;   /// Entête du fichier
} FichierTOF;

/// Fonction pour lire l'entête du fichier
void lire_entete(FichierTOF *f) {
    rewind(f->fichier); /// Retour au début du fichier
    fread(&(f->entete), sizeof(Entete), 1, f->fichier); /// Lecture de l'entête
}

/// Fonction pour mettre à jour l'entête du fichier
void mettre_a_jour_entete(FichierTOF *f) {
    rewind(f->fichier); /// Retour au début du fichier
    fwrite(&(f->entete), sizeof(Entete), 1, f->fichier); /// Écriture de l'entête
}

/// Fonction pour initialiser le fichier TOF
void initialiser_fichier(FichierTOF *f) {
    f->fichier = fopen(nom_fichier, "wb");
    if (f->fichier == NULL) {
        printf("Erreur lors de l'ouverture du fichier");
        exit(-1);
    }

    /// Initialiser l'entête
    f->entete.nb_blocs_utilises = 0;
    f->entete.compteur_inserts = 0;

    /// Écrire l'entête dans le fichier
    fwrite(&(f->entete), sizeof(Entete), 1, f->fichier);
}

void calculer_moyenne(Etudiant *e){
    float somme_notes_ponderees = 0; // Somme des notes pondérées
    int somme_coefficients = 0; // Somme des coefficients
    for (int k = 0; k < 4; k++) {
        somme_notes_ponderees += e->notes[k] * e->coefficients[k];
        somme_coefficients += e->coefficients[k];
    }

    e->moyenne = somme_notes_ponderees / somme_coefficients;
}

/// Cette fonction nous permet de visualiser le contenue du fichier binaire
void afficher_contenu_fichier_binaire(GtkListStore *list_store) {
    FILE *fichier = fopen(nom_fichier, "rb"); // Ouverture en mode lecture binaire
    if (fichier == NULL) {
        printf("Erreur lors de l'ouverture du fichier\n");
        return;
    }

    // Vider le GtkListStore avant de le remplir
    gtk_list_store_clear(list_store);

    Entete entete;
    fread(&entete, sizeof(Entete), 1, fichier); // Lire l'entête

    Bloc bloc;
    for (int i = 0; i < entete.nb_blocs_utilises; i++) {
        fread(&bloc, sizeof(Bloc), 1, fichier);
        for (int j = 0; j < bloc.nb_enregistrements; j++) {
            Etudiant etudiant = bloc.etudiants[j];
            if (!etudiant.supprime) { // Ignorer les étudiants supprimés
                GtkTreeIter iter;
                gtk_list_store_append(list_store, &iter); // Ajouter une nouvelle ligne
                gtk_list_store_set(list_store, &iter,
                                   0, etudiant.numero_inscription,
                                   1, etudiant.nom,
                                   2, etudiant.prenom,
                                   3, etudiant.annee_naissance,
                                   4, etudiant.classe,
                                   5, etudiant.moyenne,
                                   -1);
            }
        }
    }

    fclose(fichier);
}

void recherche_dichotomique(int numero_inscription, int *Trouv, int *i, int *j) {
    FILE *fichier = fopen(nom_fichier, "rb");
    if (fichier == NULL) {
        printf("Erreur lors de l'ouverture du fichier\n");
        return;
    }

    /// Lire l'entête pour connaître le nombre de blocs utilisés
    Entete entete;
    fread(&entete, sizeof(Entete), 1, fichier);

    /// Initialisation des variables
    int bi = 1; /// Borne inférieure (premier bloc)
    int bs = entete.nb_blocs_utilises; /// Borne supérieure (dernier bloc)
    *Trouv = 0; /// Indicateur de trouvé (faux par défaut)
    int stop = 0; /// Indicateur d'arrêt
    *j = 1; /// Position initiale dans le bloc

    /// Recherche externe (entre les blocs)
    while (bi <= bs && !(*Trouv) && !stop) {
        *i = (bi + bs) / 2; /// Bloc du milieu
        fseek(fichier, sizeof(Entete) + (*i - 1) * sizeof(Bloc), SEEK_SET);
        Bloc buf;
        fread(&buf, sizeof(Bloc), 1, fichier); /// Lire le bloc

        /// Vérifier si la clé est dans l'intervalle du bloc courant
        if (numero_inscription >= buf.etudiants[0].numero_inscription &&
            numero_inscription <= buf.etudiants[buf.nb_enregistrements - 1].numero_inscription) {
            /// Recherche interne (dans le bloc)
            int inf = 0; /// Borne inférieure dans le bloc
            int sup = buf.nb_enregistrements - 1; /// Borne supérieure dans le bloc
            while (inf <= sup && !(*Trouv)) {
                *j = (inf + sup) / 2; /// Position du milieu dans le bloc
                if (numero_inscription == buf.etudiants[*j].numero_inscription) {
                    *Trouv = 1; /// Étudiant trouvé
                } else if (numero_inscription < buf.etudiants[*j].numero_inscription) {
                    sup = *j - 1; /// Rechercher dans la moitié gauche
                } else {
                    inf = *j + 1; /// Rechercher dans la moitié droite
                }
            }

            /// Si l'étudiant n'est pas trouvé, déterminer où il devrait se trouver
            if (!(*Trouv)) {
                *j = inf; /// Position où l'étudiant devrait être inséré
            }
            stop = 1; /// Arrêter la recherche externe
        } else if (numero_inscription < buf.etudiants[0].numero_inscription) {
            bs = *i - 1; /// Rechercher dans la moitié gauche des blocs
        } else {
            bi = *i + 1; /// Rechercher dans la moitié droite des blocs
        }
    }

    /// Si la recherche externe termine sans trouver l'étudiant
    if (bi > bs) {
        *i = bi; /// Bloc où l'étudiant devrait être inséré
        *j = 1;  /// Position dans le bloc
    }

    fclose(fichier);
}


void ajouter_etudiant() {
    Etudiant nouvel_etudiant; // Structure pour stocker les informations du nouvel étudiant
    int Trouv, i, j; // Variables pour la recherche dichotomique

    /// Étape 1 : Saisie du numéro d'inscription
    printf("Ajout d'un nouvel étudiant :\n");
    printf("Numéro d'inscription : ");
    scanf("%d", &nouvel_etudiant.numero_inscription);

    /// Étape 2 : Vérifier si l'étudiant existe déjà
    recherche_dichotomique(nouvel_etudiant.numero_inscription, &Trouv, &i, &j);

    if (Trouv) {
        // Si l'étudiant existe déjà, afficher un message d'erreur et arrêter la fonction
        printf("Un étudiant avec ce numéro d'inscription existe déjà.\n");
        return;
    }

    /// Étape 3 : Saisie des autres informations de l'étudiant
    printf("Nom : ");
    scanf("%s", nouvel_etudiant.nom);
    printf("Prénom : ");
    scanf("%s", nouvel_etudiant.prenom);
    printf("Année de naissance : ");
    scanf("%d", &nouvel_etudiant.annee_naissance);
    printf("classe : ");
    scanf("%s", nouvel_etudiant.classe);
    printf("Notes et coefficients :\n");
    for (int k = 0; k < 4; k++) {
        printf("Module %d :\n", k + 1);
        printf("   Note : ");
        scanf("%f", &nouvel_etudiant.notes[k]);
        printf("   Coefficient : ");
        scanf("%d", &nouvel_etudiant.coefficients[k]);
    }
    //calcul de moyenne
    calculer_moyenne(&nouvel_etudiant);

    // positioné l'indice de supression a 0
    nouvel_etudiant.supprime=0;

    /// Étape 5 : Recherche de la position d'insertion
    recherche_dichotomique(nouvel_etudiant.numero_inscription, &Trouv, &i, &j);

    if (!Trouv) {
        // Ouvrir le fichier en mode lecture/écriture binaire
        FILE *fichier = fopen(nom_fichier, "r+b");
        if (fichier == NULL) {
            printf("Erreur lors de l'ouverture du fichier\n");
            return;
        }

        // Lire l'entête du fichier
        FichierTOF F;
        F.fichier = fichier;
        lire_entete(&F);

        Bloc buf; // Buffer pour stocker un bloc d'étudiants
        Etudiant x; // Variable temporaire pour stocker un étudiant lors des décalages
        int continu = 1; // Indicateur pour continuer ou non les décalages

        /// Étape 6 : Décalage des enregistrements et insertion
        while (continu && i <= F.entete.nb_blocs_utilises) {
            // Lire le bloc courant
            fseek(fichier, sizeof(Entete) + (i - 1) * sizeof(Bloc), SEEK_SET);
            fread(&buf, sizeof(Bloc), 1, fichier);

            // Sauvegarder le dernier enregistrement du bloc courant
            x = buf.etudiants[buf.nb_enregistrements - 1];

            // Décaler les enregistrements vers le bas pour faire de la place
            for (int k = buf.nb_enregistrements; k > j; k--) {
                buf.etudiants[k] = buf.etudiants[k - 1];
            }

            // Insérer le nouvel étudiant à la position j
            buf.etudiants[j] = nouvel_etudiant;

            // Si le bloc n'est pas plein, insérer x à la fin et arrêter
            if (buf.nb_enregistrements < MAX_ENREGISTREMENTS) {
                buf.nb_enregistrements++; // Augmenter le nombre d'enregistrements dans le bloc
                buf.etudiants[buf.nb_enregistrements - 1] = x; // Insérer x à la fin
                fseek(fichier, sizeof(Entete) + (i - 1) * sizeof(Bloc), SEEK_SET);
                fwrite(&buf, sizeof(Bloc), 1, fichier); // Réécrire le bloc modifié
                continu = 0; // Arrêter la boucle
            } else {
                // Si le bloc est plein, écrire le bloc et passer au suivant
                fseek(fichier, sizeof(Entete) + (i - 1) * sizeof(Bloc), SEEK_SET);
                fwrite(&buf, sizeof(Bloc), 1, fichier);
                i++; // Passer au bloc suivant
                j = 0; // Position d'insertion dans le nouveau bloc
                nouvel_etudiant = x; // x devient le nouvel étudiant à insérer
            }
        }

        /// Étape 7 : Si on dépasse la fin du fichier, ajouter un nouveau bloc
        if (i > F.entete.nb_blocs_utilises) {
            buf.etudiants[0] = nouvel_etudiant; // Insérer l'étudiant dans le nouveau bloc
            buf.nb_enregistrements = 1; // Le nouveau bloc contient un seul enregistrement
            fseek(fichier, sizeof(Entete) + (i - 1) * sizeof(Bloc), SEEK_SET);
            fwrite(&buf, sizeof(Bloc), 1, fichier); // Écrire le nouveau bloc
            F.entete.nb_blocs_utilises++; // Mettre à jour le nombre de blocs utilisés
        }

        /// Étape 8 : Mettre à jour l'entête du fichier
        F.entete.compteur_inserts++; // Incrémenter le compteur d'insertions
        mettre_a_jour_entete(&F); // Réécrire l'entête dans le fichier

        // Fermer le fichier
        fclose(fichier);
        printf("L'étudiant a été ajouté avec succès.\n");
    }
}

int find(){
    /// Recherche dichotomique d'un étudiant par son numéro d'inscription
    int Trouv, i, j;
    int numero_inscription_recherche;
    printf("id d'etudiant:\n");
    scanf("%d", &numero_inscription_recherche);
    recherche_dichotomique(numero_inscription_recherche, &Trouv, &i, &j);

    if (Trouv) {
        /// Ouvrir le fichier pour lire les informations de l'étudiant trouvé
        FILE *fichier = fopen(nom_fichier, "rb");
        if (fichier == NULL) {
            printf("Erreur lors de l'ouverture du fichier\n");
            return -1;
        }

        /// Se positionner sur le bloc contenant l'étudiant
        fseek(fichier, sizeof(Entete) + (i - 1) * sizeof(Bloc), SEEK_SET);
        Bloc buf;
        fread(&buf, sizeof(Bloc), 1, fichier); /// Lire le bloc

        /// Afficher les informations de l'étudiant trouvé
        Etudiant etudiant = buf.etudiants[j];
        printf("Étudiant trouvé:\n");
        printf("Numéro d'inscription: %d\n", etudiant.numero_inscription);
        printf("Nom: %s\n", etudiant.nom);
        printf("Prénom: %s\n", etudiant.prenom);
        printf("Année de naissance: %d\n", etudiant.annee_naissance);
        printf("Groupe: %s\n", etudiant.classe);
        printf("Notes: \n");
        for (int k = 0; k < 4; k++) {
            printf("Module %d: %.2f ", k + 1, etudiant.notes[k]);
            printf("__Coefficients: %d \n", etudiant.coefficients[k]);
        }
        printf("Moyenne pondérée: %.2f\n", etudiant.moyenne);
        printf("\n");

        fclose(fichier);
    } else {
        printf("Étudiant non trouvé. Il devrait être inséré dans le bloc %d à la position %d\n", i, j);
    }

    return 0;
}
/// Fonction pour réorganiser le fichier binaire en supprimant les enregistrements marqués comme supprimés
int Reorganisation() {
    float T = 0.8f;
    FichierTOF *F = (FichierTOF *)malloc(sizeof(FichierTOF));
    F->fichier = fopen(nom_fichier, "rb");
    if (F->fichier == NULL) {
        printf("Erreur d'ouverture du fichier %s\n", nom_fichier);
        return -1;
    }
    lire_entete(F);

    FichierTOF *F1 = (FichierTOF *)malloc(sizeof(FichierTOF));
    F1->fichier = fopen("temp.bin", "wb");
    if (F1->fichier == NULL) {
        printf("Erreur d'ouverture du fichier temporaire\n");
        fclose(F->fichier);
        free(F);
        return -1;
    }

    F1->entete.nb_blocs_utilises = 0;
    F1->entete.compteur_inserts = 0;
    mettre_a_jour_entete(F1);

    Bloc Buf, Buf1;
    int nb_enr = 0;
    int nb_blc = 0;
    int j;
    int k;
    int q = 0;

    while (fread(&Buf, sizeof(Buf), 1, F->fichier) == 1) {
        j = Buf.nb_enregistrements;
        k = 0;
        while (k < j) {
            if (Buf.etudiants[k].supprime == 0) {
                if (q < T * MAX_ENREGISTREMENTS) {
                    Buf1.etudiants[q] = Buf.etudiants[k];
                    q++;
                } else {
                    Buf1.nb_enregistrements = q;
                    fwrite(&Buf1, sizeof(Bloc), 1, F1->fichier);
                    nb_blc++;
                    Buf1.etudiants[0] = Buf.etudiants[k];
                    q = 1;
                }
                nb_enr++;
            }
            k++;
        }
        if (q > 0) {
            Buf1.nb_enregistrements = q;
            fwrite(&Buf1, sizeof(Bloc), 1, F1->fichier);
            nb_blc++;
            q = 0;
        }
    }

    F1->entete.compteur_inserts = nb_enr;
    F1->entete.nb_blocs_utilises = nb_blc;
    mettre_a_jour_entete(F1);

    fclose(F->fichier);
    fclose(F1->fichier);
    free(F);
    free(F1);

    remove(nom_fichier);
    rename("temp.bin", nom_fichier);

    return 0;
}
void extractByClass() {
    FILE *file = fopen(nom_fichier, "rb");
    if (!file) {
        printf("Error: Could not open the file %s.\n", nom_fichier);
        return;
    }
    char class_name[10];
    printf("class name:\n");
    scanf("%s", class_name);
    // Read the entete
    Entete entete;
    fread(&entete, sizeof(entete), 1, file);

    Bloc bloc;
    Etudiant filtered[MAX_FILTERED_STUDENTS];
    int filtered_count = 0;

    // Process blocs
    for (int i = 0; i < entete.nb_blocs_utilises; i++) {
        fread(&bloc, sizeof(Bloc), 1, file);
        for (int j = 0; j < bloc.nb_enregistrements; j++) {
            Etudiant etudiant = bloc.etudiants[j];
            if (strcmp(etudiant.classe, class_name) == 0 && etudiant.supprime == 0) {
                if (filtered_count < MAX_FILTERED_STUDENTS) {
                    filtered[filtered_count++] = etudiant;
                } else {
                    printf("Warning: Too many etudiants to filter. Some may be ignored.\n");
                }
            }
        }
    }
    fclose(file);

    if (filtered_count == 0) {
        printf("No etudiants found for the class %s.\n", class_name);
        return;
    }

    // Sort etudiants by weighted average in descending order
    for (int i = 0; i < filtered_count - 1; i++) {
        for (int j = 0; j < filtered_count - i - 1; j++) {
            if (filtered[j].moyenne< filtered[j + 1].moyenne) {
                Etudiant temp = filtered[j];
                filtered[j] = filtered[j + 1];
                filtered[j + 1] = temp;
            }
        }
    }

    // Display the sorted etudiants
    printf("\netudiants from class %s sorted by weighted average (descending):\n", class_name);
    printf("=================================================================\n");
    printf("| %-5s | %-15s | %-15s | %-7s |\n", "ID", "First Name", "Last Name", "Average");
    printf("=================================================================\n");

    for (int i = 0; i < filtered_count; i++) {
        printf("| %-5d | %-15s | %-15s | %-7.2f |\n",
               filtered[i].numero_inscription,
               filtered[i].prenom,
               filtered[i].nom,
               filtered[i].moyenne);
    }
    printf("=================================================================\n");
}

void modifier_etudiant() {
    int Trouv, i, j, numero_inscription;
    printf("id d'etudiant:\n");
    scanf("%d", &numero_inscription);

    recherche_dichotomique(numero_inscription, &Trouv, &i, &j);

    if (Trouv) {
        FILE *fichier = fopen(nom_fichier, "r+b");
        if (fichier == NULL) {
            printf("Erreur lors de l'ouverture du fichier\n");
            return;
        }

        /// Se positionner sur le bloc contenant l'étudiant
        fseek(fichier, sizeof(Entete) + (i - 1) * sizeof(Bloc), SEEK_SET);
        Bloc buf;
        fread(&buf, sizeof(Bloc), 1, fichier);

        Etudiant *etudiant = &buf.etudiants[j];
        int continuer = 1; /// Indicateur pour continuer ou non la modification

        while (continuer) {
            /// Afficher les informations actuelles de l'étudiant
            printf("\nInformations actuelles de l'étudiant :\n");
            printf("1. Nom : %s\n", etudiant->nom);
            printf("2. Prénom : %s\n", etudiant->prenom);
            printf("3. Année de naissance : %d\n", etudiant->annee_naissance);
            printf("4. Groupe : %s\n", etudiant->classe);
            printf("5. Notes et coefficients :\n");
            for (int k = 0; k < 4; k++) {
                printf("   Module %d : Note = %.2f, Coefficient = %d\n", k + 1, etudiant->notes[k], etudiant->coefficients[k]);
            }
            printf("6. Quitter\n");

            /// Demander à l'utilisateur quel champ modifier
            int choix;
            printf("\nEntrez le numéro du champ à modifier (1-6) : ");
            scanf("%d", &choix);

            /// Modifier le champ sélectionné
            switch (choix) {
                case 1:
                    printf("Nouveau nom : ");
                    scanf("%s", etudiant->nom);
                    break;
                case 2:
                    printf("Nouveau prénom : ");
                    scanf("%s", etudiant->prenom);
                    break;
                case 3:
                    printf("Nouvelle année de naissance : ");
                    scanf("%d", &etudiant->annee_naissance);
                    break;
                case 4:
                    printf("Nouveau groupe : ");
                    scanf("%s", etudiant->classe);
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
                    calculer_moyenne(etudiant);
                    break;
                case 6:
                    continuer = 0; /// Quitter la boucle
                    break;
                default:
                    printf("Choix invalide. Veuillez réessayer.\n");
                    break;
            }

            /// Réécrire le bloc modifié dans le fichier après chaque modification
            fseek(fichier, sizeof(Entete) + (i - 1) * sizeof(Bloc), SEEK_SET);
            fwrite(&buf, sizeof(Bloc), 1, fichier);
        }

        fclose(fichier);
        printf("\nLes modifications ont été enregistrées.\n");
    } else {
        printf("Aucun étudiant trouvé avec le numéro d'inscription %d.\n", numero_inscription);
    }
}

void suppression_logique() {
    int Trouv, i, j, numero_inscription;
    printf("enter student id:\n");
    scanf("%d", &numero_inscription);

    recherche_dichotomique(numero_inscription, &Trouv, &i, &j);

    if (Trouv) {
        FILE *fichier = fopen(nom_fichier, "r+b");
        if (fichier == NULL) {
            printf("Erreur lors de l'ouverture du fichier\n");
            return;
        }

        // Verify block index i
        if (i >= 1 ) {
            // Check for fseek error
            if (fseek(fichier, sizeof(Entete) + (i - 1) * sizeof(Bloc), SEEK_SET) != 0) {
                perror("fseek error");
                fclose(fichier);
                return;
            }

            Bloc buf;
            if (fread(&buf, sizeof(Bloc), 1, fichier) != 1) {
                perror("fread error");
                fclose(fichier);
                return;
            }

            // Check j
            if (j >= 0 && j < buf.nb_enregistrements) {
               buf.etudiants[j].supprime = 1;

                // Seek back to the beginning of the block to rewrite
                if (fseek(fichier, sizeof(Entete) + (i - 1) * sizeof(Bloc), SEEK_SET) != 0) {
                    perror("fseek error (rewriting)");
                    fclose(fichier);
                    return;
                }

                if (fwrite(&buf, sizeof(Bloc), 1, fichier) != 1) {
                    perror("fwrite error");
                    fclose(fichier);
                    return;
                }

                printf("L'étudiant avec le numéro d'inscription %d a été marqué comme supprimé.\n", numero_inscription);
            } else {
                printf("Error: Invalid j value: %d\n", j);
            }
        } else {
            printf("Error: Invalid block index i: %d\n", i);
        }

        fclose(fichier);
    } else {
        printf("Aucun étudiant trouvé avec le numéro d'inscription %d.\n", numero_inscription);
    }
}

void initializeBinaryFile() {
    FILE *file = fopen(nom_fichier, "wb");
    if (!file) {
        printf("Error: Could not create the file %s.\n", nom_fichier);
        exit(1);
    }

    // Initialize the entete
    Entete entete = {0, 0};
    fwrite(&entete, sizeof(Entete), 1, file);

    fclose(file);
}

// Function to write etudiants to a binary file in blocs
void writeBinaryFile() {
  int size = 40;
  Etudiant etudiants[40] = {
    {1, "Ali", "Ahmed", 2001, "3A", {15.5, 12.0, 14.0, 16.5}, {4, 3, 2, 5}, 14.2, 0},
    {2, "Sarah", "Moulay", 2000, "2B", {18.0, 16.5, 17.0, 19.0}, {4, 3, 2, 5}, 17.4, 0},
    {3, "Karim", "Benmoussa", 2002, "1C", {10.0, 9.5, 12.0, 11.0}, {4, 3, 2, 5}, 10.8, 0},
    {4, "Benmoussa", "Karim", 2002, "1C", {10.0, 4.0, 9.5, 3.0}, {12, 2, 11, 5}, 10.8, 0},
    {5, "Yasmine", "Cherif", 2003, "2B", {19.0, 18.5, 17.5, 20.0}, {4, 3, 2, 5}, 18.8, 0},
    {6, "Amine", "Haddad", 2001, "3A", {14.0, 13.0, 15.0, 16.0}, {4, 3, 2, 5}, 14.8, 1},
    {7, "Fatima", "Mezouar", 2002, "1A", {16.0, 15.0, 14.5, 18.0}, {4, 3, 2, 5}, 15.8, 0},
    {8, "Mourad", "Khellaf", 2000, "2B", {12.0, 10.5, 11.0, 13.0}, {4, 3, 2, 5}, 12.1, 0},
    {9, "Lila", "Saidi", 2003, "1C", {8.0, 9.0, 7.5, 10.0}, {4, 3, 2, 5}, 8.7, 0},
    {10, "Hamid", "Djebbar", 2002, "3A", {20.0, 19.5, 18.0, 17.0}, {4, 3, 2, 5}, 18.7, 0},
    {11, "Malek", "Belarbi", 1998, "3B", {15.0, 14.0, 12.5, 13.0}, {4, 3, 2, 5}, 14.1, 1},
    {12, "Yacine", "Guerfi", 2000, "2A", {17.0, 18.0, 16.5, 19.0}, {4, 3, 2, 5}, 17.8, 0},
    {13, "Samira", "Amrani", 2001, "3A", {13.5, 12.0, 11.0, 14.0}, {4, 3, 2, 5}, 12.9, 1},
    {14, "Nadia", "Rahmouni", 1999, "1B", {9.0, 8.5, 10.0, 9.0}, {4, 3, 2, 5}, 8.9, 0},
    {15, "Lina", "Kaci", 2003, "2B", {20.0, 19.5, 18.5, 20.0}, {4, 3, 2, 5}, 19.6, 0},
    {16, "Adam", "Boukhelfa", 2002, "1C", {11.0, 10.5, 12.0, 10.0}, {4, 3, 2, 5}, 10.8, 1},
    {17, "Amina", "Saadallah", 2001, "3B", {14.0, 13.5, 14.0, 15.0}, {4, 3, 2, 5}, 14.2, 0},
    {18, "Khaled", "Hamdi", 2002, "2A", {18.0, 17.0, 16.0, 19.0}, {4, 3, 2, 5}, 17.4, 0},
    {19, "Amel", "Belhadi", 1998, "1A", {10.0, 9.5, 8.0, 11.0}, {4, 3, 2, 5}, 9.8, 0},
    {20, "Nassim", "Larbi", 2001, "2B", {12.5, 13.0, 14.0, 15.0}, {4, 3, 2, 5}, 13.5, 0},
    {21, "Salima", "Aouini", 2003, "3A", {16.5, 17.0, 15.0, 18.5}, {4, 3, 2, 5}, 17.2, 0},
    {22, "Reda", "Chikhi", 1999, "1B", {10.5, 9.0, 8.0, 12.0}, {4, 3, 2, 5}, 10.3, 1},
    {23, "Farid", "Medjahdi", 2002, "1C", {14.0, 13.5, 14.0, 15.0}, {4, 3, 2, 5}, 14.3, 0},
    {24, "Leila", "Dib", 2003, "3B", {19.0, 18.5, 17.0, 20.0}, {4, 3, 2, 5}, 18.6, 0},
    {25, "Fouad", "Nezzar", 2001, "2A", {15.0, 14.5, 13.0, 14.0}, {4, 3, 2, 5}, 14.3, 1},
    {26, "Amine", "Belaid", 1999, "2B", {10.0, 11.0, 12.0, 13.0}, {4, 3, 2, 5}, 11.9, 0},
    {27, "Wafa", "Cherrad", 2000, "1A", {18.0, 19.0, 17.5, 20.0}, {4, 3, 2, 5}, 18.7, 0},
    {28, "Imane", "Fekir", 2001, "2C", {13.0, 14.0, 12.5, 15.0}, {4, 3, 2, 5}, 13.8, 0},
    {29, "Redouane", "Khir", 2003, "3A", {17.0, 16.0, 15.5, 18.0}, {4, 3, 2, 5}, 16.9, 0},
    {30, "Hiba", "Selmani", 1998, "1B", {9.0, 8.5, 10.0, 9.5}, {4, 3, 2, 5}, 9.1, 1},
    {31, "Oussama", "Abdi", 2002, "2A", {14.0, 13.0, 12.5, 15.0}, {4, 3, 2, 5}, 13.8, 0},
    {32, "Malika", "Ziani", 1999, "3B", {19.5, 20.0, 18.5, 19.0}, {4, 3, 2, 5}, 19.1, 0},
    {33, "Tarek", "Zerouki", 2003, "1C", {11.0, 10.0, 9.5, 12.0}, {4, 3, 2, 5}, 10.9, 0},
    {34, "Soraya", "Mekki", 2002, "3A", {16.5, 17.0, 15.0, 18.0}, {4, 3, 2, 5}, 16.8, 0},
    {35, "Nadia", "Fellah", 2001, "2B", {13.5, 14.0, 15.0, 16.0}, {4, 3, 2, 5}, 14.2, 0},
    {36, "Lahcen", "Ouali", 2002, "1A", {15.0, 16.5, 14.0, 13.5}, {4, 3, 2, 5}, 14.8, 0},
    {37, "Fadi", "Chouikhi", 2000, "2C", {14.0, 15.0, 13.5, 12.5}, {4, 3, 2, 5}, 13.7, 0},
    {38, "Salim", "Boudiaf", 1999, "3B", {17.0, 18.0, 19.5, 18.5}, {4, 3, 2, 5}, 18.0, 0},
    {39, "Zahra", "Mahmoudi", 2001, "1C", {10.5, 12.0, 11.0, 9.0}, {4, 3, 2, 5}, 10.1, 0},
    {40, "Sami", "Laoui", 2002, "2B", {16.0, 15.0, 14.0, 15.5}, {4, 3, 2, 5}, 15.3, 0}
};


    FILE *file = fopen(nom_fichier, "rb+");
    if (!file) {
        printf("Error: Could not open the file %s.\n", nom_fichier);
        exit(1);
    }

    // Read the entete
    Entete entete;
    fread(&entete, sizeof(Entete), 1, file);

    Bloc bloc = {0};
    int bloc_index = 0;

    // Position the file pointer after the entete
    fseek(file, sizeof(Entete), SEEK_SET);
    // Read existing blocs into memory to find the last partially filled bloc
    if (entete.nb_blocs_utilises > 0) {
        fseek(file, (entete.nb_blocs_utilises - 1) * sizeof(Bloc), SEEK_CUR);
        fread(&bloc, sizeof(Bloc), 1, file);
        bloc_index = entete.nb_blocs_utilises - 1;
    }

    // Write the etudiants into blocs
    for (int i = 0; i < size; i++) {
        if (bloc.nb_enregistrements < MAX_STUDENTS_PER_BLOC) {
            bloc.etudiants[bloc.nb_enregistrements++] = etudiants[i];
        } else {
            // Write the full bloc and start a new one
            fseek(file, sizeof(Entete) + bloc_index * sizeof(Bloc), SEEK_SET);
            fwrite(&bloc, sizeof(Bloc), 1, file);
            bloc_index++;
            entete.nb_blocs_utilises++;
            bloc.nb_enregistrements = 0;
            bloc.etudiants[bloc.nb_enregistrements++] = etudiants[i];
        }
    }

    // Write the last bloc
    fseek(file, sizeof(Entete) + bloc_index * sizeof(Bloc), SEEK_SET);
    fwrite(&bloc, sizeof(Bloc), 1, file);
    entete.nb_blocs_utilises++;

    // Update the entete
    rewind(file);
    fwrite(&entete, sizeof(Entete), 1, file);

    fclose(file);
}


void reset(){
  initializeBinaryFile();
  writeBinaryFile();
}

