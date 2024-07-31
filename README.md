# RGB-images-to-Grayscale-using-OpenCL

## Overview

This project demonstrates the conversion of RGB images to grayscale using OpenCL. The host program initializes OpenCL, loads images, processes them to grayscale, and saves the results. This project showcases parallel and distributed computing techniques applied to image processing.

## Host Program Description

The host program performs the following tasks:

1. **Initialization**:
   - Sets up OpenCL environment including context, command queue, and memory buffers.
   - Loads RGB images from an input directory and saves the grayscale images to an output directory.

2. **Processing**:
   - Uses an OpenCL kernel to perform the grayscale conversion.
   - Handles memory cleanup and error management throughout the process.

3. **Configuration**:
   - Manages image dimensions and allocates memory for processing.
   - Utilizes the STB Image library for loading and saving images.

4. **OpenCL Workflow**:
   - Initializes OpenCL platform, device, context, and command queue.
   - Compiles and executes the grayscale conversion kernel.
   - Reads the processed grayscale image data and saves the results.

## Grayscale Conversion Algorithm

The conversion from RGB to grayscale is performed using the luminosity method:
```
Gray = 0.299 * Red + 0.587 * Green + 0.114 * Blue
```
The kernel processes each pixel of the input image to compute its grayscale value based on the weighted sum of the RGB components.

## Compilation and Execution

To compile and run the project, ensure you have the OpenCL library and the STB Image library available. Use the following commands:

1. **Compile**:
   ```
   gcc -o rgb_to_gray rgb_to_gray.c -lOpenCL -lm
   ```

2. **Run**:
   ```
   ./rgb_to_gray
   ```

## Directory Structure

- **`ISIC_2020_Test_Input/`**: Directory containing RGB images to be processed.
- **`ISIC_2020_Test_Output/`**: Directory where grayscale images will be saved.

## Dependencies

- **OpenCL**: For parallel processing.
- **STB Image Library**: For image loading and saving.

Include the following headers in your project:
- `stb_image.h`
- `stb_image_write.h`

## Results

The host program successfully processes each RGB image in the input directory, converts it to grayscale using OpenCL, and saves the result in the output directory. Results can be verified by checking the output directory for grayscale images.
