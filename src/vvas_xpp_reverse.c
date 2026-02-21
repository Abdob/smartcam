/*
 * Copyright 2021 Xilinx, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <vvas/vvas_kernel.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

enum {
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG
};

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define LOG_MESSAGE(level, ...) {\
  do {\
    char *str; \
    if (level == LOG_LEVEL_ERROR) str = (char*)"ERROR";\
    else if (level == LOG_LEVEL_WARNING) str = (char*)"WARNING";\
    else if (level == LOG_LEVEL_INFO) str = (char*)"INFO";\
    else if (level == LOG_LEVEL_DEBUG) str = (char*)"DEBUG";\
    if (level <= kernel_priv->log_level) {\
      printf("[%s %s:%d] %s: ",__FILENAME__, __func__, __LINE__, str);\
      printf(__VA_ARGS__);\
      printf("\n");\
    }\
  } while (0); \
}

typedef struct _kern_priv {
    int log_level;
} GrayKernelPriv;

int32_t xlnx_kernel_start(VVASKernel *handle, int start, VVASFrame *input[MAX_NUM_OBJECT], VVASFrame *output[MAX_NUM_OBJECT]);
int32_t xlnx_kernel_done(VVASKernel *handle);
int32_t xlnx_kernel_init(VVASKernel *handle);
uint32_t xlnx_kernel_deinit(VVASKernel *handle);

uint32_t xlnx_kernel_deinit(VVASKernel *handle) {
    GrayKernelPriv *kernel_priv = (GrayKernelPriv *)handle->kernel_priv;
    if (kernel_priv) free(kernel_priv);
    return 0;
}

int32_t xlnx_kernel_init(VVASKernel *handle) {
    json_t *jconfig = handle->kernel_config;
    json_t *val;
    GrayKernelPriv *kernel_priv;

    handle->is_multiprocess = 0;
    kernel_priv = (GrayKernelPriv *)calloc(1, sizeof(GrayKernelPriv));
    if (!kernel_priv) return -1;

    val = json_object_get(jconfig, "debug_level");
    if (!val || !json_is_integer(val))
        kernel_priv->log_level = LOG_LEVEL_WARNING;
    else
        kernel_priv->log_level = json_integer_value(val);

    handle->kernel_priv = (void *)kernel_priv;
    LOG_MESSAGE(LOG_LEVEL_INFO, "Minimal Gray Kernel Initialized");
    return 0;
}

int32_t xlnx_kernel_start(VVASKernel *handle, int start, VVASFrame *input[MAX_NUM_OBJECT], VVASFrame *output[MAX_NUM_OBJECT]) {
    GrayKernelPriv *kernel_priv = (GrayKernelPriv *)handle->kernel_priv;

    // 1. Extract Stride and Dimensions
    uint32_t in_stride  = input[0]->props.stride;
    uint32_t out_stride = output[0]->props.stride;
    uint32_t width      = input[0]->props.width;
    uint32_t height     = input[0]->props.height;

    // 2. Log the values to the console
    LOG_MESSAGE(LOG_LEVEL_INFO, "DIMENSIONS: %dx%d", width, height);

    int ret = vvas_kernel_start(handle, "pppuu",
        input[0]->paddr[0],
        output[0]->paddr[0],
        output[0]->paddr[1],
        input[0]->props.width,
        output[0]->props.stride
    );

    if (ret < 0) {
        LOG_MESSAGE(LOG_LEVEL_ERROR, "Failed to issue execute command");
        return ret;
    }

    ret = vvas_kernel_done(handle, 1000);
    if (ret < 0) {
        LOG_MESSAGE(LOG_LEVEL_ERROR, "Kernel timeout - Hardware hang detected");
        return ret;
    }

    return 0;
}

int32_t xlnx_kernel_done(VVASKernel *handle) {
    return 0;
}