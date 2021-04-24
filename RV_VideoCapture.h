
#ifndef _RV_VIDEOCAPTURE_H_
#define _RV_VIDEOCAPTURE_H_

#ifdef __cplusplus
extern "C"
{
#include <assert.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "common/sample_common.h"
#include "rkmedia_api.h"
#include "rkmedia_venc.h"
}
#endif /* __cplusplus */

#include "opencv2/opencv.hpp"

class VideoCapture {
    typedef cv::Mat Mat;

private:
    RK_U32 width = 1920;
    RK_U32 height = 1080;
    const RK_CHAR *device = "rkispp_scale0";
    IMAGE_TYPE_E pixFormat = IMAGE_TYPE_NV12;

public:
    VideoCapture(RK_U32 w, RK_U32 h, const RK_CHAR *dev, IMAGE_TYPE_E pix) : width(w), height(h), device(dev),
                                                                             pixFormat(pix) {};

    VideoCapture() = default;;

    bool start_vi(RK_U32 channel);

    void stop_vi(RK_U32 channel) {
        RK_MPI_VI_DisableChn(0, channel);
    }

    static MEDIA_BUFFER getMediaBuffer(MOD_ID_E mode, RK_U32 channel);

    Mat buffConvertToMat(MEDIA_BUFFER mb);

    bool imgWrite(RK_U32 ch_vi, RK_U32 ch_venc);

    bool imgWrite(const char *filename, RK_U32 ch_vi, RK_U32 ch_venc);

    bool vi_rtsp(RK_U32 ch_vi, RK_U32 ch_venc);

    static Mat imgProcess(Mat img);

    static void releaseBuff(MEDIA_BUFFER mb) {
        RK_MPI_MB_ReleaseBuffer(mb);
    }

    RK_U32 input(RK_U32 ch_vi, VI_CHN_WORK_MODE vi_work = VI_WORK_MODE_NORMAL, int buf_num = 4) {
        RK_U32 ret;
        VI_CHN_ATTR_S vi_chn_attr;
        vi_chn_attr.pcVideoNode = device;
        vi_chn_attr.u32BufCnt = buf_num;
        vi_chn_attr.u32Width = width;
        vi_chn_attr.u32Height = height;
        vi_chn_attr.enPixFmt = pixFormat;
        vi_chn_attr.enWorkMode = vi_work;
        ret = RK_MPI_VI_SetChnAttr(0, ch_vi, &vi_chn_attr);
        ret |= RK_MPI_VI_EnableChn(0, ch_vi);
        if (ret) {
            printf("Create Vi failed! ret=%d\n", ret);
        }
        return ret;
    }

    RK_U32 encode(RK_U32 ch_venc, RK_U32 dst_width, RK_U32 dst_height, CODEC_TYPE_E enType = RK_CODEC_TYPE_JPEG) {
        RK_U32 ret;
        VENC_CHN_ATTR_S venc_chn_attr;
        memset(&venc_chn_attr, 0, sizeof(VENC_CHN_ATTR_S));
        venc_chn_attr.stVencAttr.enType = enType;
        venc_chn_attr.stVencAttr.imageType = pixFormat;
        venc_chn_attr.stVencAttr.u32PicWidth = width;
        venc_chn_attr.stVencAttr.u32PicHeight = height;
        venc_chn_attr.stVencAttr.u32VirWidth = dst_width;
        venc_chn_attr.stVencAttr.u32VirHeight = dst_height;
        venc_chn_attr.stVencAttr.stAttrJpege.u32ZoomWidth = dst_width;
        venc_chn_attr.stVencAttr.stAttrJpege.u32ZoomHeight = dst_height;
        venc_chn_attr.stVencAttr.stAttrJpege.u32ZoomVirWidth = dst_width;
        venc_chn_attr.stVencAttr.stAttrJpege.u32ZoomVirHeight = dst_height;
        // venc_chn_attr.stVencAttr.enRotation = VENC_ROTATION_90;

        switch (venc_chn_attr.stVencAttr.enType) {
            case RK_CODEC_TYPE_H264:
                venc_chn_attr.stRcAttr.enRcMode = VENC_RC_MODE_H264CBR;
                venc_chn_attr.stRcAttr.stH264Cbr.u32Gop = 30;
                venc_chn_attr.stRcAttr.stH264Cbr.u32BitRate = width * height * 30 / 14;
                venc_chn_attr.stRcAttr.stH264Cbr.fr32DstFrameRateDen = 1;
                venc_chn_attr.stRcAttr.stH264Cbr.fr32DstFrameRateNum = 30;
                venc_chn_attr.stRcAttr.stH264Cbr.u32SrcFrameRateDen = 1;
                venc_chn_attr.stRcAttr.stH264Cbr.u32SrcFrameRateNum = 30;
                break;
            case RK_CODEC_TYPE_H265:
                venc_chn_attr.stRcAttr.enRcMode = VENC_RC_MODE_H265VBR;
                venc_chn_attr.stRcAttr.stH265Vbr.u32Gop = 30;
                venc_chn_attr.stRcAttr.stH265Vbr.u32MaxBitRate = width * height * 30 / 14;
                venc_chn_attr.stRcAttr.stH265Vbr.fr32DstFrameRateDen = 1;
                venc_chn_attr.stRcAttr.stH265Vbr.fr32DstFrameRateNum = 30;
                venc_chn_attr.stRcAttr.stH265Vbr.u32SrcFrameRateDen = 1;
                venc_chn_attr.stRcAttr.stH265Vbr.u32SrcFrameRateNum = 30;
                break;
            default:
                break;
        }
        ret = RK_MPI_VENC_CreateChn(ch_venc, &venc_chn_attr);
        if (ret) {
            printf("Create Venc failed! ret=%d\n", ret);
        }
        return ret;
    }

    static void NV12ConvertToRGB24(unsigned char *yuvbuffer, unsigned char *rga_buffer,
                                   int width, int height);

    ~VideoCapture() = default;;

private:
    static void getToSendBuff(RK_U32 ch_vi, RK_U32 ch_venc);
};

#endif
