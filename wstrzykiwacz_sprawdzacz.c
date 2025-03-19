#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <pty.h>
#include <string.h>

int main(void) {
    int master_fd, slave_fd;
    char slave_name[100];
    int liczba; 

    printf("Podaj liczbę wierzcholkow, jaka chcesz przekazac do drugiego programu: ");
    scanf("%d", &liczba);

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

    // plik output.txt - wczytywanie go (wyswietlanie też dodałem, why not?)
    FILE *file = fopen("output.txt", "r");
    if (file == NULL) {
        perror("BŁAD! Nie mozna otworzyc pliku output.txt");
    } else {
        char line[256];
        printf("\nPo wygenerowaniu 'output.txt' za pomoca programu drugiej grupy, \njego zawarosc wyglada nastepujaco wyglada nastepujaco:\n");
        printf("---------------------------------------------------------------\n\n");
        while (fgets(line, sizeof(line), file) != NULL) {
            printf("%s", line);
        }
        
    }

    








    fclose(file);
    return 0;
}
