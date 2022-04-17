#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#define MAX_SIZE 20

typedef struct {
    int rowIndex;
    int colIndex;
} elemThreadReq;

long matrixA[MAX_SIZE][MAX_SIZE] = {{0}};
long matrixB[MAX_SIZE][MAX_SIZE] = {{0}};
long matrixC[MAX_SIZE][MAX_SIZE] = {{0}};

FILE *fileA, *fileB, *fileCS, *fileCPerRow, *fileCPerElem;
int numRowsA = 0, numColsA = 0, numRowsB = 0, numColsB = 0;

void createFiles(); /////creates the output files
void createCustomFiles(char *argv[]); /////creates the custom output files if needed
void readFile(FILE * ptr); /// read matrix dimensions and entries from a file
int getRowsOrCols(const char word[]);///// to get the dimensions of a matrix by parsing 1st line in the file

void threadPerMatrix(); /////creates a thread whose function is the mentioned just below
void *multiplyMatrixByMatrix(void *threadID); /////implement multiplication of mat by another

void threadPerRow(); ////creates a thread per each row in A and the given func below is its func
void *multiplyRowByMatrix(void * rowIndex); ////implement multiplic. of each row by the second matrix

void threadPerElement();////creates a thread per each row and col in C and the given func below is its func
void *multiplyRowByCol(void * req); ////multiply a row of mat. by col. of another to get an entry in C

int main(int argc, char *argv[]) {
    if(argc == 1) createFiles();
    else createCustomFiles(argv);
    readFile(fileA);
    readFile(fileB);

    struct timeval start, end;
    gettimeofday(&start, NULL);
    threadPerMatrix();
    gettimeofday(&end, NULL);
    printf("time taken = %lu microSec approximately %lu sec\n",
           end.tv_usec - start.tv_usec , end.tv_sec - start.tv_sec);
    for (int i = 0; i < numRowsA && numColsA == numRowsB; i++) {
        for (int j = 0; j < numColsB; j++) {
            printf("%5ld ", matrixC[i][j]);
            matrixC[i][j] = 0;
        }
        printf("\n");
    }
    printf("\n");
    gettimeofday(&start, NULL);
    threadPerRow();
    gettimeofday(&end, NULL);
    printf("time taken = %lu microSec approximately %lu sec\n",
           end.tv_usec - start.tv_usec , end.tv_sec - start.tv_sec);
    for (int i = 0; i < numRowsA && numColsA == numRowsB; i++) {
        for (int j = 0; j < numColsB; j++) {
            printf("%5ld ", matrixC[i][j]);
            matrixC[i][j] = 0;
        }
        printf("\n");
    }
    printf("\n");
    gettimeofday(&start, NULL);
    threadPerElement();
    gettimeofday(&end, NULL);
    printf("time taken = %lu microSec approximately %lu sec\n",
           end.tv_usec - start.tv_usec , end.tv_sec - start.tv_sec);
    for (int i = 0; i < numRowsA && numColsA == numRowsB; i++) {
        for (int j = 0; j < numColsB; j++) {
            printf("%5ld ", matrixC[i][j]);
        }
        printf("\n");
    }
    return 0;
}

void createFiles(){
    char path[1000] = "";
    chdir("..");
    getcwd(path, 1000);
    char pathA[1100], pathB[1100], pathCSingle[1100], pathCPerRow[1100], pathCPerElem[1100];
    strcat(strcpy(pathA, path), "/a.txt");
    strcat(strcpy(pathB, path), "/b.txt");
    strcat(strcpy(pathCSingle, path), "/c_per_matrix.txt");
    strcat(strcpy(pathCPerRow, path), "/c_per_row.txt");
    strcat(strcpy(pathCPerElem, path), "/c_per_element.txt");

    fileA = fopen(pathA, "r");
    fileB = fopen(pathB, "r");
    fileCS = fopen(pathCSingle, "w");
    fileCPerRow = fopen(pathCPerRow, "w");
    fileCPerElem = fopen(pathCPerElem, "w");
}
void createCustomFiles(char *argv[]){
    char path[1000] = "", temp[1000] = "/";
    chdir("..");
    getcwd(path, 1000);
    char pathA[1100], pathB[1100], pathCSingle[1100], pathCPerRow[1100], pathCPerElem[1100];
    strcat(temp, argv[1]);
    strcat(strcpy(pathA, path), strcat(temp, ".txt"));
    strcpy(temp, "/");
    strcat(temp, argv[2]);
    strcat(strcpy(pathB, path), strcat(temp, ".txt"));
    strcpy(temp, "/");
    strcat(temp, argv[3]);
    strcat(strcpy(pathCSingle, path), strcat(temp, "_per_matrix.txt"));
    strcpy(temp, "/");
    strcat(temp, argv[3]);
    strcat(strcpy(pathCPerRow, path), strcat(temp, "_per_row.txt"));
    strcpy(temp, "/");
    strcat(temp, argv[3]);
    strcat(strcpy(pathCPerElem, path), strcat(temp, "_per_element.txt"));

    fileA = fopen(pathA, "r");
    fileB = fopen(pathB, "r");
    fileCS = fopen(pathCSingle, "w");
    fileCPerRow = fopen(pathCPerRow, "w");
    fileCPerElem = fopen(pathCPerElem, "w");
}
void readFile(FILE * ptr){
    char *line;
    line = malloc(2000);
    char *word ;
    short firstLine = 1, rowFilled = 0;
    int counter = 0;
    while (fgets(line, 2000, ptr) != NULL){
        if(strcmp(line, "\n") != 0){
            if(!firstLine) {////////get each entry
                word = strtok(line, " \t");
                while(word != NULL) {
                    if(word[strlen(word) - 1] == '\n')
                        word[strlen(word) - 1] = '\0';
                    if(ptr == fileA)
                        matrixA[counter / numColsA][counter % numColsA] = (long)atoi(word);
                    else
                        matrixB[counter / numColsB][counter % numColsB] = (long)atoi(word);
                    counter++;
                    word = strtok(NULL, " \t");
                }
            }
            else {///////get cols and rows
                word = strtok(line, " ");
                while (word != NULL) {
                    if (ptr == fileA) {
                        if (!rowFilled) {
                            numRowsA = getRowsOrCols(word);
                            rowFilled = 1;
                        } else numColsA = getRowsOrCols(word);
                    } else {
                        if (!rowFilled) {
                            numRowsB = getRowsOrCols(word);
                            rowFilled = 1;
                        } else numColsB = getRowsOrCols(word);
                    }
                    word = strtok(NULL, " \t");
                }
                firstLine = 0;
            }
        }
    }
    if(ptr == fileA) fclose(fileA);
    else fclose(fileB);
}
int getRowsOrCols(const char word[]){
    int i = 0, counter = 0;
    short equalFound = 0;
    char result[1000] = "";
    while (word[i] != '\0' && word[i] != '\n'){
        if(equalFound) result[counter++] = word[i];
        else if(word[i] == '=') equalFound = 1;
        i++;
    }
    result[counter] = '\0';
    return atoi(result);
}

void threadPerMatrix(){
    printf("Method: A thread per matrix\n");
    fputs("Method: A thread per matrix\n", fileCS);
    if (numColsA != numRowsB || (numRowsA == 0 && numColsA == 0) || (numRowsB == 0 && numColsB == 0)) {
        fputs("can not multiply as no.of cols of A != no.of rows B\n", fileCS);
        printf("can not multiply as no.of cols of A != no.of rows B\n");
        return;
    }
    pthread_t matrixThread;
    long id = 1;
    long status = pthread_create(&matrixThread, NULL,
                                 multiplyMatrixByMatrix, &id);
    if(status){
        printf("error with return code: %ld", status);
        exit(-1);
    }
    pthread_join(matrixThread, NULL);
    fclose(fileCS);
}
void *multiplyMatrixByMatrix(void *threadId) {
    printf("hello from the single thread %ld\n", * (long *)threadId);
    for (int i = 0; i < numRowsA; i++) {
        for (int j = 0; j < numColsB; j++) {
            for (int k = 0; k < numRowsB; k++){
                matrixC[i][j] += matrixA[i][k] * matrixB[k][j];
            }
            fprintf(fileCS, "%5ld ", matrixC[i][j]);
        }
        fputs("\n", fileCS);
    }
    printf("done successfully\n");
    return NULL;
}

void threadPerRow(){
    fputs("Method: A thread per row\n", fileCPerRow);
    printf("Method: A thread per row\n");
    if (numColsA != numRowsB || (numRowsA == 0 && numColsA == 0) || (numRowsB == 0 && numColsB == 0)) {
        fputs("can not multiply as no.of cols of A != no.of rows B\n", fileCPerRow);
        printf("can not multiply as no.of cols of A != no.of rows B\n");
        return;
    }
    pthread_t rowThreads[numRowsA];
    long rowId = 0;
    while (rowId < numRowsA){
        long status = pthread_create(&rowThreads[rowId], NULL,
                                     multiplyRowByMatrix, (void *) rowId);
        if(status){
            printf("error with return code: %ld", status);
            exit(-1);
        }
        rowId++;
    }
    for(int id = 0; id < numRowsA; id++){
        pthread_join(rowThreads[id], NULL);
        printf("stopping thread: %d\n", id);
    }
    printf("done successfully\n");
    for(int i = 0; i < numRowsA; i++){
        for(int j = 0; j < numColsB; j++)
            fprintf(fileCPerRow, "%5ld ", matrixC[i][j]);
        fputs("\n", fileCPerRow);
    }
    fclose(fileCPerRow);
}
void *multiplyRowByMatrix(void * rowIndex){
    printf("hello from row-Matrix thread: %ld\n", (long)rowIndex);
    for(int i = 0; i < numColsB; i++){
        for(int k = 0; k < numRowsB; k++)
            matrixC[(long)rowIndex][i] += matrixA[(long)rowIndex][k] * matrixB[k][i];
    }
    return ++rowIndex;
}

void threadPerElement() {
    fputs("Method: A thread per element\n", fileCPerElem);
    printf("Method: A thread per element\n");
    if (numColsA != numRowsB || (numRowsA == 0 && numColsA == 0) || (numRowsB == 0 && numColsB == 0)) {
        fputs("can not multiply as no.of cols of A != no.of rows B\n", fileCPerElem);
        printf("can not multiply as no.of cols of A != no.of rows B\n");
        return;
    }
    elemThreadReq *reqs[numRowsA][numColsB];
    for (int i = 0; i < numRowsA; i++) {
        for (int j = 0; j < numColsB; j++) {
            elemThreadReq *req = malloc(sizeof(elemThreadReq));
            req->rowIndex = i, req->colIndex = j;
            reqs[i][j] = req;
        }
    }
    pthread_t threads[numRowsA][numColsB];
    long status;
    for (int i = 0; i < numRowsA; i++) {
        for (int j = 0; j < numColsB; j++) {
            status = pthread_create(&threads[i][j], NULL,
                                    multiplyRowByCol, reqs[i][j]);
            if (status) {
                printf("error with return code: %ld", status);
                exit(-1);
            }
        }
    }
    for (int i = 0; i < numRowsA; i++) {
        for (int j = 0; j < numColsB; j++) {
            pthread_join(threads[i][j], NULL);
            printf("stopping thread of row %d of A and col %d of B\n", i, j);
            free(reqs[i][j]);
        }
    }
    printf("done successfully\n");
    for (int i = 0; i < numRowsA; i++) {
        for (int j = 0; j < numColsB; j++) {
            fprintf(fileCPerElem, "%5ld ", matrixC[i][j]);
        }
        fputs("\n", fileCPerElem);
    }
    fclose(fileCPerElem);
}
void *multiplyRowByCol(void * req){
    int row = ((elemThreadReq *)req) ->rowIndex;
    int col = ((elemThreadReq *)req) ->colIndex;
    printf("hello from thread of row %d of A and col %d of B\n", row, col);
    for(int i = 0; i < numRowsB; i++)
        matrixC[row][col] += matrixA[row][i] * matrixB[i][col];
    return NULL;
}