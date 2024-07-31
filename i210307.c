#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image.h"
#include "stb_image_write.h"

void convert_to_grayscale(const unsigned char *inputImage, unsigned char *outputImage, int width, int height, int channels)
{
    cl_int err;
    cl_platform_id platform;
    cl_device_id device;
    cl_context context;
    cl_command_queue queue;
    cl_program program;
    cl_kernel kernel;
    cl_mem inputBuffer, outputBuffer;

    // Initialize OpenCL
    err = clGetPlatformIDs(1, &platform, NULL);
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_DEFAULT, 1, &device, NULL);
    if (err != CL_SUCCESS)
    {
        printf("Error getting platform or device.\n");
        return;
    }

    // Create a context
    context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    if (err != CL_SUCCESS)
    {
        printf("Error creating context.\n");
        return;
    }

    // Create a command queue
    queue = clCreateCommandQueue(context, device, 0, &err);
    if (err != CL_SUCCESS)
    {
        printf("Error creating command queue.\n");
        clReleaseContext(context);
        return;
    }

    // Create memory buffers
    inputBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(unsigned char) * width * height * channels, (void *)inputImage, &err);
    outputBuffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(unsigned char) * width * height, NULL, &err);

    // Load kernel source
    const char *source = "__kernel void RGB_to_Gray(__global const unsigned char *inputImage, "
                        "__global unsigned char *outputImage, const int width, const int height, const int channels) "
                        "{ "
                            "int y = get_global_id(0); "
                            "int x = get_global_id(1); "
                            "int idx = (y * width + x) * channels;"
                            "if (y < height && x < width)"
                            "{ "
                                "unsigned char grayscale = 0; "
                                "unsigned char r = inputImage[idx + 0]; " 
                                "unsigned char g = inputImage[idx + 1]; " 
                                "unsigned char b = inputImage[idx + 2]; "
                                "grayscale = (unsigned char)(0.299f * r + 0.587f * g + 0.114f * b); "
                                "outputImage[y * width + x] = grayscale; "
                            "} "
                        "} ";

    
    program = clCreateProgramWithSource(context, 1, &source, NULL, &err);
    if (err != CL_SUCCESS)
    {
        printf("Error creating program.\n");
        clReleaseMemObject(inputBuffer);
        clReleaseMemObject(outputBuffer);
        clReleaseCommandQueue(queue);
        clReleaseContext(context);
        return;
    }

    // Build the program
    err = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        printf("Error building program.\n");
        clReleaseProgram(program);
        clReleaseMemObject(inputBuffer);
        clReleaseMemObject(outputBuffer);
        clReleaseCommandQueue(queue);
        clReleaseContext(context);
        return;
    }

    // Create kernel
    kernel = clCreateKernel(program, "RGB_to_Gray", &err);
    if (err != CL_SUCCESS)
    {
        printf("Error creating kernel.\n");
        clReleaseProgram(program);
        clReleaseMemObject(inputBuffer);
        clReleaseMemObject(outputBuffer);
        clReleaseCommandQueue(queue);
        clReleaseContext(context);
        return;
    }

    // Set kernel arguments
    clSetKernelArg(kernel, 0, sizeof(cl_mem), &inputBuffer);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), &outputBuffer);
    clSetKernelArg(kernel, 2, sizeof(int), &width);
    clSetKernelArg(kernel, 3, sizeof(int), &height);
    clSetKernelArg(kernel, 4, sizeof(int), &channels);

    // Execute kernel
    size_t globalSize[2] = {height, width};
    err = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, globalSize, NULL, 0, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        printf("Error executing kernel.\n");
        clReleaseKernel(kernel);
        clReleaseProgram(program);
        clReleaseMemObject(inputBuffer);
        clReleaseMemObject(outputBuffer);
        clReleaseCommandQueue(queue);
        clReleaseContext(context);
        return;
    }

    // Read the output buffer
    err = clEnqueueReadBuffer(queue, outputBuffer, CL_TRUE, 0, sizeof(unsigned char) * width * height, outputImage, 0, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        printf("Error reading output buffer.\n");
        clReleaseKernel(kernel);
        clReleaseProgram(program);
        clReleaseMemObject(inputBuffer);
        clReleaseMemObject(outputBuffer);
        clReleaseCommandQueue(queue);
        clReleaseContext(context);
        return;
    }

    // Clean up
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseMemObject(inputBuffer);
    clReleaseMemObject(outputBuffer);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
}

int main()
{
    const char *inputFolder = "ISIC_2020_Test_Input";
    const char *outputFolder = "ISIC_2020_Test_Output";

    DIR *dir = opendir(inputFolder);
    if (!dir)
    {
        perror("Error opening input folder");
        return 1;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        // Skip "." and ".." entries
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char inputPath[257]; // 1 for null terminator
        snprintf(inputPath, sizeof(inputPath), "%s/%s", inputFolder, entry->d_name);

        char outputPath[257]; // 1 for null terminator
        snprintf(outputPath, sizeof(outputPath), "%s/%s", outputFolder, entry->d_name);


        int width, height, channels;
        unsigned char *inputImage = stbi_load(inputPath, &width, &height, &channels, 0);
        if (inputImage)
        {
            printf("Image loaded successfully: %s\n", inputPath);

            unsigned char *outputImage = (unsigned char *)malloc(width * height * sizeof(unsigned char));

            convert_to_grayscale(inputImage, outputImage, width, height, channels);

            int success = stbi_write_png(outputPath, width, height, 1, outputImage, width * 1);
            if (success)
            {
                printf("Grayscale image saved: %s\n", outputPath);
            }
            else
            {
                printf("Error writing grayscale image: %s\n", outputPath);
            }

            stbi_image_free(inputImage);
            free(outputImage);
        }
        else
        {
            printf("Error loading image: %s\n", inputPath);
        }
    }

    closedir(dir);

    return 0;
}
