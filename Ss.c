#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <assert.h>
#include <pthread.h>
#include <math.h>
#include <stdbool.h>
#include <limits.h>

#define MAX_SIZE 4096
#define MAX_POINTS 4096
#define MAX_CLUSTERS 32
typedef double matrix[MAX_SIZE][MAX_SIZE];
char *args_arr[10];
char *kmeans_file;

int	N;		/* matrix size		*/
int	maxnum;		/* max number of element*/
char* Init;		/* matrix init type	*/
int	PRINT;		/* print switch		*/
matrix	A;		/* matrix A		*/
matrix I = {0.0};  /* The A inverse matrix, which will be initialized to the identity matrix */
//char pathAbs[200]="/Users/abbas-ali/Desktop/This Mac/D Drive/Practise Projects/Clion Project/Project/Computed Results/";

typedef struct point
{
    float x; // The x-coordinate of the point
    float y; // The y-coordinate of the point
    int cluster; // The cluster that the point belongs to
} point;

int	N;		// number of entries in the data
int clusters;
point data[MAX_POINTS];		// Data coordinates
point cluster[MAX_CLUSTERS]; // The coordinates of each cluster center (also called centroid)

void read_data(int k,char* filename)
{
    //char pathRead[120]="";
    //char pathRead1[200];
    //strcpy(pathRead1,pathRead);
    //strcat(pathRead1,filename);
    N = 1797;
    FILE* fp = fopen(filename,"r");
    if (fp == NULL) {
        perror("Cannot open the file kmeansdata.txt");
        exit(EXIT_FAILURE);
    }

    // Initialize points from the data file
    float temp;
    for (int i = 0; i < N; i++)
    {
        fscanf(fp, "%f %f", &data[i].x, &data[i].y);
        data[i].cluster = -1; // Initialize the cluster number to -1
    }
    // Initialize centroids randomly
    srand(0); // Setting 0 as the random number generation seed
    for (int i = 0; i < k; i++)
    {
        int r = rand() % N;
        cluster[i].x = data[r].x;
        cluster[i].y = data[r].y;
    }
    fclose(fp);
}

int get_closest_centroid(int i, int k)
{
    /* find the nearest centroid */
    int nearest_cluster = -1;
    double xdist, ydist, dist, min_dist;
    min_dist = dist = INT_MAX;
    for (int c = 0; c < k; c++)
    { // For each centroid
        // Calculate the square of the Euclidean distance between that centroid and the point
        xdist = data[i].x - cluster[c].x;
        ydist = data[i].y - cluster[c].y;
        dist = xdist * xdist + ydist * ydist; // The square of Euclidean distance
        //printf("%.2lf \n", dist);
        if (dist <= min_dist)
        {
            min_dist = dist;
            nearest_cluster = c;
        }
    }
    //printf("-----------\n");
    return nearest_cluster;
}

bool assign_clusters_to_points()
{
    bool something_changed = false;
    int old_cluster = -1, new_cluster = -1;
    for (int i = 0; i < N; i++)
    { // For each data point
        old_cluster = data[i].cluster;
        new_cluster = get_closest_centroid(i, clusters);
        data[i].cluster = new_cluster; // Assign a cluster to the point i
        if (old_cluster != new_cluster)
        {
            something_changed = true;
        }
    }
    return something_changed;
}

void update_cluster_centers()
{
    /* Update the cluster centers */
    int c;
    int count[MAX_CLUSTERS] = { 0 }; // Array to keep track of the number of points in each cluster
    point temp[MAX_CLUSTERS] = { 0.0 };

    for (int i = 0; i < N; i++)
    {
        c = data[i].cluster;
        count[c]++;
        temp[c].x += data[i].x;
        temp[c].y += data[i].y;
    }
    for (int i = 0; i < clusters; i++)
    {
        cluster[i].x = temp[i].x / count[i];
        cluster[i].y = temp[i].y / count[i];
    }
}

void kmeans(int k)
{
    bool somechange;
    int iter = 0;
    do {
        iter++; // Keep track of number of iterations
        somechange = assign_clusters_to_points();
        update_cluster_centers();
    } while (somechange);
}

void write_results(char* filename)
{
    FILE* fp = fopen(filename, "w");
    if (fp == NULL) {
        perror("Cannot open the file");
        exit(EXIT_FAILURE);
    }
    else
    {
        for (int i = 0; i < N; i++)
        {
            fprintf(fp, "%.2f %.2f %d\n", data[i].x, data[i].y, data[i].cluster);
        }
    }
}
/* forward declarations */
void find_inverse(void);
void Init_Matrix(void);
char* Print_Matrix(matrix M, char name[]);
void Init_Default(void);
int Read_Options(int, char**);
int Read_Input_Params(char input[]);
void Write_To_File(char* filename,char * output);

struct arg_struct {
    int socket_desc;
    int client_id;
};


void *client_handler(void *argument) {
    struct arg_struct *args = (struct arg_struct *)argument;
    // Get the socket descriptor
    int sock = args->socket_desc;
    int client_id =args->client_id;
    int read_size;
    char *message , client_message[100];
    char fileName[100];
    char pathAbs1[200];
    int argument_size;
    int requests=1;
    while (1){

    if((read_size = read(sock, client_message, 100)) > 0) {
        //int problem_type = atoi(problem_t);

       printf("From Client :%s\nSending Solution to Client : %s\n",client_message,fileName);
       argument_size = Read_Input_Params(client_message);
       if(strcmp(args_arr[0],"matinvpar")==0){
           //strcpy(pathAbs1,pathAbs);
           sprintf(fileName,"matinvpar_client%dsoln%d.txt",client_id,requests);
           //strcat(pathAbs1,fileName);
           Init_Default();		/* Init default values	*/
           Read_Options(argument_size, args_arr);	/* Read arguments	*/
           Init_Matrix();		/* Init the matrix	*/
           find_inverse();
           message = Print_Matrix(I, "Inversed");
           Write_To_File(fileName,message);
           write(sock, fileName, strlen(fileName));
           requests++;
       }else if(strcmp(args_arr[0],"kmeanspar")==0){
           //strcpy(pathAbs1,pathAbs);
           sprintf(fileName,"kmeans_client%dsoln%d.txt",client_id,requests);
           //strcat(pathAbs1,fileName);
           Read_Options(argument_size, args_arr);
           read_data(clusters,kmeans_file);
           kmeans(clusters);
           write_results(fileName);
           write(sock, fileName, strlen(fileName));
           requests++;
       }
    }

    if(read_size == 0) {
        puts("Client disconnected");
        fflush(stdout);
        break;
    } else if(read_size == -1) {
        perror("recv failed");
        break;
    }
    }

    return 0;
}


int main(int argc, char *argv[]) {
    int sockfd, newsockfd, portno, clilen;
    char buffer[256];
    pthread_t thread_id;
    struct sockaddr_in serv_addr, cli_addr;
    int n, problem;

    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        perror("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
             sizeof(serv_addr)) < 0)
        perror("ERROR on binding");
    listen(sockfd,5);
    clilen = sizeof(cli_addr);
    int client_number=1;
    while (1) {
        newsockfd = accept(sockfd,
                           (struct sockaddr *) &cli_addr,
                           (socklen_t*)&clilen);
        printf("%s %d\n","Connected to Client ", client_number);
        if (newsockfd < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        // Create a new thread to handle the client
        struct arg_struct args;
        args.socket_desc = newsockfd;
        args.client_id = client_number;
        if(pthread_create( &thread_id , NULL ,  client_handler , (void*) &args) < 0) {
            perror("could not create thread");
            exit(EXIT_FAILURE);
        }
        client_number++;
    }
    close(sockfd);
    return 0;
}




void find_inverse()
{
    int row, col, p; // 'p' stands for pivot (numbered from 0 to N-1)
    double pivalue; // pivot value

    /* Bringing the matrix A to the identity form */
    for (p = 0; p < N; p++) { /* Outer loop */
        pivalue = A[p][p];
        for (col = 0; col < N; col++)
        {
            A[p][col] = A[p][col] / pivalue; /* Division step on A */
            I[p][col] = I[p][col] / pivalue; /* Division step on I */
        }
        assert(A[p][p] == 1.0);

        double multiplier;
        for (row = 0; row < N; row++) {
            multiplier = A[row][p];
            if (row != p) // Perform elimination on all except the current pivot row
            {
                for (col = 0; col < N; col++)
                {
                    A[row][col] = A[row][col] - A[p][col] * multiplier; /* Elimination step on A */
                    I[row][col] = I[row][col] - I[p][col] * multiplier; /* Elimination step on I */
                }
                assert(A[row][p] == 0.0);
            }
        }
    }
}

void Init_Matrix()
{
    int row, col;

    // Set the diagonal elements of the inverse matrix to 1.0
    // So that you get an identity matrix to begin with
    for (row = 0; row < N; row++) {
        for (col = 0; col < N; col++) {
            if (row == col)
                I[row][col] = 1.0;
        }
    }



    if (strcmp(Init, "rand") == 0) {
        for (row = 0; row < N; row++) {
            for (col = 0; col < N; col++) {
                if (row == col) /* diagonal dominance */
                    A[row][col] = (double)(rand() % maxnum) + 5.0;
                else
                    A[row][col] = (double)(rand() % maxnum) + 1.0;
            }
        }
    }
    if (strcmp(Init, "fast") == 0) {
        for (row = 0; row < N; row++) {
            for (col = 0; col < N; col++) {
                if (row == col) /* diagonal dominance */
                    A[row][col] = 5.0;
                else
                    A[row][col] = 2.0;
            }
        }
    }

    if (PRINT == 1)
    {
        //Print_Matrix(A, "Begin: Input");
        //Print_Matrix(I, "Begin: Inverse");
    }
}

char* Print_Matrix(matrix M, char name[])
{
    char buffer[30];
    char* result = (char*) malloc(N * N * 20 * sizeof(char));
    result[0] = '\0';
    sprintf(buffer,"\nsize      = %dx%d ", N, N);
    strcat(result, buffer);
    sprintf(buffer,"\nmaxnum    = %d \n", maxnum);
    strcat(result, buffer);
    sprintf(buffer,"Init	  = %s \n", Init);
    strcat(result, buffer);
    sprintf(buffer,"Initializing matrix...done\n\n");
    strcat(result, buffer);
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            sprintf(buffer, " %5.2f", M[i][j]);
            strcat(result, buffer);
        }
        strcat(result, "\n");
    }
    return result;
}

void Init_Default()
{
    N = 5;
    Init = "fast";
    maxnum = 15.0;
    PRINT = 1;
}

int Read_Options(int argc, char** argv)
{
    char* prog;
    prog = *argv;
    while (++argv, --argc > 0)
        if (**argv == '-')
            switch (*++ * argv) {
                case 'n':
                    --argc;
                    N = atoi(*++argv);
                    break;
                case 'h':
                    printf("\nHELP: try matinv -u \n\n");
                    exit(0);
                    break;
                case 'u':
                    printf("\nUsage: matinv [-n problemsize]\n");
                    printf("           [-D] show default values \n");
                    printf("           [-h] help \n");
                    printf("           [-I init_type] fast/rand \n");
                    printf("           [-m maxnum] max random no \n");
                    printf("           [-P print_switch] 0/1 \n");
                    exit(0);
                    break;
                case 'D':
                    printf("\nDefault:  n         = %d ", N);
                    printf("\n          Init      = rand");
                    printf("\n          maxnum    = 5 ");
                    printf("\n          P         = 0 \n\n");
                    exit(0);
                    break;
                case 'I':
                    --argc;
                    Init = *++argv;
                    break;
                case 'm':
                    --argc;
                    maxnum = atoi(*++argv);
                    break;
                case 'P':
                    --argc;
                    PRINT = atoi(*++argv);
                    break;
                case 'f':
                    --argc;
                    kmeans_file=*++argv;
                    break;
                case 'k':
                    --argc;
                    clusters = atoi(*++argv);
                    break;
                default:
                    printf("%s: ignored option: -%s\n", prog, *argv);
                    printf("HELP: try %s -u \n\n", prog);
                    break;
            }
    return 1;
}

int Read_Input_Params(char input[]){
    int i = 0;

    // Get the first token
    char *token = strtok(input, " ");

    // Loop through the string and get all tokens
    while (token != NULL) {
        args_arr[i++] = token;
        token = strtok(NULL, " ");
    }
    return i;
}

void Write_To_File(char* filename,char* output){
    FILE *fp;
    fp = fopen(filename, "w+");
    fputs(output, fp);
    fclose(fp);
}
