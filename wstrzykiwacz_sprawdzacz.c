#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <pty.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>


int policz_wierzcholki() {
    FILE *file = fopen("output.txt", "r");
    if (file == NULL) {
        perror("Nie mozna otworzyc pliku");
        return -1; // Zwróć -1 w przypadku błędu
    }

    char line[256];
    int liczba_wierzcholkow = 0;

    // Przeszukaj plik linia po linii
    while (fgets(line, sizeof(line), file) != NULL) {
        // Sprawdź, czy linia zaczyna się od "wierzcholki w grafie:"
        if (strstr(line, "wierzcholki w grafie:") != NULL) {
            // Zlicz liczbę wystąpień 'W' w tej linii
            for (int i = 0; line[i] != '\0'; i++) {
                if (line[i] == 'W') {
                    liczba_wierzcholkow++;
                }
            }
            break; // Zakończ przeszukiwanie po znalezieniu odpowiedniej linii
        }
    }

    fclose(file);

    return liczba_wierzcholkow; // Zwróć liczbę wierzchołków
}
int sprawdz_polaczenia() {
    FILE *file = fopen("output.txt", "r");
    if (file == NULL) {
        perror("Nie mozna otworzyc pliku");
        return -1; // Zwróć -1 w przypadku błędu
    }

    char line[256];
    bool polaczenia[100][100] = {false}; // Tablica do przechowywania połączeń (zakładamy maksymalnie 100 wierzchołków)
    bool dwukierunkowe[100][100] = {false}; // Tablica do przechowywania połączeń dwukierunkowych
    bool powtorzenia = false;
    bool istnieja_dwukierunkowe = false;

    // Przeszukaj plik linia po linii
    while (fgets(line, sizeof(line), file) != NULL) {
        // Sprawdź, czy linia zaczyna się od "polaczenia w grafie:"
        if (strstr(line, "polaczenia w grafie:") != NULL) {
            // Przejdź do następnych linii, aby odczytać połączenia
            while (fgets(line, sizeof(line), file) != NULL) {
                int x, y;
                // Przetwarzaj linię, szukając połączeń w formacie "Wx->Wy"
                char *token = strtok(line, " ");
                while (token != NULL) {
                    if (sscanf(token, "W%d->W%d", &x, &y) == 2) {
                        // Sprawdź, czy połączenie już istnieje
                        if (polaczenia[x][y]) {
                            powtorzenia = true; // Znaleziono powtórzenie
                        } else {
                            polaczenia[x][y] = true; // Zaznacz połączenie
                        }

                        // Sprawdź, czy istnieje połączenie dwukierunkowe
                        if (polaczenia[y][x]) {
                            istnieja_dwukierunkowe = true; // Znaleziono połączenie dwukierunkowe
                            dwukierunkowe[x][y] = true;
                            dwukierunkowe[y][x] = true;
                        }
                    }
                    token = strtok(NULL, " ");
                }
            }
            break; // Zakończ przeszukiwanie po znalezieniu sekcji "polaczenia w grafie:"
        }
    }

    fclose(file);

    // Zwróć odpowiedni kod w zależności od warunków
    if (powtorzenia && !istnieja_dwukierunkowe) {
        return 0; // Są powtarzające się połączenia, ale nie ma dwukierunkowych
    } else if (istnieja_dwukierunkowe && powtorzenia) {
        return 1; // Są dwukierunkowe i powtarzające się połączenia
    } else if (!powtorzenia && !istnieja_dwukierunkowe) {
        return 2; // Nie ma powtarzających się ani dwukierunkowych połączeń
    } else if (!powtorzenia && istnieja_dwukierunkowe) {
        return 3; // Nie ma powtarzających się, ale są dwukierunkowe połączenia
    }


}

void test(int liczba){
    int master_fd, slave_fd;
    char slave_name[100];

    
    // SPOSÓB: ten kod tworzy parę tzw. "pseudoterminali" (w relacji master i slave)
    if (openpty(&master_fd, &slave_fd, slave_name, NULL, NULL) == -1) {
        perror("openpty");
        exit(EXIT_FAILURE);
    }
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if (pid == 0) {
        // Proces dziecka: zamykamy uchwyt master, używamy slave
        close(master_fd);

        // ustawiona jest nowa sesja, aby 'slave' stał się terminalem kontrolującym
        if (setsid() < 0) {
            perror("setsid");
            exit(EXIT_FAILURE);
        }

        // Przekieruj standardowe wejście, wyjście i błąd do slave pty
        if (dup2(slave_fd, STDIN_FILENO) != STDIN_FILENO ||
            dup2(slave_fd, STDOUT_FILENO) != STDOUT_FILENO ||
            dup2(slave_fd, STDERR_FILENO) != STDERR_FILENO) {
            perror("dup2");
            exit(EXIT_FAILURE);
        }
        if (slave_fd > STDERR_FILENO)
            close(slave_fd);

        //z parametrem -wf, zeby generowal output.txt
        execl("./program_testowany", "program_testowany", "-wf", NULL);
        perror("execl");
        exit(EXIT_FAILURE);
    } else {
        // Proces rodzica: zamykamy uchwyt slave, bo piszemy tylko do master
        close(slave_fd);

        // Opcjonalnie: krótka przerwa, aby dziecko rozpoczęło działanie
        sleep(1);

        // Wpisz kolejno dane symulujące interakcję użytkownika:
        // "s", potem "7", potem "y"
        if (write(master_fd, "s\n", 2) == -1) {
            perror("write s");
        }
        sleep(1); // czekamy chwilę przed kolejnymi danymi

        char liczba_str[4];   //dlugosc np. max 4 cyfry
        snprintf(liczba_str, sizeof(liczba_str), "%d\n", liczba);
        if (write(master_fd, liczba_str, strlen(liczba_str)) == -1) {
            perror("write liczba");
        }

        if (write(master_fd, "y\n", 2) == -1) {
            perror("write y");
        }

        // Czekamy na zakończenie dziecka
        wait(NULL);
        close(master_fd);
    }
 
}
int komunikaty(int liczba){
    int tests_failed = 0;
    if(liczba == policz_wierzcholki()){
        printf("ilość wieżchołków - PASSED\n");
    }
    else{
        printf("ilość wieżchołków - FAILED\n");
        tests_failed++;
    }
    int komunikat = sprawdz_polaczenia();
    if(komunikat == 0){
        printf("Brak powtarzania wierzchołków - FAILED\nKrawędzie dwukierunkowe - FAILED\n");
        tests_failed = tests_failed + 2;
    }
    else if(komunikat == 1){
        printf("Brak powtarzania wierzchołków - FAILED\nKrawędzie dwukierunkowe - PASSED\n");
        tests_failed++;
    }
    else if(komunikat == 2){
        printf("Brak powtarzania wierzchołków - PASSED\nKrawędzie dwukierunkowe - FAILED\n");
        tests_failed++;
    }
    else{
        printf("Brak powtarzania wierzchołków - PASSED\nKrawędzie dwukierunkowe - PASSED\n");
    }
    return tests_failed;
    
}
void print_output(){
    FILE *file = fopen("output.txt", "r");
    if (file == NULL) {
        perror("BŁAD! Nie mozna otworzyc pliku output.txt");
    } else {
        char line[256];
        while (fgets(line, sizeof(line), file) != NULL) {
            printf("%s", line);
        }
        
    }
    fclose(file);
}
int main(void) {
    int ilosc_test, liczba, bledy;
    int i=0;
    int total_failed_tests = 0;
    printf("Enter the number of tests: ");
    scanf("%d", &ilosc_test);
    srand(time(NULL));
    // Generowanie losowej liczby od 2 do 10
    do{
        i++;
        liczba = rand() % 9 + 2;
        test(liczba);
        printf("\nTEST nr.%d\n", i);
        bledy = komunikaty(liczba);
        if (bledy != 0){
            printf("----------------------------\n");
            print_output();
            printf("----------------------------\n");
        }
        total_failed_tests = total_failed_tests + bledy;
        
    }   while (i < ilosc_test);
    


    printf("Total failed tests: %d\n", total_failed_tests);
    printf("Total passed tests: %d\n", 3 * i - total_failed_tests);
    return 0;
}
