#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "callbacks.h"

GtkBuilder* builder;
int rouge, vert = 0;

void on_window1_destroy(GtkObject* object, gpointer user_data) {
	printf("Fermeture de la fenetre.\n");
	stop_animation();
	gtk_main_quit();
}

void init_spin_button(GtkSpinButton* spb, double min_val, double max_val, double val, double step, int digits)
{
	gtk_spin_button_set_digits(spb, digits);
	gtk_spin_button_set_range(spb, min_val, max_val);
	gtk_spin_button_set_value(spb, val);
	gtk_spin_button_set_increments(spb, step, step);
}

void on_window1_realize(GtkObject* object, gpointer user_data) {
	GtkSpinButton* nb_objet_assembler = GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "nb_objet_assembler"));

	FILE* desc = NULL;
	errno_t err;
	err = fopen_s(&desc, "nb_objet.txt", "r");
	if (err != 0) {
		perror("Erreur ouverture fichier en ecriture");
	}

	/*Initialisation du spin button*/
	fscanf_s(desc, "%d", &rouge);
	init_spin_button(nb_objet_assembler, 0, 1000, rouge, 1, 0);

	/*Affichage des chiffres vert et rouge*/
	char msg_rouge[128], msg_vert[128];
	vert = 0;

	GtkLabel* label_vert = GTK_LABEL(gtk_builder_get_object(builder, "label_vert"));
	GtkLabel* label_rouge = GTK_LABEL(gtk_builder_get_object(builder, "label_rouge"));

	sprintf_s(msg_rouge, 128, " %d ", rouge);
	gtk_label_set_text(label_rouge, msg_rouge);
	sprintf_s(msg_vert, 128, " %d ", vert);
	gtk_label_set_text(label_vert, msg_vert);

	fclose(desc);
}

void on_button_actualiser_clicked(GtkObject* object, gpointer user_data) {
	GtkSpinButton* nb_objet_assembler = GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "nb_objet_assembler"));
	rouge = gtk_spin_button_get_value(nb_objet_assembler);

	/*Ecriture dans un fichier du nombre d'objets ? assembler*/
	FILE* desc = NULL;
	errno_t err;
	err = fopen_s(&desc, "nb_objet.txt", "w");
	if (err != 0) {
		perror("Erreur ouverture fichier en ecriture");
	}
	else {
		fprintf_s(desc, "%d", rouge);
		fclose(desc);
	}

	/*Affichage dans le carr? rouge*/
	char msg_rouge[128];
	GtkLabel* label_rouge = GTK_LABEL(gtk_builder_get_object(builder, "label_rouge"));

	sprintf_s(msg_rouge, 128, " %d ", rouge);
	gtk_label_set_text(label_rouge, msg_rouge);
}

void on_button_start_clicked(GtkObject* object, gpointer user_data) {
	start_animation();
}

void on_button_stop_clicked(GtkObject* object, gpointer user_data) {
	stop_animation();
}

void on_button_reset_clicked(GtkObject* object, gpointer user_data) {
	on_window1_realize(object, user_data);
	vert = 0;
	reset();
}