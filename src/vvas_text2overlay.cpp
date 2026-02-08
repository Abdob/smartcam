/*
 * Copyright 2021-2022 Xilinx, Inc.
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

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <vvas/vvas_kernel.h>
#include <xrt/xrt_device.h>
#include <xrt/xrt_bo.h>

int log_level;
using namespace cv;
using namespace std;

enum
{
  LOG_LEVEL_ERROR,
  LOG_LEVEL_WARNING,
  LOG_LEVEL_INFO,
  LOG_LEVEL_DEBUG
};

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define LOG_MESSAGE(level, ...) {\
  do {\
    char *str; \
    if (level == LOG_LEVEL_ERROR)\
      str = (char*)"ERROR";\
    else if (level == LOG_LEVEL_WARNING)\
      str = (char*)"WARNING";\
    else if (level == LOG_LEVEL_INFO)\
      str = (char*)"INFO";\
    else if (level == LOG_LEVEL_DEBUG)\
      str = (char*)"DEBUG";\
    if (level <= log_level) {\
      printf("[%s %s:%d] %s: ",__FILENAME__, __func__, __LINE__, str);\
      printf(__VA_ARGS__);\
      printf("\n");\
    }\
  } while (0); \
}

struct overlayframe_info
{
  VVASFrame *inframe;
  Mat lumaImg;
};

struct vvas_xoverlaypriv
{
  float font_size;
  unsigned int font;
  unsigned int y_offset;
  unsigned int x_offset;
  struct overlayframe_info frameinfo;
};

extern "C"
{
  int32_t xlnx_kernel_init (VVASKernel * handle)
  {
    LOG_MESSAGE (LOG_LEVEL_DEBUG, "enter");

    vvas_xoverlaypriv *kpriv =
        (vvas_xoverlaypriv *) malloc (sizeof (vvas_xoverlaypriv));
    memset (kpriv, 0, sizeof (vvas_xoverlaypriv));

    json_t *jconfig = handle->kernel_config;
    json_t *val;

    val = json_object_get (jconfig, "debug_level");
    if (!val || !json_is_integer (val))
        log_level = LOG_LEVEL_WARNING;
    else
        log_level = json_integer_value (val);

    val = json_object_get (jconfig, "font_size");
    if (!val || !json_is_number (val))
        kpriv->font_size = 0.5;
    else
        kpriv->font_size = json_number_value (val);

    val = json_object_get (jconfig, "font");
    if (!val || !json_is_integer (val))
        kpriv->font = 0;
    else
        kpriv->font = json_integer_value (val);

    val = json_object_get (jconfig, "y_offset");
    if (!val || !json_is_integer (val))
        kpriv->y_offset = 30;
    else
        kpriv->y_offset = json_integer_value (val);

    val = json_object_get (jconfig, "x_offset");
    if (!val || !json_is_integer (val))
        kpriv->x_offset = 800;
    else
        kpriv->x_offset = json_integer_value (val);

    handle->kernel_priv = (void *) kpriv;
    return 0;
  }

  uint32_t xlnx_kernel_deinit (VVASKernel * handle)
  {
    LOG_MESSAGE (LOG_LEVEL_DEBUG, "enter");
    vvas_xoverlaypriv *kpriv = (vvas_xoverlaypriv *) handle->kernel_priv;

    if (kpriv)
        free (kpriv);

    return 0;
  }

  uint32_t xlnx_kernel_start (VVASKernel * handle, int start,
      VVASFrame * input[MAX_NUM_OBJECT], VVASFrame * output[MAX_NUM_OBJECT])
  {
    vvas_xoverlaypriv *kpriv = (vvas_xoverlaypriv *) handle->kernel_priv;
    struct overlayframe_info *frameinfo = &(kpriv->frameinfo);
    frameinfo->inframe = input[0];

    char *lumaBuf = (char *) frameinfo->inframe->vaddr[0];

    // Create Mat exactly as before
    frameinfo->lumaImg.create (input[0]->props.height, input[0]->props.stride/4, CV_8UC4);
    frameinfo->lumaImg.data = (unsigned char *) lumaBuf;

    char text_buffer[512] = "hello";
    
    /* Draw static "hello" on the frame */
    putText(frameinfo->lumaImg, text_buffer, cv::Point(kpriv->x_offset, kpriv->y_offset), kpriv->font,
            kpriv->font_size, Scalar (255.0, 255.0, 255.0), 1, 1);

    return 0;
  }

  int32_t xlnx_kernel_done (VVASKernel * handle)
  {
      return 0;
  }
}