#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define LOWERBOUND 0.55
#define INCREASE 1.55
#define DECREASE 0.67

typedef enum { NONE, COMEDY, ADVENTURE, EDUCATIONAL, SCIFI, FANTASY } Category;

typedef struct Book {
    char isbn[15];
    char name[30];
    char auther[30];
    int pages;
    int year;
    Category category;
} Book;

typedef struct Shelf {
    Book** books;
    int capacity;
    int size;
} Shelf;

Category stringToEnum(char* str) {
    if (0 == strcmp("Comedy", str)) return COMEDY;
    if (0 == strcmp("Adventure", str)) return ADVENTURE;
    if (0 == strcmp("Educational", str)) return EDUCATIONAL;
    if (0 == strcmp("SciFi", str)) return SCIFI;
    if (0 == strcmp("Fantasy", str)) return FANTASY;
    return NONE;
}

char* enumToString(Category cat) {
    switch (cat) {
        case COMEDY:
            return "Comedy";
        case ADVENTURE:
            return "Adventure";
        case EDUCATIONAL:
            return "Educational";
        case SCIFI:
            return "SciFi";
        case FANTASY:
            return "Fantasy";
        default:
            return "None";
    }
}

void printBook(FILE* output, Book* book) {
    if (NULL == output || NULL == book) return;

    fprintf(output, "%s,%s,%s,%d,%d,%s\n",
                                        book->isbn,
                                        book->name,
                                        book->auther,
                                        book->pages,
                                        book->year,
                                        enumToString(book->category));
}

void printShelf(FILE* output, Shelf* shelf) {
    if (NULL == output || NULL == shelf || NULL == shelf->books) return;

    for (int i = 0; i < shelf->size; ++i)
        printBook(output, shelf->books[i]);
}

int findBook(Shelf* shelf, char* isbn) {
    if (NULL == shelf || NULL == shelf->books || NULL == isbn) return -1;

    for (int i = 0; i < shelf->size; ++i)
        if (0 == strcmp(shelf->books[i]->isbn, isbn))
            return i;
    
    return -1;
}

void resizeShelf(Shelf* shelf) {
    if (NULL == shelf) return;

    if (shelf->size == shelf->capacity) {
        shelf->capacity = ceil(shelf->capacity * INCREASE);
        if (0 == shelf->capacity) shelf->capacity = 1;
    }
    else if (shelf->size / shelf->capacity <= LOWERBOUND) {
        shelf->capacity = ceil(shelf->capacity * DECREASE);
        for (int i = shelf->size; i < shelf->capacity-1; ++i) free(shelf->books[i]);
    }
    
    shelf->books = (Book**)realloc(shelf->books, shelf->capacity * sizeof(Book*));
}

void addBookToShelf(Shelf* shelf, Book* book) {
    if (NULL == shelf || NULL == book || -1 != findBook(shelf, book->isbn)) return;

    resizeShelf(shelf);
    shelf->books[shelf->size++] = book;
}

void shiftBooks(Book** start, Book** end) {
    if (NULL == start || NULL == end) return;

    for (; start != end; ++start) {
        start[0] = start[1];
        start[1] = NULL;
    }
}

void removeBookFromShelf(Shelf* shelf, char* isbn) {
    if (NULL == shelf || NULL == shelf->books || NULL == isbn) return;

    int i = findBook(shelf, isbn);
    if (-1 != i) {
        free(shelf->books[i]);
        shelf->books[i] = NULL;
        shiftBooks(&shelf->books[i], &shelf->books[--shelf->size]);
        resizeShelf(shelf);
    }
}

Book* parseLine(char* line) {
    if (NULL == line || '#' == line[0] || '\n' == line[0] || '\r' == line[0]) return NULL;

    Book* book = (Book*)calloc(1, sizeof(Book));
    if (NULL == book) return NULL;

    strncpy(book->isbn, strtok(line, ",\n\r"), 14);
    strncpy(book->name, strtok(NULL, ",\n\r"), 29);
    strncpy(book->auther, strtok(NULL, ",\n\r"), 29);
    sscanf(strtok(NULL, ",\n\r"), "%d", &book->pages);
    sscanf(strtok(NULL, ",\n\r"), "%d", &book->year);
    book->category = stringToEnum(strtok(NULL, ",\n\r"));

    return book;
}

void freeBooks(Book** books, int start, int end) {
    for (int i = start; i < end; ++i) free(books[i]);
}

void initShelf(Shelf* shelf) {
    if (NULL != shelf) {
        freeBooks(shelf->books, 0, shelf->size);
        if (NULL != shelf->books) free(shelf->books);
        shelf->books = NULL;
        shelf->size = shelf->capacity = 0;
    }
}

typedef enum { WRITE, APPEND } Mode;

void readFromFile(FILE* input, Shelf* shelf, Mode mode) {
    if (NULL == shelf) return;

    char line[256] = { "\0" };    //  assuming a line won't be longer then 255 characters
    switch (mode) {
        case WRITE:
            initShelf(shelf);
        case APPEND:
            while (NULL != fgets(line, 256, input)) {
                addBookToShelf(shelf, parseLine(line));
            }
            break;
    }
}

void printMenu() {
    system("cls || clear");
    printf( "Menu: select option\n"\
            "1) add book\n"\
            "2) remove book\n"\
            "3) find book\n"\
            "4) print shelf\n"\
            "5) save shelf\n"\
            "6) load shelf\n"\
            "7) exit.\n"\
            ">");
}

int main(int argc, char* argv[]) {
    char input[256];
    FILE* inputFile;

    if (!(1 < argc)) {
        printf("please enter file name:\n>");
        scanf("%255s", input);
        inputFile = fopen(input, "r");
    }
    else
        inputFile = fopen(argv[1], "r");

    Shelf shelf = {NULL, 0, 0};
    readFromFile(inputFile, &shelf, WRITE);
    fclose(inputFile);

    puts("press <enter> to continue");
    while (1) {     //  0 is FALSE else TRUE!
        fflush(stdin);
        getchar();
        printMenu();
        scanf("%d", &argc);     //  reusing argc as prompt variable
        fflush(stdin);
        
        switch (argc) {
            case 1:
                printf("enter book details as follows: id,book name,author,pages,yearofpublishing,category\n>");
                addBookToShelf(&shelf, parseLine(fgets(input, 256, stdin)));
                continue;
            case 2:
                printf("please enter book isbn for removal:\n>");
                scanf("%255s", input);
                removeBookFromShelf(&shelf, input);
                continue;
            case 3:
                printf("please enter book isbn for viewing:\n>");
                scanf("%255s", input);
                int i;
                if (-1 != (i = findBook(&shelf, input)))
                    printBook(stdout, shelf.books[i]);
                else
                    printf("book not found!\n");
                continue;
            case 4:
                printShelf(stdout, &shelf);
                continue;
            case 5:
                printf("please enter save file name:\n>");
                scanf("%255s", input);
                FILE* outputFile = fopen(input, "w");
                printShelf(outputFile, &shelf);
                fclose(outputFile);
                continue;
            case 6:
                printf("please enter load file name and mode: mode = (A)ppend, (W)rite\n>");
                scanf("%255s %c", input, &argc);
                inputFile = fopen(input, "r");
                if ('W' == argc)
                    readFromFile(inputFile, &shelf, WRITE);
                else
                    readFromFile(inputFile, &shelf, APPEND);
                fclose(inputFile);
                continue;
            case 7:
                freeBooks(shelf.books, 0, shelf.size);
                break;
            default:
                printf("invalid input!\n");
                continue;
        }
        break;
    }

    return 0;
}
