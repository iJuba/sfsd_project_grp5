#include <gtk/gtk.h>
#include "student.h"




void afficher_etudiants(GtkWidget *widget, gpointer data) {
    GtkListStore *list_store = GTK_LIST_STORE(data);
    gtk_list_store_clear(list_store); // Vider le modèle avant d'ajouter de nouvelles données
    afficher_contenu_fichier_binaire(list_store);
}

// Function to display messages in a popup dialog
void show_message(GtkWidget *parent, const gchar *message, GtkMessageType type) {
    GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(parent), GTK_DIALOG_MODAL, type, GTK_BUTTONS_OK, "%s", message);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

// Callback to handle adding a student from the form
void ajouter_etudiant_from_form(GtkWidget *widget, gpointer data) {
    GtkWidget **entries = (GtkWidget **)data;

    const gchar *num_inscription = gtk_entry_get_text(GTK_ENTRY(entries[0]));
    const gchar *nom = gtk_entry_get_text(GTK_ENTRY(entries[1]));
    const gchar *prenom = gtk_entry_get_text(GTK_ENTRY(entries[2]));
    const gchar *annee_naissance = gtk_entry_get_text(GTK_ENTRY(entries[3]));
    const gchar *classe = gtk_entry_get_text(GTK_ENTRY(entries[4]));

    // Perform validation and save logic here
    if (strlen(num_inscription) == 0 || strlen(nom) == 0 || strlen(prenom) == 0) {
        show_message(NULL, "Veuillez remplir tous les champs requis.", GTK_MESSAGE_ERROR);
        return;
    }

    // Simulate saving student and show success message
    show_message(NULL, "Étudiant ajouté avec succès.", GTK_MESSAGE_INFO);
}

// Callback to handle searching for a student
void find_etudiant(GtkWidget *widget, gpointer data) {
    GtkWidget **entries = (GtkWidget **)data;

    const gchar *num_inscription = gtk_entry_get_text(GTK_ENTRY(entries[0]));
    if (strlen(num_inscription) == 0) {
        show_message(NULL, "Veuillez entrer un numéro d'inscription.", GTK_MESSAGE_ERROR);
        return;
    }

    int Trouv, i, j;
    int numero_inscription_recherche = atoi(num_inscription);
    recherche_dichotomique(numero_inscription_recherche, &Trouv, &i, &j);

    if (Trouv) {
        FILE *fichier = fopen(nom_fichier, "rb");
        if (fichier == NULL) {
            show_message(NULL, "Erreur lors de l'ouverture du fichier.", GTK_MESSAGE_ERROR);
            return;
        }

        fseek(fichier, sizeof(Entete) + (i - 1) * sizeof(Bloc), SEEK_SET);
        Bloc buf;
        fread(&buf, sizeof(Bloc), 1, fichier);

        Etudiant etudiant = buf.etudiants[j];

        gchar *result_message = g_strdup_printf(
            "Étudiant trouvé:\nNuméro d'inscription: %d\nNom: %s\nPrénom: %s\nAnnée de naissance: %d\nClasse: %s\nMoyenne: %.2f",
            etudiant.numero_inscription,
            etudiant.nom,
            etudiant.prenom,
            etudiant.annee_naissance,
            etudiant.classe,
            etudiant.moyenne
        );

        show_message(NULL, result_message, GTK_MESSAGE_INFO);
        g_free(result_message);

        fclose(fichier);
    } else {
        show_message(NULL, "Étudiant non trouvé.", GTK_MESSAGE_INFO);
    }
}

int main(int argc, char *argv[]) {
    GtkWidget *window;
    GtkWidget *vbox;
    GtkWidget *menu_bar;
    GtkWidget *notebook;
    GtkWidget *status_bar;

    // Initialize GTK
    gtk_init(&argc, &argv);

    // Create the main window
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Gestion des Étudiants");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    void afficher_etudiants(GtkWidget *widget, gpointer data) {
    GtkListStore *list_store = GTK_LIST_STORE(data);
    afficher_contenu_fichier_binaire(list_store);
}

    // Create a vertical box layout
    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // Create a menu bar
    menu_bar = gtk_menu_bar_new();
    GtkWidget *menu_file = gtk_menu_item_new_with_label("Fichier");
    GtkWidget *menu_help = gtk_menu_item_new_with_label("Aide");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_file);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_help);
    gtk_box_pack_start(GTK_BOX(vbox), menu_bar, FALSE, FALSE, 0);

    // Create a notebook
    notebook = gtk_notebook_new();
    gtk_box_pack_start(GTK_BOX(vbox), notebook, TRUE, TRUE, 0);

    // Tab 1: Ajouter Étudiant
    GtkWidget *tab_ajouter = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);

    // Fields for adding a student
    GtkWidget *entry_num = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_num), "Numéro d'inscription");
    GtkWidget *entry_nom = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_nom), "Nom");
    GtkWidget *entry_prenom = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_prenom), "Prénom");
    GtkWidget *entry_annee = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_annee), "Année de naissance");
    GtkWidget *entry_classe = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_classe), "Classe");

    GtkWidget *btn_valider = gtk_button_new_with_label("Valider");
    GtkWidget *icon_valider = gtk_image_new_from_icon_name("dialog-ok", GTK_ICON_SIZE_BUTTON);
    gtk_button_set_image(GTK_BUTTON(btn_valider), icon_valider);

    // Add widgets to the tab
    gtk_box_pack_start(GTK_BOX(tab_ajouter), entry_num, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(tab_ajouter), entry_nom, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(tab_ajouter), entry_prenom, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(tab_ajouter), entry_annee, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(tab_ajouter), entry_classe, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(tab_ajouter), btn_valider, FALSE, FALSE, 10);

    GtkWidget *tab_label_ajouter = gtk_label_new("Ajouter");
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), tab_ajouter, tab_label_ajouter);

    // Connect the validation button
    GtkWidget *form_entries[] = {entry_num, entry_nom, entry_prenom, entry_annee, entry_classe};
    g_signal_connect(btn_valider, "clicked", G_CALLBACK(ajouter_etudiant_from_form), form_entries);

    // Tab 2: Rechercher Étudiant
    GtkWidget *tab_rechercher = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    GtkWidget *entry_search = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_search), "Numéro d'inscription à rechercher");
    GtkWidget *btn_rechercher = gtk_button_new_with_label("Rechercher Étudiant");
    GtkWidget *icon_rechercher = gtk_image_new_from_icon_name("system-search", GTK_ICON_SIZE_BUTTON);
    gtk_button_set_image(GTK_BUTTON(btn_rechercher), icon_rechercher);
    gtk_widget_set_margin_top(entry_search, 10);
    gtk_widget_set_margin_start(entry_search, 10);
    gtk_widget_set_margin_top(btn_rechercher, 10);
    gtk_widget_set_margin_start(btn_rechercher, 10);
    gtk_box_pack_start(GTK_BOX(tab_rechercher), entry_search, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(tab_rechercher), btn_rechercher, FALSE, FALSE, 0);
    GtkWidget *tab_label_rechercher = gtk_label_new("Rechercher");
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), tab_rechercher, tab_label_rechercher);

    GtkWidget *search_entries[] = {entry_search};
    g_signal_connect(btn_rechercher, "clicked", G_CALLBACK(find_etudiant), search_entries);

   // Tab 3: Liste des Étudiants
    GtkWidget *tab_liste = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);

    // Création du TreeView
    GtkWidget *tree_view = gtk_tree_view_new();
    gtk_box_pack_start(GTK_BOX(tab_liste), tree_view, TRUE, TRUE, 0);

    // Création du modèle (ListStore)
    GtkListStore *list_store = gtk_list_store_new(6, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT, G_TYPE_STRING, G_TYPE_DOUBLE);
    gtk_tree_view_set_model(GTK_TREE_VIEW(tree_view), GTK_TREE_MODEL(list_store));
    g_object_unref(list_store); // Libérer la référence supplémentaire

    // Ajouter des colonnes au TreeView
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();

    GtkTreeViewColumn *col_num = gtk_tree_view_column_new_with_attributes("Numéro", renderer, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), col_num);

    GtkTreeViewColumn *col_nom = gtk_tree_view_column_new_with_attributes("Nom", renderer, "text", 1, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), col_nom);

    GtkTreeViewColumn *col_prenom = gtk_tree_view_column_new_with_attributes("Prénom", renderer, "text", 2, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), col_prenom);

    GtkTreeViewColumn *col_annee = gtk_tree_view_column_new_with_attributes("Année", renderer, "text", 3, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), col_annee);

    GtkTreeViewColumn *col_classe = gtk_tree_view_column_new_with_attributes("Classe", renderer, "text", 4, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), col_classe);

    GtkTreeViewColumn *col_moyenne = gtk_tree_view_column_new_with_attributes("Moyenne", renderer, "text", 5, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), col_moyenne);

    // Bouton pour afficher les étudiants
    GtkWidget *btn_afficher = gtk_button_new_with_label("Afficher Étudiants");
    gtk_box_pack_start(GTK_BOX(tab_liste), btn_afficher, FALSE, FALSE, 5);

    // Connecter le bouton à la fonction de rappel
    g_signal_connect(btn_afficher, "clicked", G_CALLBACK(afficher_etudiants), list_store);

    // Ajouter l'onglet au Notebook
    GtkWidget *tab_label_liste = gtk_label_new("Liste");
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), tab_liste, tab_label_liste);



    // Tab 4: Réorganiser et Réinitialiser
    GtkWidget *tab_tools = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    GtkWidget *btn_reorganiser = gtk_button_new_with_label("Réorganiser");
    GtkWidget *icon_reorganiser = gtk_image_new_from_icon_name("view-refresh", GTK_ICON_SIZE_BUTTON);
    gtk_button_set_image(GTK_BUTTON(btn_reorganiser), icon_reorganiser);
    GtkWidget *btn_reset = gtk_button_new_with_label("Réinitialiser");
    GtkWidget *icon_reset = gtk_image_new_from_icon_name("edit-clear", GTK_ICON_SIZE_BUTTON);
    gtk_button_set_image(GTK_BUTTON(btn_reset), icon_reset);
    gtk_widget_set_margin_top(btn_reorganiser, 10);
    gtk_widget_set_margin_start(btn_reorganiser, 10);
    gtk_widget_set_margin_top(btn_reset, 10);
    gtk_widget_set_margin_start(btn_reset, 10);
    gtk_box_pack_start(GTK_BOX(tab_tools), btn_reorganiser, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(tab_tools), btn_reset, FALSE, FALSE, 0);
    GtkWidget *tab_label_tools = gtk_label_new("Outils");
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), tab_tools, tab_label_tools);
    g_signal_connect(btn_reorganiser, "clicked", G_CALLBACK(Reorganisation), NULL);
    g_signal_connect(btn_reset, "clicked", G_CALLBACK(reset), NULL);

    // Status bar
    status_bar = gtk_statusbar_new();
    gtk_box_pack_end(GTK_BOX(vbox), status_bar, FALSE, FALSE, 0);

    // Show all widgets
    gtk_widget_show_all(window);

    gtk_main();
    return 0;}
