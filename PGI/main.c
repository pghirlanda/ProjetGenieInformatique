#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include "callbacks.h"

GtkBuilder* builder;

/*Dimensions*/

#define lignes 5
#define colonnes 10

/*Structures*/

typedef struct Robot {
	int x;
	int y;
} *bot;

typedef struct Ilot {
	int numero;
	int etat; /*0 = vide, 1 = rouge, 2 = vert*/
} *ilot;

/*Macro constantes*/

extern rouge, vert;

int G_idle_timeout = 0;
GtkImage* G_images[lignes][colonnes];
GtkImage* G_ilots[3][1];
GtkImage* G_robot2[3][1];

GdkPixbuf* G_pixbuf_font;
GdkPixbuf* G_pixbuf_robot;
GdkPixbuf* G_pixbuf_robot_rouge;
GdkPixbuf* G_pixbuf_robot_vert;
GdkPixbuf* G_pixbuf_ilot;
GdkPixbuf* G_pixbuf_ilot_rouge;
GdkPixbuf* G_pixbuf_ilot_vert;
GdkPixbuf* G_pixbuf_ilot_vert1;
GdkPixbuf* G_pixbuf_ilot_vert2;
GdkPixbuf* G_pixbuf_ilot_vert3;

bot robot1;
bot robot2;
ilot ilot1;
ilot ilot2;
ilot ilot3;

/*Prototypes des fonctions utilisées*/

gboolean idle_function(gpointer user_data);
void start_animation();
void stop_animation();
void redessiner_grille(GdkPixbuf* G_pixbuf_robot);
void redessiner_ilot();
void redessiner_robot2();
void aller_rouge_robot1();
void retour_rouge_robot1();
void trajet_vert();
void aller_vert_robot1();
void retour_vert_robot1();
void score_rouge();
void score_vert();
void avancer_robot2();
void reset();


int main(int argc, char* argv[]) {
	GtkWidget* window1;
	GtkTable* table;
	int x, y;
	char txt[20];

	/* Initialisation de GTK */
	gtk_init(&argc, &argv);

	/* Creation des widgets de l'interface a partir du fichier XML de Glade, grace au builder GTK */
	builder = gtk_builder_new();
	if (gtk_builder_add_from_file(builder, "PGI2.glade", NULL) == 0) {
		fprintf(stderr, "Erreur: ouverture du fichier GLADE\n");
		exit(EXIT_FAILURE);
	}

	/* Recuperation du widget de la fenetre principale */
	window1 = GTK_WIDGET(gtk_builder_get_object(builder, "window1"));

	/* Connection des fonctions de rappel avec les signaux de l'interface */
	gtk_builder_connect_signals(builder, NULL);

	/*================================================
	 * Récupération de toutes les images
	 *===============================================*/

	G_pixbuf_font = gdk_pixbuf_new_from_file("images/font.png", NULL);
	G_pixbuf_robot = gdk_pixbuf_new_from_file("images/robot.png", NULL);
	G_pixbuf_robot_rouge = gdk_pixbuf_new_from_file("images/robot_rouge.png", NULL);
	G_pixbuf_robot_vert = gdk_pixbuf_new_from_file("images/robot_vert.png", NULL);
	G_pixbuf_ilot = gdk_pixbuf_new_from_file("images/ilot.png", NULL);
	G_pixbuf_ilot_rouge = gdk_pixbuf_new_from_file("images/ilot_rouge.png", NULL);
	G_pixbuf_ilot_vert = gdk_pixbuf_new_from_file("images/ilot_vert.png", NULL);
	G_pixbuf_ilot_vert1 = gdk_pixbuf_new_from_file("images/ilot_vert1.png", NULL);
	G_pixbuf_ilot_vert2 = gdk_pixbuf_new_from_file("images/ilot_vert2.png", NULL);
	G_pixbuf_ilot_vert3 = gdk_pixbuf_new_from_file("images/ilot_vert3.png", NULL);

	/*================================================
	 * Création des ilots
	 *===============================================*/

	/*Creation des structures ilots*/
	ilot1 = (struct ilot*) malloc(sizeof(ilot));
	ilot2 = (struct ilot*) malloc(sizeof(ilot));
	ilot3 = (struct ilot*) malloc(sizeof(ilot));

	if (ilot1 == NULL) {
		perror("Erreur allocation memoire");
		exit(1);
	}
	if (ilot2 == NULL) {
		perror("Erreur allocation memoire");
		exit(1);
	}
	if (ilot3 == NULL) {
		perror("Erreur allocation memoire");
		exit(1);
	}

	/*Initialisation ilots*/
	ilot1->numero = 1;
	ilot1->etat = 0;
	ilot2->numero = 2;
	ilot2->etat = 0;
	ilot3->numero = 3;
	ilot3->etat = 0;	

	/*Creation du tableau d'ilots*/
	GtkTable* table2;
	table2 = GTK_TABLE(gtk_builder_get_object(builder, "table2"));

	for (int i = 0; i < 3; i++) {
		/* Creer un nouveau GtkImage */
		G_ilots[i][0] = (GtkImage*)gtk_image_new_from_pixbuf(G_pixbuf_ilot);

		/* Lui associer une etiquette avec son numéro */
		sprintf_s(txt, 20, "Ilot %d", i + 1);
		gtk_widget_set_tooltip_text(GTK_WIDGET(G_ilots[i][0]), txt);

		/* Le placer dans la grille d'agencement prevue a cet effet */
		gtk_table_attach_defaults(table2, G_ilots[i][0], 0, 1, i, i + 1);
	}

	/*================================================
	 * Création du robot1
	 *===============================================*/

	table = GTK_TABLE(gtk_builder_get_object(builder, "table1"));

	/* Redimensioner la grille d'agencement */
	gtk_table_resize(table, lignes, colonnes);

	for (y = 0; y < lignes; y++) {
		for (x = 0; x < colonnes; x++) {
			/* Creer un nouveau GtkImage */
			G_images[y][x] = (GtkImage*)gtk_image_new_from_pixbuf(G_pixbuf_font);

			/* Lui associer une etiquette avec ses coordonnees */
			sprintf_s(txt, 20, "( %d ; %d )", x, y);
			gtk_widget_set_tooltip_text(GTK_WIDGET(G_images[y][x]), txt);

			/* Le placer dans la grille d'agencement prevue a cet effet */
			gtk_table_attach_defaults(table, G_images[y][x], x, x + 1, y, y + 1);
		}
	}

	/*Creer le premier robot*/
	robot1 = (struct bot*) malloc(sizeof(bot));

	if (robot1 == NULL) {
		perror("Erreur allocation memoire");
		exit(1);
	}

	/*Initialiser le robot1*/
	robot1->x = 0;
	robot1->y = lignes - 1;


	/*================================================
	 * Création du robot2
	 *===============================================*/
	
	/*Création du tableau*/
	GtkTable* table3;
	table3 = GTK_TABLE(gtk_builder_get_object(builder, "table3"));

	for (int i = 0; i < 3; i++) {
		/* Creer un nouveau GtkImage */
		G_robot2[i][0] = (GtkImage*)gtk_image_new_from_pixbuf(G_pixbuf_font);

		/* Le placer dans la grille d'agencement prevue a cet effet */
		gtk_table_attach_defaults(table3, G_robot2[i][0], 0, 1, i, i + 1);
	}

	/*Initisalisation du robot2*/
	robot2 = (struct bot*) malloc(sizeof(bot));

	if (robot2 == NULL) {
		perror("Erreur allocation memoire");
		exit(1);
	}

	robot2->x = 0;
	robot2->y = 0;

	/*================================================
	 * Affichage de base
	 *===============================================*/
	redessiner_grille(G_pixbuf_robot);
	redessiner_ilot();
	redessiner_robot2();

	/* Changement de la fenetre principale a l'etat visible */
	gtk_widget_show_all(GTK_WIDGET(window1));

	/* Lancement de l'execution de la boucle GTK */
	gtk_main();
	g_object_unref(G_OBJECT(builder));

	printf("Fin du programme\n");

	/* Fin du programme */
	return EXIT_SUCCESS;
}

void redessiner_grille(GdkPixbuf* G_pixbuf_robot) {
	int x, y;

	for (y = 0; y < lignes; y++) {
		for (x = 0; x < colonnes; x++) {

			/* Effacer l'image */
			gtk_image_clear(GTK_IMAGE(G_images[y][x]));

			if (x == robot1->x && y == robot1->y) {
				gtk_image_set_from_pixbuf(GTK_IMAGE(G_images[y][x]), G_pixbuf_robot); /* Afficher le robot1 dans l'image (x,y) */
			}
			else {
				gtk_image_set_from_pixbuf(GTK_IMAGE(G_images[y][x]), G_pixbuf_font); /* Afficher le font dans l'image (x,y) */
			}
		}
	}
}

void redessiner_ilot() {
	G_pixbuf_ilot = gdk_pixbuf_new_from_file("images/ilot.png", NULL);
	G_pixbuf_ilot_rouge = gdk_pixbuf_new_from_file("images/ilot_rouge.png", NULL);
	G_pixbuf_ilot_vert = gdk_pixbuf_new_from_file("images/ilot_vert.png", NULL);
	G_pixbuf_ilot_vert1 = gdk_pixbuf_new_from_file("images/ilot_vert1.png", NULL);
	G_pixbuf_ilot_vert2 = gdk_pixbuf_new_from_file("images/ilot_vert2.png", NULL);
	G_pixbuf_ilot_vert3 = gdk_pixbuf_new_from_file("images/ilot_vert3.png", NULL);

	/*Les ilots sont vides*/
	if (ilot1->etat == 0){
		gtk_image_clear(GTK_IMAGE(G_ilots[0][0]));
		gtk_image_set_from_pixbuf(GTK_IMAGE(G_ilots[0][0]), G_pixbuf_ilot);
	}
	if (ilot2->etat == 0) {
		gtk_image_clear(GTK_IMAGE(G_ilots[1][0]));
		gtk_image_set_from_pixbuf(GTK_IMAGE(G_ilots[1][0]), G_pixbuf_ilot);
	}
	if (ilot3->etat == 0) {
		gtk_image_clear(GTK_IMAGE(G_ilots[2][0]));
		gtk_image_set_from_pixbuf(GTK_IMAGE(G_ilots[2][0]), G_pixbuf_ilot);
	}

	/*Les ilots sont rouges*/
	if (ilot1->etat == 1) {
		gtk_image_clear(GTK_IMAGE(G_ilots[0][0]));
		gtk_image_set_from_pixbuf(GTK_IMAGE(G_ilots[0][0]), G_pixbuf_ilot_rouge);
	}
	if (ilot2->etat == 1) {
		gtk_image_clear(GTK_IMAGE(G_ilots[1][0]));
		gtk_image_set_from_pixbuf(GTK_IMAGE(G_ilots[1][0]), G_pixbuf_ilot_rouge);
	}
	if (ilot3->etat == 1) {
		gtk_image_clear(GTK_IMAGE(G_ilots[2][0]));
		gtk_image_set_from_pixbuf(GTK_IMAGE(G_ilots[2][0]), G_pixbuf_ilot_rouge);
	}

	/*Les ilots sont vert*/
	if (ilot1->etat == 2) {
		gtk_image_clear(GTK_IMAGE(G_ilots[0][0]));
		gtk_image_set_from_pixbuf(GTK_IMAGE(G_ilots[0][0]), G_pixbuf_ilot_vert1);

		gtk_image_clear(GTK_IMAGE(G_ilots[0][0]));
		gtk_image_set_from_pixbuf(GTK_IMAGE(G_ilots[0][0]), G_pixbuf_ilot_vert2);

		gtk_image_clear(GTK_IMAGE(G_ilots[0][0]));
		gtk_image_set_from_pixbuf(GTK_IMAGE(G_ilots[0][0]), G_pixbuf_ilot_vert3);

		gtk_image_clear(GTK_IMAGE(G_ilots[0][0]));
		gtk_image_set_from_pixbuf(GTK_IMAGE(G_ilots[0][0]), G_pixbuf_ilot_vert);
	}
	if (ilot2->etat == 2) {
		gtk_image_clear(GTK_IMAGE(G_ilots[1][0]));
		gtk_image_set_from_pixbuf(GTK_IMAGE(G_ilots[1][0]), G_pixbuf_ilot_vert1);

		gtk_image_clear(GTK_IMAGE(G_ilots[1][0]));
		gtk_image_set_from_pixbuf(GTK_IMAGE(G_ilots[1][0]), G_pixbuf_ilot_vert2);

		gtk_image_clear(GTK_IMAGE(G_ilots[1][0]));
		gtk_image_set_from_pixbuf(GTK_IMAGE(G_ilots[1][0]), G_pixbuf_ilot_vert3);

		gtk_image_clear(GTK_IMAGE(G_ilots[1][0]));
		gtk_image_set_from_pixbuf(GTK_IMAGE(G_ilots[1][0]), G_pixbuf_ilot_vert);
	}
	if (ilot3->etat == 2) {
		gtk_image_clear(GTK_IMAGE(G_ilots[2][0]));
		gtk_image_set_from_pixbuf(GTK_IMAGE(G_ilots[2][0]), G_pixbuf_ilot_vert1);

		gtk_image_clear(GTK_IMAGE(G_ilots[2][0]));
		gtk_image_set_from_pixbuf(GTK_IMAGE(G_ilots[2][0]), G_pixbuf_ilot_vert2);

		gtk_image_clear(GTK_IMAGE(G_ilots[2][0]));
		gtk_image_set_from_pixbuf(GTK_IMAGE(G_ilots[2][0]), G_pixbuf_ilot_vert3);

		gtk_image_clear(GTK_IMAGE(G_ilots[2][0]));
		gtk_image_set_from_pixbuf(GTK_IMAGE(G_ilots[2][0]), G_pixbuf_ilot_vert);
	}
	redessiner_grille(G_pixbuf_robot);
}

void redessiner_robot2() {
	G_pixbuf_robot = gdk_pixbuf_new_from_file("images/robot.png", NULL);

	gtk_image_clear(GTK_IMAGE(G_robot2[robot2->x][robot2->y]));
	gtk_image_set_from_pixbuf(GTK_IMAGE(G_robot2[robot2->x][robot2->y]), G_pixbuf_robot);
}

void aller_rouge_robot1() {

	if (ilot1->etat == 0) {
		if (robot1->y > 0) {
			robot1->y += -1;
		}
		if (robot1->x < colonnes) {
			robot1->x += 1;
		}
	}
	else if (ilot2->etat == 0) {
		if (robot1->y > lignes/2) {
			robot1->y += -1;
		}
		if (robot1->x < colonnes) {
			robot1->x += 1;
		}
	}
	else if (ilot3->etat == 0 && robot1->x < colonnes) {
		robot1->x += 1;
	}

	G_pixbuf_robot_rouge = gdk_pixbuf_new_from_file("images/robot_rouge.png", NULL);
	redessiner_grille(G_pixbuf_robot_rouge);
}

void aller_vert_robot1() {

	if (ilot1->etat == 2) {
		if (robot1->y > 0) {
			robot1->y += -1;
		}
		if (robot1->x < colonnes) {
			robot1->x += 1;
		}
	}
	else if (ilot2->etat == 2) {
		if (robot1->y > lignes / 2) {
			robot1->y += -1;
		}
		if (robot1->x < colonnes) {
			robot1->x += 1;
		}
	}
	else if ((ilot3->etat == 2) && (robot1->x < colonnes)) {
		robot1->x += 1;
	}

	G_pixbuf_robot = gdk_pixbuf_new_from_file("images/robot.png", NULL);
	redessiner_grille(G_pixbuf_robot);
}

void retour_rouge_robot1() {
	if (ilot1->etat == 1){
		if (robot1->x > 0) {
			robot1->x += -1;
		}
		if (robot1->y < lignes - 1) {
			robot1->y += 1;
		}
	}

	else if (ilot2->etat == 1) {
		if (robot1->x > 0) {
			robot1->x += -1;
		}
		if (robot1->y < lignes - 1) {
			robot1->y += 1;
		}
	}

	else if (ilot3->etat == 1 && robot1->x > 0) {
		robot1->x += -1;
	}

	redessiner_grille(G_pixbuf_robot);
}

void retour_vert_robot1() {
	if (ilot1->etat == 0 && robot1->x > 0) {
		robot1->x += -1;
	}

	else if (ilot2->etat == 0) {
		if (robot1->x > 0) {
			robot1->x += -1;
		}
		if (robot1->y > 0) {
			robot1->y += -1;
		}
	}

	else if (ilot3->etat == 0) {
		if (robot1->x > 0) {
			robot1->x += -1;
		}
		if (robot1->y > 0) {
			robot1->y += -1;
		}
	}

	G_pixbuf_robot_vert = gdk_pixbuf_new_from_file("images/robot_vert.png", NULL);
	redessiner_grille(G_pixbuf_robot_vert);
}

void score_rouge() {
	char msg_rouge[128];
	GtkLabel* label_rouge = GTK_LABEL(gtk_builder_get_object(builder, "label_rouge"));

	if (robot1->x == 0 && robot1->y == lignes - 1) {
		rouge -= 1;
		sprintf_s(msg_rouge, 128, " %d ", rouge);
		gtk_label_set_text(label_rouge, msg_rouge);
	}
}

void score_vert() {
	char msg_vert[128];
	GtkLabel* label_vert = GTK_LABEL(gtk_builder_get_object(builder, "label_vert"));

	if (robot1->x == 0 && robot1->y == 0) {
		vert += 1;
		sprintf_s(msg_vert, 128, " %d ", vert);
		gtk_label_set_text(label_vert, msg_vert);
	}
}

void avancer_robot2() {
	G_pixbuf_font = gdk_pixbuf_new_from_file("images/font.png", NULL);

	if (robot2->x == 0 && robot2->y == 0 && ilot1->etat == 1) {
		redessiner_robot2();
		ilot1->etat = 2;
		redessiner_ilot();

		gtk_image_clear(GTK_IMAGE(G_robot2[robot2->x][robot2->y]));
		gtk_image_set_from_pixbuf(GTK_IMAGE(G_robot2[robot2->x][robot2->y]), G_pixbuf_font);

		robot2->y = 1;
	}
	if (robot2->x == 0 && robot2->y == 1 && ilot2->etat == 1) {
		redessiner_robot2();
		ilot2->etat = 2;
		redessiner_ilot();

		gtk_image_clear(GTK_IMAGE(G_robot2[robot2->x][robot2->y]));
		gtk_image_set_from_pixbuf(GTK_IMAGE(G_robot2[robot2->x][robot2->y]), G_pixbuf_font);

		robot2->y = 2;
	}
	if (robot2->x == 0 && robot2->y == 2 && ilot3->etat == 1) {
		redessiner_robot2();
		ilot3->etat = 2;
		redessiner_ilot();

		gtk_image_clear(GTK_IMAGE(G_robot2[robot2->x][robot2->y]));
		gtk_image_set_from_pixbuf(GTK_IMAGE(G_robot2[robot2->x][robot2->y]), G_pixbuf_font);

		robot2->y = 0;
	}
}

gboolean idle_function(gpointer user_data) {
	/*================================================
	 * Lorsque les ilots sont vides
	 *===============================================*/

	if (ilot1->etat == 0 && rouge > 0) {
		/*Le robot1 prends un carré rouge*/
		G_pixbuf_robot_rouge = gdk_pixbuf_new_from_file("images/robot_rouge.png", NULL);
		redessiner_grille(G_pixbuf_robot_rouge);
		score_rouge();

		/*Le robot1 va jusqu'à l'ilot vide*/
		while (robot1->x != 9 || robot1->y != 0) {
			redessiner_grille(G_pixbuf_robot_rouge);
			aller_rouge_robot1();
		}

		/*Dépôt du carré rouge sur l'ilot*/
		ilot1->etat = 1;
		redessiner_ilot();

		/*Retour au point de départ*/
		while ((robot1->x != 0) || (robot1->y != 4)) {
			retour_rouge_robot1();
		}
	}

	else if (ilot2->etat == 0 && rouge > 0) {
		/*Le robot1 prend un carré rouge*/
		G_pixbuf_robot_rouge = gdk_pixbuf_new_from_file("images/robot_rouge.png", NULL);
		redessiner_grille(G_pixbuf_robot_rouge);
		score_rouge();

		/*Le robot va jusqu'à l'ilot vide*/
		while (robot1->x != 9 || robot1->y != 2) {
			redessiner_grille(G_pixbuf_robot_rouge);
			aller_rouge_robot1();
		}
		
		/*Dépôt du carré rouge sur l'ilot*/
		ilot2->etat = 1;
		redessiner_ilot();

		/*Retour au point de départ*/
		while ((robot1->x != 0) || (robot1->y != 4)) {
			retour_rouge_robot1();
		}
	}

	else if (ilot3->etat == 0 && rouge > 0) {
		/*Le robot1 prend un carré rouge*/
		G_pixbuf_robot_rouge = gdk_pixbuf_new_from_file("images/robot_rouge.png", NULL);
		redessiner_grille(G_pixbuf_robot_rouge);
		score_rouge();

		/*Le robot va jusqu'à l'ilot vide*/
		while (robot1->x != 9 || robot1->y != 4) {
			redessiner_grille(G_pixbuf_robot_rouge);
			aller_rouge_robot1();
		}

		/*Dépôt du carré rouge sur l'ilot*/
		ilot3->etat = 1;
		redessiner_ilot();

		/*Retour au point de départ*/
		while ((robot1->x != 0) || (robot1->y != 4)) {
			retour_rouge_robot1();
		}
	}

	/*================================================
	 * Lorsque les ilots sont pleins
	 *===============================================*/
	
	if (ilot1->etat == 2 && ilot2->etat == 2 && ilot3->etat == 2) {
		trajet_vert();
	}

	/*================================================
	 * Déplacement du robot2
	 *===============================================*/
	
	avancer_robot2();
	redessiner_robot2();

	/*================================================
	 * Récupération du dernier carré vert
	 *===============================================*/

	if (rouge == 0) {
		trajet_vert();
	}
		

	return TRUE; /* should return FALSE if the source should be removed */
}

void trajet_vert() {
	while (ilot1->etat != 0 || ilot2->etat != 0 || ilot3->etat != 0) {
		if (ilot1->etat == 2) {
			/*Le robot1 va jusqu'à l'ilot vert*/
			while (robot1->x != 9 || robot1->y != 0) {
				redessiner_grille(G_pixbuf_robot);
				aller_vert_robot1();
			}

			/*Le robot1 prends le carré vert*/
			G_pixbuf_robot_vert = gdk_pixbuf_new_from_file("images/robot_vert.png", NULL);
			ilot1->etat = 0;
			redessiner_ilot();
			redessiner_grille(G_pixbuf_robot_vert);

			/*Le robot1 va deposer le carré vert*/
			while (robot1->x != 0 || robot1->y != 0) {
				retour_vert_robot1();
			}
			score_vert();
			redessiner_grille(G_pixbuf_robot);
			
			/*Le robot1 retourne à son point de départ*/
			while (robot1->x != 0 || robot1->y != 4) {
				robot1->y += 1;
				redessiner_grille(G_pixbuf_robot);
			}
		}
		else if (ilot2->etat == 2) {
			/*Le robot1 va jusqu'à l'ilot vert*/
			while (robot1->x != 9 || robot1->y != 2) {
				redessiner_grille(G_pixbuf_robot);
				aller_vert_robot1();
			}

			/*Le robot1 prends le carré vert*/
			G_pixbuf_robot_vert = gdk_pixbuf_new_from_file("images/robot_vert.png", NULL);
			redessiner_grille(G_pixbuf_robot_vert);
			ilot2->etat = 0;
			redessiner_ilot();

			/*Le robot1 va deposer le carré vert*/
			while (robot1->x != 0 || robot1->y != 0) {
				retour_vert_robot1();
			}
			score_vert();
			redessiner_grille(G_pixbuf_robot);

			/*Le robot1 retourne à son point de départ*/
			while (robot1->x != 0 || robot1->y != 4) {
				robot1->y += 1;
				redessiner_grille(G_pixbuf_robot);
			}
		}
		else if (ilot3->etat == 2) {
			/*Le robot1 va jusqu'à l'ilot vert*/
			while (robot1->x != 9 || robot1->y != 4) {
				redessiner_grille(G_pixbuf_robot);
				aller_vert_robot1();
			}

			/*Le robot1 prends le carré vert*/
			G_pixbuf_robot_vert = gdk_pixbuf_new_from_file("images/robot_vert.png", NULL);
			redessiner_grille(G_pixbuf_robot_vert);
			ilot3->etat = 0;
			redessiner_ilot();

			/*Le robot1 va deposer le carré vert*/
			while (robot1->x != 0 || robot1->y != 0) {
				retour_vert_robot1();
			}
			score_vert();
			redessiner_grille(G_pixbuf_robot);

			/*Le robot1 retourne à son point de départ*/
			while (robot1->x != 0 || robot1->y != 4) {
				robot1->y += 1;
				redessiner_grille(G_pixbuf_robot);
			}
		}
	}
}

void start_animation() {
	if (G_idle_timeout == 0) {
		/* Demander que la fonction idle_function() soit appellee toutes les 1s */
		G_idle_timeout = g_timeout_add(1000, (GSourceFunc)idle_function, NULL);
	}
}

void stop_animation() {
	if (G_idle_timeout > 0) {
		/* Demander de ne plus appeller la fonction idle_function() toutes les 1s */
		g_source_remove(G_idle_timeout);
		G_idle_timeout = 0;
	}
}

void reset() {
	/*Suppression de là où était les robots*/
	gtk_image_clear(GTK_IMAGE(G_images[robot1->y][robot1->x]));
	gtk_image_clear(GTK_IMAGE(G_robot2[robot2->y][robot2->x]));

	/*Reset des ilots*/
	ilot1->etat = 0;
	ilot2->etat = 0;
	ilot3->etat = 0;

	/*Reset des robots*/
	robot1->x = 0;
	robot1->y = lignes - 1;
	robot2->x = 0;
	robot2->y = 0;

	redessiner_grille(G_pixbuf_robot);
	redessiner_ilot();
	redessiner_robot2();
}