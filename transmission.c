//
//  main.c
//  cl_transmission
//
//  Created by Linus Yang on 12-5-6.
//  Copyright (c) 2012 Linus Yang. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#ifdef __APPLE__
#include <mach/mach.h>
#include <mach/mach_time.h>
#else
#ifdef _WIN32
#include <windows.h>
#endif
#endif

#define DATA_SIZE (2097152)
#define BUFFER_SIZE (2048)
const char *KernelSource = "\n" \
"__kernel void copy(                                                    \n" \
"   __global int* input,                                                \n" \
"   __global int* output,                                               \n" \
"   const unsigned int count)                                           \n" \
"{                                                                      \n" \
"   int i = get_global_id(0);                                           \n" \
"   if(i < count)                                                       \n" \
"       output[i] = input[i];                                           \n" \
"}                                                                      \n" \
"\n";

typedef unsigned long long uint64_t;
typedef int cl_array_type;

#ifdef __APPLE__
#define RUNTIMER(_x) *(_x) = mach_absolute_time()
double getTime(uint64_t *, uint64_t *);
double getTime(uint64_t *stime, uint64_t *etime)
{
    static mach_timebase_info_data_t sTimebaseInfo;
    if (sTimebaseInfo.denom == 0)
        (void) mach_timebase_info(&sTimebaseInfo);
    return 1e-6 * (*etime - *stime) * sTimebaseInfo.numer / sTimebaseInfo.denom;
}
#else
#ifdef _WIN32
#define RUNTIMER(_x) QueryPerformanceCounter(_x)
double getTime(LARGE_INTEGER *, LARGE_INTEGER *);
double getTime(LARGE_INTEGER *stime, LARGE_INTEGER *etime)
{
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    return 1e3 * (double) (etime->QuadPart - stime->QuadPart) / (double) frequency.QuadPart;
}
#else
#define RUNTIMER(_x) clock_gettime(CLOCK_REALTIME, _x)
double getTime(struct timespec *, struct timespec *);
double getTime(struct timespec *stime, struct timespec *etime)
{
    return (1e9 * (etime->tv_sec - stime->tv_sec) + etime->tv_nsec - stime->tv_nsec) * 1e-6;
}
#endif
#endif

int main (int argc, const char **argv)
{
    cl_int err;
    cl_uint num;
    size_t global, local, cb, len, bufflen;
    cl_device_id device_id, *devices;
    cl_context context;
    cl_command_queue commands;
    cl_program program;
    cl_kernel kernel;
    cl_mem input, output;
    cl_platform_id *platforms;
    cl_context_properties prop[] = {CL_CONTEXT_PLATFORM, 0, 0};
    cl_ulong start_kertime, end_kertime;
    cl_event event_upload, event_exec, event_download;
    int i = 0;
    unsigned int count = DATA_SIZE, correct;
    cl_array_type *data, *results;
    char *devname, buffer[BUFFER_SIZE];
    double kertime, mega_bufflen, timecost;
#ifdef __APPLE__
    uint64_t stime, etime;
#else
#ifdef _WIN32
    LARGE_INTEGER stime, etime; 
#else
    struct timespec stime, etime;
#endif
#endif
    
    printf("[OpenCL Transmission Demo - Linus Yang]\n");
    err = clGetPlatformIDs(0, 0, &num);
	if (err != CL_SUCCESS)
    {
		printf("Error: Failed to get platforms!\n");
		return EXIT_FAILURE;
	}
    platforms = (cl_platform_id *) malloc(sizeof(cl_platform_id) * num);
    err = clGetPlatformIDs(num, platforms, &num);
	if (err != CL_SUCCESS) 
    {
        printf("Error: Failed to get platform ID!\n");
		return EXIT_FAILURE;
	}
    prop[1] = (cl_context_properties) platforms[0];
    context = clCreateContextFromType(prop, CL_DEVICE_TYPE_DEFAULT, NULL, NULL, &err);
    if (!context)
    {
        printf("Error: Failed to create a compute context!\n");
        return EXIT_FAILURE;
    }
	clGetContextInfo(context, CL_CONTEXT_DEVICES, 0, NULL, &cb);
	devices = (cl_device_id *) malloc(cb);
	clGetContextInfo(context, CL_CONTEXT_DEVICES, cb, devices, 0);
    device_id = devices[0];
	clGetDeviceInfo(device_id, CL_DEVICE_NAME, 0, NULL, &cb);
	devname = (char *) malloc(cb);
	clGetDeviceInfo(device_id, CL_DEVICE_NAME, cb, devname, 0);
	printf("[Device: %s]\n", devname);
    
    srand((unsigned) time(0));
    bufflen = sizeof(cl_array_type) * count;
    results = (cl_array_type *) malloc(bufflen);
    data = (cl_array_type *) malloc(bufflen);
    for(i = 0; i < count; i++)
        results[i] = rand();
    mega_bufflen = bufflen / (1024.0 * 1024.0);
    RUNTIMER(&stime);
    memcpy(data, results, bufflen);
    RUNTIMER(&etime);
    timecost = getTime(&stime, &etime);
    printf("Step 1: %lf ms, %lf MB/s\n", timecost, 1e3 * mega_bufflen / timecost);
    
    commands = clCreateCommandQueue(context, device_id, CL_QUEUE_PROFILING_ENABLE, &err);
    if (!commands)
    {
        printf("Error: Failed to create a command commands!\n");
        return EXIT_FAILURE;
    }
    program = clCreateProgramWithSource(context, 1, (const char **) &KernelSource, NULL, &err);
    if (!program)
    {
        printf("Error: Failed to create compute program!\n");
        return EXIT_FAILURE;
    }
    err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to build program executable!\n");
        clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
        printf("%s\n", buffer);
        exit(1);
    }
    kernel = clCreateKernel(program, "copy", &err);
    if (!kernel || err != CL_SUCCESS)
    {
        printf("Error: Failed to create compute kernel!\n");
        exit(1);
    }
    input = clCreateBuffer(context,  CL_MEM_READ_ONLY,  bufflen, NULL, NULL);
    output = clCreateBuffer(context, CL_MEM_WRITE_ONLY, bufflen, NULL, NULL);
    if (!input || !output)
    {
        printf("Error: Failed to allocate device memory!\n");
        exit(1);
    }    
    err = clEnqueueWriteBuffer(commands, input, CL_TRUE, 0, bufflen, data, 0, NULL, &event_upload);
    clGetEventProfilingInfo(event_upload, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &start_kertime, NULL);
    clGetEventProfilingInfo(event_upload, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &end_kertime, NULL);
    timecost = (end_kertime - start_kertime) * 1e-6;
    printf("Step 2: %lf ms, %lf MB/s\n", timecost, 1e3 * mega_bufflen / timecost);
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to write to source array!\n");
        exit(1);
    }
    err = 0;
    err  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &input);
    err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &output);
    err |= clSetKernelArg(kernel, 2, sizeof(unsigned int), &count);
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to set kernel arguments! %d\n", err);
        exit(1);
    }
    err = clGetKernelWorkGroupInfo(kernel, device_id, CL_KERNEL_WORK_GROUP_SIZE, sizeof(local), &local, NULL);
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to retrieve kernel work group info! %d\n", err);
        exit(1);
    }
    global = count;
    err = clEnqueueNDRangeKernel(commands, kernel, 1, NULL, &global, &local, 0, NULL, &event_exec);
    if (err)
    {
        printf("Error: Failed to execute kernel!\n");
        return EXIT_FAILURE;
    }
    clFinish(commands);
    clGetEventProfilingInfo(event_exec, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &start_kertime, NULL);
    clGetEventProfilingInfo(event_exec, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &end_kertime, NULL);
    kertime = (end_kertime - start_kertime) * 1e-6;
    printf("Step 3: %lf ms, %lf MB/s\n", kertime, 1e3 * mega_bufflen / kertime);
    err = clEnqueueReadBuffer( commands, output, CL_TRUE, 0, bufflen, results, 0, NULL, &event_download);  
    clGetEventProfilingInfo(event_download, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &start_kertime, NULL);
    clGetEventProfilingInfo(event_download, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &end_kertime, NULL);
    timecost = (end_kertime - start_kertime) * 1e-6;
    printf("Step 4: %lf ms, %lf MB/s\n", timecost, 1e3 * mega_bufflen / timecost);
    
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to read output array! %d\n", err);
        exit(1);
    }
    correct = 0;
    for (i = 0; i < count; i++)
    {
        if (results[i] == data[i])
            correct++;
    }
    printf("[Check: copied '%d/%d' correct values]\n", correct, count);
    clReleaseMemObject(input);
    clReleaseMemObject(output);
    clReleaseProgram(program);
    clReleaseKernel(kernel);
    clReleaseCommandQueue(commands);
    clReleaseContext(context);
    free(platforms);
    free(devname);
    free(devices);
    free(data);
    free(results);
#ifdef _WIN32
    system("pause");
#endif
    return 0;
}
