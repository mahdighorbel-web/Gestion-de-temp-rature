#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// ============ STRUCTURES ============
typedef struct {
    float seuil_min;
    float seuil_max;
    int intervalle_mesure;
} Configuration;

typedef struct {
    float temperature;
    char timestamp[30];
} Mesure;

typedef struct {
    int niveau1;
    int niveau2;
    int niveau3;
} Statistiques;

// ============ VARIABLES GLOBALES ============
Configuration config;
Statistiques stats = {0, 0, 0};
int alertes_consecutives = 0;

// ============ TÂCHE 1 : GESTION DE LA CONFIGURATION ============
void enregistrer_configuration(float min, float max, int intervalle) {
    config.seuil_min = min;
    config.seuil_max = max;
    config.intervalle_mesure = intervalle;

    FILE *fichier = fopen("config.txt", "w");
    if (fichier == NULL) {
        printf("Erreur : Impossible de créer config.txt\n");
        return;
    }

    fprintf(fichier, "seuil_min=%.2f\n", config.seuil_min);
    fprintf(fichier, "seuil_max=%.2f\n", config.seuil_max);
    fprintf(fichier, "intervalle_mesure=%d\n", config.intervalle_mesure);

    fclose(fichier);
    printf("OK Configuration enregistree dans config.txt\n");
    printf("  Min: %.2f C | Max: %.2f C | Intervalle: %ds\n\n", min, max, intervalle);
}

void lire_configuration() {
    FILE *fichier = fopen("config.txt", "r");
    if (fichier == NULL) {
        printf("Erreur : Fichier config.txt introuvable\n");
        return;
    }

    fscanf(fichier, "seuil_min=%f\n", &config.seuil_min);
    fscanf(fichier, "seuil_max=%f\n", &config.seuil_max);
    fscanf(fichier, "intervalle_mesure=%d\n", &config.intervalle_mesure);

    fclose(fichier);
    printf("OK Configuration chargee depuis config.txt\n");
    printf("  Min: %.2f C | Max: %.2f C | Intervalle: %ds\n\n",
           config.seuil_min, config.seuil_max, config.intervalle_mesure);
}

// ============ TÂCHE 2 : SIMULATION DE CAPTEUR ============
float capturer_temperature() {
    // Simule une lecture de capteur avec valeur aléatoire entre 10°C et 35°C
    return 10.0 + ((float)rand() / RAND_MAX) * 25.0;
}

// Pour les tests : retourne une valeur fixe
float capturer_temperature_fixe(float valeur) {
    return valeur;
}

// ============ TÂCHE 3 : MOTEUR DE DÉCISION ============
void analyser_temperature(float temperature) {
    printf("Temperature mesuree: %.2f C\n", temperature);

    if (temperature < config.seuil_min) {
        float ecart = config.seuil_min - temperature;
        printf("ALERTE: Temperature TROP BASSE (ecart: %.2f C)\n", ecart);

        // Gestion des alertes avancées
        alertes_consecutives++;
        if (ecart <= 2) {
            printf("   -> Niveau 1: Avertissement simple\n");
            stats.niveau1++;
        } else if (ecart <= 5) {
            printf("   -> Niveau 2: Alerte moderee\n");
            stats.niveau2++;
        } else {
            printf("   -> Niveau 3: ALERTE CRITIQUE!\n");
            stats.niveau3++;
        }
    }
    else if (temperature > config.seuil_max) {
        float ecart = temperature - config.seuil_max;
        printf("ALERTE: Temperature TROP ELEVEE (ecart: %.2f C)\n", ecart);

        // Gestion des alertes avancées
        alertes_consecutives++;
        if (ecart <= 2) {
            printf("   -> Niveau 1: Avertissement simple\n");
            stats.niveau1++;
        } else if (ecart <= 5) {
            printf("   -> Niveau 2: Alerte moderee\n");
            stats.niveau2++;
        } else {
            printf("   -> Niveau 3: ALERTE CRITIQUE!\n");
            stats.niveau3++;
        }
    }
    else {
        printf("OK Temperature NORMALE\n");
        alertes_consecutives = 0;
    }

    // Alerte pour trop d'alertes consécutives
    if (alertes_consecutives >= 3) {
        printf("ATTENTION: %d alertes consecutives detectees!\n", alertes_consecutives);
    }

    printf("\n");
}

// ============ TÂCHE 4 : JOURNALISATION ============
void enregistrer_mesure(float temperature) {
    FILE *fichier = fopen("journalisation.txt", "a");
    if (fichier == NULL) {
        printf("Erreur : Impossible d'ouvrir journalisation.txt\n");
        return;
    }

    time_t maintenant;
    time(&maintenant);
    char timestamp[30];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&maintenant));

    fprintf(fichier, "[%s] Temperature: %.2f C\n", timestamp, temperature);
    fclose(fichier);
}

void enregistrer_evenement(const char *message) {
    FILE *fichier = fopen("journalisation.txt", "a");
    if (fichier == NULL) {
        printf("Erreur : Impossible d'ouvrir journalisation.txt\n");
        return;
    }

    time_t maintenant;
    time(&maintenant);
    char timestamp[30];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&maintenant));

    fprintf(fichier, "[%s] %s\n", timestamp, message);
    fclose(fichier);
}

// ============ TÂCHE 6 : MODULE DE STATISTIQUES ============
void calculer_statistiques() {
    FILE *fichier = fopen("journalisation.txt", "r");
    if (fichier == NULL) {
        printf("Aucune donnée disponible\n");
        return;
    }

    float temp, somme = 0, min = 100, max = -100;
    int count = 0;
    char ligne[100];

    while (fgets(ligne, sizeof(ligne), fichier)) {
        // Cherche "Temperature:" dans la ligne
        char *ptr = strstr(ligne, "Temperature:");
        if (ptr != NULL) {
            // Extrait la température après "Temperature:"
            if (sscanf(ptr, "Temperature: %f", &temp) == 1) {
                somme += temp;
                if (temp < min) min = temp;
                if (temp > max) max = temp;
                count++;
            }
        }
    }
    fclose(fichier);

    if (count == 0) {
        printf("Aucune mesure disponible\n");
        return;
    }

    printf("\n========== RAPPORT DE STATISTIQUES ==========\n");
    printf("Periode d'analyse: Toutes les mesures\n");
    printf("Nombre total de mesures: %d\n", count);
    printf("Temperature moyenne: %.2f C\n", somme / count);
    printf("Temperature minimale: %.2f C\n", min);
    printf("Temperature maximale: %.2f C\n", max);
    printf("\n--- Alertes par niveau ---\n");
    printf("Niveau 1 (Avertissement): %d alertes\n", stats.niveau1);
    printf("Niveau 2 (Moderee): %d alertes\n", stats.niveau2);
    printf("Niveau 3 (Critique): %d alertes\n", stats.niveau3);
    printf("Total des alertes: %d\n", stats.niveau1 + stats.niveau2 + stats.niveau3);
    printf("=============================================\n\n");

    // Sauvegarde dans rapport_journalier.txt
    FILE *rapport = fopen("rapport_journalier.txt", "w");
    if (rapport != NULL) {
        fprintf(rapport, "========== RAPPORT DE STATISTIQUES ==========\n");
        fprintf(rapport, "Nombre total de mesures: %d\n", count);
        fprintf(rapport, "Temperature moyenne: %.2f C\n", somme / count);
        fprintf(rapport, "Temperature minimale: %.2f C\n", min);
        fprintf(rapport, "Temperature maximale: %.2f C\n", max);
        fprintf(rapport, "\n--- Alertes par niveau ---\n");
        fprintf(rapport, "Niveau 1: %d | Niveau 2: %d | Niveau 3: %d\n",
                stats.niveau1, stats.niveau2, stats.niveau3);
        fprintf(rapport, "Total: %d alertes\n", stats.niveau1 + stats.niveau2 + stats.niveau3);
        fclose(rapport);
        printf("OK Rapport sauvegarde dans rapport_journalier.txt\n\n");
    }
}
// ============ TÂCHE 7 : convertir_temperature ============
void convertir_temperature(float temp) {
    float kelvin = temp + 273.15;
    float fahrenheit = (temp * 9.0 / 5.0) + 32.0;

    printf("\n===== CONVERSION =====\n");
    printf("Temperature en Celsius   : %.2f C\n", temp);
    printf("Temperature en Kelvin    : %.2f K\n", kelvin);
    printf("Temperature en Fahrenheit: %.2f F\n", fahrenheit);
    printf("=======================\n\n");
}
// ============ TÂCHE 8 : convertion de la temperature enregistrée dans le ficher journalisation en unité desire par l'utilisateur============
void convertir_fichier_journal() {
    FILE *fichier = fopen("journalisation.txt", "r");
    if (fichier == NULL) {
        printf("Erreur : journalisation.txt introuvable\n");
        return;
    }

    FILE *sortie = fopen("journalisation_converti.txt", "w");
    if (sortie == NULL) {
        printf("Erreur : impossible de créer journalisation_converti.txt\n");
        fclose(fichier);
        return;
    }

    int choix;
    printf("Convertir en :\n");
    printf("1. Kelvin (K)\n");
    printf("2. Fahrenheit (F)\n");
    printf("Choix : ");
    scanf("%d", &choix);

    char ligne[200];
    float tempC;

    while (fgets(ligne, sizeof(ligne), fichier)) {
        char *ptr = strstr(ligne, "Temperature:");
        if (ptr != NULL) {
            if (sscanf(ptr, "Temperature: %f", &tempC) == 1) {

                float convertie;
                char unite;

                if (choix == 1) {   // Kelvin
                    convertie = tempC + 273.15;
                    unite = 'K';
                }
                else {              // Fahrenheit
                    convertie = (tempC * 9.0 / 5.0) + 32.0;
                    unite = 'F';
                }

                // Réécrire la ligne convertie dans le nouveau fichier
                fprintf(sortie, "Temperature convertie: %.2f %c\n", convertie, unite);
            }
        }
    }

    fclose(fichier);
    fclose(sortie);

    printf("Conversion terminee. Fichier genere : journalisation_converti.txt\n\n");
}
// ============ MENU PRINCIPAL ============
void afficher_menu() {
    printf("\n===============================================\n");
    printf("   SYSTEME DE MONITORING DE TEMPERATURE      \n");
    printf("===============================================\n");
    printf("1. Configurer le systeme\n");
    printf("2. Lire la configuration\n");
    printf("3. Simuler une mesure (aleatoire)\n");
    printf("4. Tester avec une valeur fixe\n");
    printf("5. Mode surveillance continu\n");
    printf("6. Afficher les statistiques\n");
    printf("7. Conversion de temperature\n");
    printf("8. Convertir les valeurs du journal\n");
    printf("9. Quitter\n");
    printf("-----------------------------------------------\n");
    printf("Choix: ");
}

// ============ FONCTION PRINCIPALE ============
int main() {
    srand(time(NULL));
    int choix;
    float temp;

    printf("\n*** CONTROLEUR INTELLIGENT DE TEMPERATURE IoT ***\n");
    printf("==================================================\n\n");

    // Configuration par défaut
    enregistrer_configuration(18.0, 26.0, 5);

    while (1) {
        afficher_menu();
        scanf("%d", &choix);
        printf("\n");

        switch (choix) {
            case 1: {
                float min, max;
                int intervalle;
                printf("Seuil minimum (C): ");
                scanf("%f", &min);
                printf("Seuil maximum (C): ");
                scanf("%f", &max);
                printf("Intervalle de mesure (secondes): ");
                scanf("%d", &intervalle);
                enregistrer_configuration(min, max, intervalle);
                enregistrer_evenement("Configuration mise à jour");
                break;
            }
            case 2:
                lire_configuration();
                break;

            case 3:
                temp = capturer_temperature();
                enregistrer_mesure(temp);
                analyser_temperature(temp);
                break;

            case 4:
                printf("Entrez la temperature de test (C): ");
                scanf("%f", &temp);
                enregistrer_mesure(temp);
                analyser_temperature(temp);
                break;

            case 5: {
                int nb_mesures;
                printf("Nombre de mesures a effectuer: ");
                scanf("%d", &nb_mesures);
                printf("\nDemarrage de la surveillance...\n\n");

                for (int i = 0; i < nb_mesures; i++) {
                    printf("--- Mesure %d/%d ---\n", i+1, nb_mesures);
                    temp = capturer_temperature();
                    enregistrer_mesure(temp);
                    analyser_temperature(temp);

                    if (i < nb_mesures - 1) {
                        printf("Pause de %d secondes...\n\n", config.intervalle_mesure);
                        // En simulation, on ne fait pas de vraie pause
                        // time_sleep(config.intervalle_mesure);
                    }
                }
                enregistrer_evenement("Surveillance continue terminée");
                break;
            }

            case 6:
                calculer_statistiques();
                break;
            case 7:
                printf("Entrez la temperature en Celsius: ");
                scanf("%f", &temp);
                convertir_temperature(temp);
                break;
            case 8:
                convertir_fichier_journal();
                break;
            case 9:
                printf("Arret du systeme de monitoring.\n");
                enregistrer_evenement("Systeme arrete");
                return 0;
            default:
                printf("Choix invalide\n");
        }
    }

    return 0;
}
