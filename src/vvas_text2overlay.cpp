#include <vvas/vvas_kernel.h>
#include <stdio.h>
#include <stdlib.h>

extern "C"
{
    int32_t xlnx_kernel_init(VVASKernel *handle)
    {
        printf("init kernel\n");
        return 0;
    }

    uint32_t xlnx_kernel_start(VVASKernel *handle, int start,
                               VVASFrame *input[MAX_NUM_OBJECT], 
                               VVASFrame *output[MAX_NUM_OBJECT])
    {
        printf("exec kernel\n");
        return 0;
    }

    int32_t xlnx_kernel_done(VVASKernel *handle)
    {
        printf("done\n");
        return 0;
    }

    uint32_t xlnx_kernel_deinit(VVASKernel *handle)
    {
        return 0;
    }
}