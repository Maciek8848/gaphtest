#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* extract_text(){
    FILE* file = fopen("output.txt", "r");
    if (!file) {
        perror("Błąd otwarcia pliku");
        return NULL;
    }
    char line[256];
    int found_start = 0; // Flaga, czy znaleziono początek
    char* result = NULL; // Wynikowy tekst
    size_t result_size = 0; // Rozmiar wyniku

    while (fgets(line, sizeof(line), file)) {
        if (!found_start) {
            // Szukaj początku
            if (strstr(line, "polaczenia w grafie:")) {
                found_start = 1; // Znaleziono początek
            }
        } else {
            // Szukaj końca
            if (strstr(line, "---")) {
                break; // Znaleziono koniec, przerywamy
            }

            // Dodaj linię do wyniku
            size_t line_len = strlen(line);
            result = realloc(result, result_size + line_len + 1); // Zwiększ pamięć
            if (!result) {
                perror("Błąd alokacji pamięci");
                fclose(file);
                return NULL;
            }
            strcpy(result + result_size, line); // Dodaj linię do wyniku
            result_size += line_len; // Zaktualizuj rozmiar wyniku
        }
    }

    fclose(file);

    if (!found_start) {
        printf("Nie znaleziono początkowego markera");
        free(result);
        return NULL;
    }
    return result;
}
int main() {
    // Kompilacja pliku program_testowany.c
    printf("Kompilowanie program_testowany.c...\n");
    int compile_result = system("gcc -Wall --pedantic program_testowany.c -o program_testowany.exe -lcurl");
    
    if (compile_result != 0) {
        printf("Błąd kompilacji pliku program_testowany.c\n");
        return 1;
    }

    // Nadaj uprawnienia do uruchomienia
    printf("Nadawanie uprawnień do uruchomienia...\n");
    printf("Uruchamianie program_testowany...\n");

    if (system("chmod +x program_testowany.exe") != 0) {
        printf("Błąd nadawania uprawnień do uruchomienia\n");
        return 1;
    }

    // Przekierowanie stdout do pliku output.txt
    fflush(stdout); // Wyczyść bufor przed przekierowaniem
    if (freopen("output.txt", "w", stdout) == NULL) {
        perror("Błąd przekierowania stdout");
        return 1;
    }

    // Uruchom program_testowany w obecnym terminalu
    int run_result = system("./program_testowany.exe");
    //--------------------------
    if (run_result != 0) {
        printf("Błąd uruchomienia programu program_testowany\n");
        return 1;
    }
printf("---");

    // Przywrócenie stdout do konsoli
    fflush(stdout); // Wyczyść bufor przed przywróceniem
#ifdef _WIN32
    freopen("CON", "w", stdout); // Windows
#else
    freopen("/dev/tty", "w", stdout); // Linux/Unix
#endif

    printf("Wyjście zostało przywrócone do konsoli.\n");


    printf("%s",extract_text());
    return 0;
}