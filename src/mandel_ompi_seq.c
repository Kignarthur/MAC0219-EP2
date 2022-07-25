#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define  MASTER 0

int PROCESSES;

double X_MIN;
double X_MAX;
double Y_MIN;
double Y_MAX;

double pixel_width;
double pixel_height;

int MAX_ITER = 200;

int image_size;
unsigned char **image_buffer;

int IMAGE_WIDTH;
int IMAGE_HEIGHT;
int image_buffer_size;

int gradient_size = 16;
int colors[17][3] = {
                        {66, 30, 15},
                        {25, 7, 26},
                        {9, 1, 47},
                        {4, 4, 73},
                        {0, 7, 100},
                        {12, 44, 138},
                        {24, 82, 177},
                        {57, 125, 209},
                        {134, 181, 229},
                        {211, 236, 248},
                        {241, 233, 191},
                        {248, 201, 95},
                        {255, 170, 0},
                        {204, 128, 0},
                        {153, 87, 0},
                        {106, 52, 3},
                        {16, 16, 16},
                    };

void allocate_image_buffer(){
    int rgb_size = 3;
    image_buffer = (unsigned char **) malloc(sizeof(unsigned char *) * image_buffer_size);

    for(int i = 0; i < image_buffer_size; i++){
        image_buffer[i] = (unsigned char *) malloc(sizeof(unsigned char) * rgb_size);
    };
};

void free_image_buffer(){
    for(int i = 0; i < image_buffer_size; i++)
        free(image_buffer[i]);
    free(image_buffer);
}

void init(int argc, char *argv[]){
    if(argc < 6){
        printf("usage: ./mandel_ompi_seq c_x_min c_x_max c_y_min c_y_max image_size\n");
        printf("examples with image_size = 11500:\n");
        printf("    Full Picture:         ./mandel_ompi_seq -2.5 1.5 -2.0 2.0 11500\n");
        printf("    Seahorse Valley:      ./mandel_ompi_seq -0.8 -0.7 0.05 0.15 11500\n");
        printf("    Elephant Valley:      ./mandel_ompi_seq 0.175 0.375 -0.1 0.1 11500\n");
        printf("    Triple Spiral Valley: ./mandel_ompi_seq -0.188 -0.012 0.554 0.754 11500\n");
        exit(0);
    }
    else{
        sscanf(argv[1], "%lf", &X_MIN);
        sscanf(argv[2], "%lf", &X_MAX);
        sscanf(argv[3], "%lf", &Y_MIN);
        sscanf(argv[4], "%lf", &Y_MAX);
        sscanf(argv[5], "%d", &image_size);

        IMAGE_WIDTH       = image_size;
        IMAGE_HEIGHT      = image_size;
        image_buffer_size = image_size * image_size;

        pixel_width       = (X_MAX - X_MIN) / IMAGE_WIDTH;
        pixel_height      = (Y_MAX - Y_MIN) / IMAGE_HEIGHT;
    };
};

void update_rgb_buffer(int iteration, int x, int y){
    int color;

    if(iteration == MAX_ITER){
        image_buffer[(IMAGE_HEIGHT * y) + x][0] = colors[gradient_size][0];
        image_buffer[(IMAGE_HEIGHT * y) + x][1] = colors[gradient_size][1];
        image_buffer[(IMAGE_HEIGHT * y) + x][2] = colors[gradient_size][2];
    }
    else{
        color = iteration % gradient_size;

        image_buffer[(IMAGE_HEIGHT * y) + x][0] = colors[color][0];
        image_buffer[(IMAGE_HEIGHT * y) + x][1] = colors[color][1];
        image_buffer[(IMAGE_HEIGHT * y) + x][2] = colors[color][2];
    };
};

void write_to_file(){
    FILE * file;
    char * filename = "output.ppm";
    char * comment  = "# ";

    int max_color_component_value = 255;

    file = fopen(filename,"wb");

    fprintf(file, "P6\n %s\n %d\n %d\n %d\n", comment,
            IMAGE_WIDTH, IMAGE_HEIGHT, max_color_component_value);

    for(int i = 0; i < image_buffer_size; i++){
        fwrite(image_buffer[i], 1 , 3, file);
    };

    fclose(file);
};

void *compute_mandelbrot(int taskid){
    double z_x;
    double z_y;
    double z_x_squared;
    double z_y_squared;
    double escape_radius_squared = 4;

    int iteration;
    int x_i;
    int y_i;

    double c_x;
    double c_y;

    int y_start = taskid * IMAGE_HEIGHT / PROCESSES;
    int y_end = (taskid+1) * IMAGE_HEIGHT / PROCESSES;

    for(y_i = y_start; y_i < y_end; y_i++){
        for(x_i = 0; x_i < IMAGE_WIDTH; x_i++){
            c_y = Y_MIN + y_i * pixel_height;
            c_x = X_MIN + x_i * pixel_width;

            if(fabs(c_y) < pixel_height / 2) c_y = 0.0;

            z_x = 0.0;
            z_y = 0.0;

            z_x_squared = 0.0;
            z_y_squared = 0.0;

            for(iteration = 0;
                iteration < MAX_ITER && \
                ((z_x_squared + z_y_squared) < escape_radius_squared);
                iteration++){
                z_y         = 2 * z_x * z_y + c_y;
                z_x         = z_x_squared - z_y_squared + c_x;

                z_x_squared = z_x * z_x;
                z_y_squared = z_y * z_y;
            };
        };
    };
};

int main(int argc, char *argv[]){
    init(argc, argv);

    int numtasks, taskid;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &taskid);

    PROCESSES = numtasks;

    compute_mandelbrot(taskid);

    MPI_Finalize();
};
