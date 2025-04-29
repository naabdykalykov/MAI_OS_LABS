#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>
#include <time.h>

typedef struct {
    double* input;      // откуда берем данные 
    double* output;     // куда вывводим даные
    double* kernel;      //наше ядро свёртки
    int rows;            //колво строк
    int cols;            //столбцов
    int kernelSize;      // размер ядра свёртки
    int rowStart;        //начало строки далее конец строки
    int rowEnd;          
} ThreadData;

// чтобы поток обработал свою часть, переданных через thread data (то что выше)
void* threadConvolution(void* lpParam)
{
    ThreadData* td = (ThreadData*)lpParam;
    //центровка ядра
    // kr - kernel row; kc - kernel cols; rr - в исходной; cc - в исходной
    int kHalf = td->kernelSize / 2;  
    for (int r = td->rowStart; r < td->rowEnd; r++) {
        for (int c = 0; c < td->cols; c++) {
            double sum = 0.0;
            for (int kr = 0; kr < td->kernelSize; kr++) {
                for (int kc = 0; kc < td->kernelSize; kc++) {
                    int rr = r + kr - kHalf;
                    int cc = c + kc - kHalf;
                    
                    if (rr >= 0 && rr < td->rows && cc >= 0 && cc < td->cols) {
                        sum += td->input[rr * td->cols + cc] 
                               * td->kernel[kr * td->kernelSize + kc];
                    }
                }
            }
            td->output[r * td->cols + c] = sum;
        }
    }
    return NULL;
}

void applyConvolution2D(
    double* input, 
    double* output, 
    double* kernel, 
    int rows, 
    int cols, 
    int kernelSize, 
    int maxThreads)
{
    int numThreads = (rows < maxThreads) ? rows : maxThreads;
    //потоков не может быть больше чем строк
    
    int chunkSize = rows / numThreads;
    int remainder = rows % numThreads;
    
    // массив для хранения идентификаторов потоков
    pthread_t* threads = (pthread_t*)malloc(numThreads * sizeof(pthread_t));

    // массив для хранения данных (параметров) для каждого потока
    ThreadData* td = (ThreadData*)malloc(numThreads * sizeof(ThreadData));

    
    int currentStart = 0;
    
    for (int i = 0; i < numThreads; i++) {
        int start = currentStart;
        int size = chunkSize + ((i < remainder) ? 1 : 0);
        int end = start + size;
        
        td[i].input = input;
        td[i].output = output;
        td[i].kernel = kernel;
        td[i].rows = rows;
        td[i].cols = cols;
        td[i].kernelSize = kernelSize;
        td[i].rowStart = start;
        td[i].rowEnd = end;
        
        pthread_create(&threads[i], NULL, threadConvolution, &td[i]);
        // создаём поток, который запустит функцию threadConvolution.
        currentStart = end;
    }
    
    for (int i = 0; i < numThreads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    free(threads);
    free(td);
}

int main(int argc, char* argv[])
{
    if (argc < 6) {
        printf("Usage: %s <rows> <cols> <kernelSize> <K> <maxThreads>\n", argv[0]);
        return 1;
    }
    
    int rows = atoi(argv[1]);
    int cols = atoi(argv[2]);
    int kernelSize = atoi(argv[3]);
    int K = atoi(argv[4]);
    int maxThreads = atoi(argv[5]);
    
    if (rows <= 0 || cols <= 0 || kernelSize <= 0 || K <= 0 || maxThreads <= 0) {
        printf("Error: invalid arguments\n");
        return 1;
    }
    
    double* matrix = (double*)malloc(rows * cols * sizeof(double));
    double* temp   = (double*)malloc(rows * cols * sizeof(double));


     // Инициализация генератора случайных чисел
    srand(12345);
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            matrix[r * cols + c] = (double)(rand() % 100) / 10.0;
        }
    }

    printf("Matrix size: %dx%d\n", rows, cols);
    printf("Kernel size: %d\n", kernelSize);
    printf("Number of iterations (K): %d\n", K);
    printf("Maximum number of threads: %d\n", maxThreads);
    printf("\n");

    // Вывод начальной матрицы
    printf("Initial matrix:\n");
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            printf("%6.2f ", matrix[r * cols + c]);
        }
        printf("\n");
    }
    printf("\n");
    
    double* kernel = (double*)malloc(kernelSize * kernelSize * sizeof(double));
    for (int i = 0; i < kernelSize * kernelSize; i++) {
        kernel[i] = (double)(rand() % 10) / 10.0;
    }
    
    // Замер времени начала выполнения
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < K; i++) {
        applyConvolution2D(matrix, temp, kernel, rows, cols, kernelSize, maxThreads);
        double* swap = matrix;
        matrix = temp;
        temp = swap;
    }

    // Замер времени окончания выполнения
    clock_gettime(CLOCK_MONOTONIC, &end);

    // Вычисление затраченного времени в секундах
    double time_taken = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;


    printf("Resulting matrix:\n");
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            printf("%6.2f ", matrix[r * cols + c]);
        }
        printf("\n");
    }
    
    printf("\nTime taken: %.6f seconds\n", time_taken);
    
    free(matrix);
    free(temp);
    free(kernel);

    return 0;
}