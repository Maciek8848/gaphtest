#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <time.h>

void WypiszGraf(int **T, int n, FILE* out);
void PodajWynik(int argc, char** argv, int n, int **T);
char* AskChatbot(FILE* in);
int** ExtractData(char* text, int* n); // deklaracja przed main żeby -Wall nie krzyczał

int main(int argc, char** argv) // dodatkowa uwaga: niepoprawne parametry wywołania lub ich nadmierna ilość są po prostu ignorowane
{
    FILE* in = stdin;
    for(int i = 1; i < argc; i++)
    {
        if(strcmp(argv[i], "-rf") == 0) {
            in = fopen("input.txt", "r"); // pamiętać o zamknięciu pliku ZAWSZE
            if (in == NULL) {
                printf("Wywołano program w trybie czytania poleceń z pliku, ale nie znaleziono input.txt\n");
                return -1;
            }
            break;
        }
    }

    printf("Chcesz utworzyć graf samemu czy przy pomocy czatbota?\n[s - samemu, c - czatbot] ");
    int mode = fgetc(in);
    printf("\n");

    if(mode == 's')
    {
		int n = 0;
		printf("podaj liczbe wierzcholkow: ");
		fscanf(in, " %d", &n);
		if(n < 1) { 
            fclose(in);
			return 1;
		}

		int **T = malloc(n * sizeof(int*));
		if(T == NULL){
            printf("Nie udało się przypisać pamięci dla tablicy reprezentującej graf.\n");
            fclose(in);
			return 2;
		}
		for (int i = 0; i < n; i++) {
			T[i] = malloc(n * sizeof(int));
			if(T[i] == NULL){
                printf("Nie udało się przypisać pamięci dla tablicy reprezentującej graf.\n");
                fclose(in);
				return 3;
			}
		}
		for(int i = 0; i < n; i++)
		{
			for(int j = 0; j < n; j++)
			{
				T[i][j] = 0;
			}
		}
		int a = 0, b = 0;
		int r=n*n*2+1;
		srand(time(NULL));
		printf("jesli chcesz by graf byl losowany, wpisz y, by w przeciwnym wypadku kliknij enter ");
		int los = fgetc(in);
			if(los == '\n') {
				los = fgetc(in);
			}
		for(;;)
		{
			if(los == 'y')
			{
                a=rand()%n+1;
                b=rand()%n+1;
			}
            else
			{
                a = 0;
                b = 0;
                printf("podaj wierzcholek z ktorego chcesz zrobic polaczenie, lub q jesli chcesz przestac pisac wierzcholki:");
                a = fgetc(in);
                if(a == '\n' || a == ' ')
                {
                    a = fgetc(in);
                }
                if(a == 'q')
                {
                    break;
                }
                
                printf("podaj wierzcholek do ktorego chcesz zrobic polaczenie:");
                b = fgetc(in);
                if(b == '\n' || b == ' ')
                {
                    b = fgetc(in);
                }
			}
			if((los == 'y'  &&  rand()%r == 0) || a == 'q')
			{
				break;
			}
			if(los != 'y')
			{
				a=a-'0';
				b=b-'0';
			}
			if(a>0 && b>0 && a<=n && b<=n)
			{
				T[a-1][b-1]=1;
			}
		}
        PodajWynik(argc, argv, n, T);
		free(T);
    }
    else if(mode == 'c')
    {
        int n = 0;
        int **T = ExtractData(AskChatbot(in), &n);
        if (T == NULL) {
            fclose(in);
            return -2;
        }
        PodajWynik(argc, argv, n, T);
		free(T);
    }
    else 
    {
        printf("Podano niewłaściwą opcję.\n");
        fclose(in);
        return -1;
    }

    fclose(in);
	return 0;
}

void WypiszGraf(int **T, int n, FILE* out)
{
	fprintf(out, "wierzcholki w grafie: ");
	for(int i = 0; i < n; i++)
	{
		fprintf(out, "W%d ",i+1);
	}
    fprintf(out, "\npolaczenia w grafie: ");
    for(int i = 0; i < n; i++)
    {
        fprintf(out, "\n");
        for(int j = 0; j < n; j++)
        {
            if(T[i][j] == 1){
                fprintf(out, "W%d->W%d ", i+1, j+1);
            }			
        }
    }
    fprintf(out, "\n");
}

void PodajWynik(int argc, char** argv, int n, int **T)
{
    for(int i = 1; i < argc; i++)
    {
        if(strcmp(argv[i], "-wf") == 0)
        {
            FILE* out = fopen("output.txt", "w");
            WypiszGraf(T, n, out);
            fclose(out);
            printf("Graf został zapisany do pliku output.txt\n");
            return;
        }
    }
    WypiszGraf(T, n, stdout);
}

char* AskChatbot(FILE* in)
{
    char message[2000];
    printf("Napisz wiadomość do czatbota: \n");
    int c = fgetc(in); // czasami w strumieniu zostaje \n po scanf i psuje fgets
    if (c != '\n') {
        ungetc(c, in);
    }
	fgets(message, sizeof(message), in);
	
    char* url = "http://localhost:1234/api/v0/chat/completions";
	// UWAGA na wsl serwer może mieć problemy ale to już jego sprawa, kod działa tylko serwer nie chce odpowiedzieć
    char jsonInputString[5000];
    sprintf(jsonInputString,
    "{"
        " \"model\": \"granite-3.0-2b-instruct\","
        " \"messages\": [ "
            " { \"role\": \"system\", \"content\": \"Twoim jedynym zadaniem jest pomóc użytkownikowi stworzyć graf."
			" Musisz odpowiedzieć w następującym formacie: # N  W x1 x2 xN  W x1 x2 xN ...  W x1 x2 xN gdzie # to po"
			" prostu #, N to ilość wierzchołków, W to numer wierzchołka od 1 do N, x1 x2 ... xN to cyfry 0 lub 1"
            " w ilości N gdzie 0/1 na miejscu x1 oznacza że wierzchołek W ma/nie ma łuku do wierzchołka nr 1 itd."
            " Maksymalna ilość wierzchołków to 5\" },"
            " { \"role\": \"user\", \"content\": \"%s\" }"
        " ],"
        " \"temperature\": 0.0,"
        " \"max_tokens\": -1,"
        " \"stream\": false"
    "}", message);

    CURL* curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if (curl) {
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, "Accept: application/json");

		char* response = malloc(4000 * sizeof(char)); // inaczej -Wall nie pozwoli na return zm lokalnej
		size_t write_callback;

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonInputString);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L); // limit czasowy na 15 sekund

        printf("\nOczekiwanie na połączenie...\n");

        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            printf("CURL request failed: %s\n", curl_easy_strerror(res));
            char* example = malloc(400 * sizeof(char)); //ExtractData potrzebuje dynamicznego ciągu znaków
            strcpy(example, "Cześć, chętnie pomogę ci utworzyć graf! Oto zapis grafu zgodny z formatem który"
            " otrzymałem: # 5   1 1 1 0 1 0   2 1 0 1 0 0   3 1 0 0 1 1   4 1 1 1 0 1   5 1 0 0 0 1"); // strcpy żeby nie nadpisać wskaźnika
            printf("Zwrócono przykładową odpowiedź od czatbota: \n\n%s\n\n", example);
            curl_easy_cleanup(curl);
            curl_slist_free_all(headers);
            free(response);
            return example;
        }
        else
        {
            printf("Chatbot: %s\n", response);
            curl_easy_cleanup(curl);
            curl_slist_free_all(headers);
            return response;
        }
    }
    else
    {
        printf("Error: Failed to initialize curl.\n");
        return "error";
    }
    curl_global_cleanup();
}

int** ExtractData(char text[], int* n)
{
    char* znak = "#";
    char* token;

    token = strtok(text, znak); // dotarł do znaku (albo do końca napisu i go nie znalazł)
    if(token == NULL) {
        printf("Błędny format odpowiedzi: Nie znaleziono znaku '%s' w ciągu znaków.\n", znak);
        free(text);
        return NULL;
    }
    token = strtok(NULL, " "); // przechodzi do następnego tokena
    if(token == NULL || sscanf(token, "%d", n) != 1) {
        printf("Błędny format odpowiedzi: Nie znaleziono liczby całkowitej po znaku kodowym '%s'.\n", znak);
        free(text);
        return NULL;
    }
    
	int **T = malloc((*n) * sizeof(int*));
	if(T == NULL){
        printf("Nie udało się przypisać pamięci dla tablicy reprezentującej graf.\n");
        free(text);
		return NULL;
	}
	for (int i = 0; i < (*n); i++) {
		T[i] = malloc((*n) * sizeof(int));
		if(T[i] == NULL){
            printf("Nie udało się przypisać pamięci dla tablicy reprezentującej graf.\n");
            free(text);
            free(T);
			return NULL;
		}
	}

    token = strtok(NULL, " "); // przechodzimy na liczbę oznaczającą 1szy wierzchołek

    if (token != NULL)
    {
        for(int i = 0; i < *n; i++)
        {
            token = strtok(NULL, " "); // pierwsza liczba z serii to nr wierzchołka
            for(int j = 0; j < *n; j++)
            {
                if(token == NULL || sscanf(token, "%d", &T[i][j]) != 1) {
                    printf("Błędny format odpowiedzi: Spodziewano się informacji o połączeniu W%d->W%d, ale nie trafiono na liczbę.\n", i, j);
                    free(text);
                    free(T);
                    return NULL;
                }
                if(T[i][j] != 0 && T[i][j] != 1) { // <- nie może być else if bo sprawdzamy wynik z warunku pierwszego if
                    printf("Błędny format odpowiedzi: Spodziewano się informacji o połączeniu W%d->W%d, ale trafiono na symbol wierzchołka '%d'\n", i+1, j+1, T[i][j]);
                    free(text);
                    free(T);
                    return NULL;
                }
                token = strtok(NULL, " ");
            }
        }
    }
    else {
        printf("Błędny format odpowiedzi: Nie znaleziono nazwy wierzchołka po podanej ilości wierzchołków.\n");
    }
    free(text);
    return T;
}
