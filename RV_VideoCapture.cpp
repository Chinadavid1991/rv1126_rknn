#include "RV_VideoCapture.h"
#include <memory.h>
#include <iostream>
#include "common/PixelConvert.h"
#include <memory.h>
#include "rtsp/rtsp.h"

static void video_packet_cb(MEDIA_BUFFER mb) {
    printf("Get JPEG packet:ptr:%p, fd:%d, size:%zu, mode:%d, channel:%d, "
           "timestamp:%lld\n",
           RK_MPI_MB_GetPtr(mb), RK_MPI_MB_GetFD(mb),
           RK_MPI_MB_GetSize(mb), RK_MPI_MB_GetModeID(mb),
           RK_MPI_MB_GetChannelID(mb), RK_MPI_MB_GetTimestamp(mb));
    FILE *file = fopen("./test.jpeg", "w");
    if (file) {
        fwrite(RK_MPI_MB_GetPtr(mb), 1, RK_MPI_MB_GetSize(mb), file);
        fclose(file);
    }

    RK_MPI_MB_ReleaseBuffer(mb);
}

void VideoCapture::getToSendBuff(RK_U32 ch_vi, RK_U32 ch_venc) {
    MEDIA_BUFFER mb = RK_MPI_SYS_GetMediaBuffer(RK_ID_VI, ch_vi, -1);
    if (!mb) {
        printf("get VI media buff failed\n");
    }
    printf("get VI media buff succeed,start to send buff to venc\n");
    // send from VI to VENC
    RK_MPI_SYS_SendMediaBuffer(RK_ID_VENC, ch_venc, mb);
    RK_MPI_MB_ReleaseBuffer(mb);
}

bool VideoCapture::start_vi(RK_U32 channel) {
    RK_U32 ret;
    RK_MPI_SYS_Init();
    //创建一个input

    ret = input(channel, VI_WORK_MODE_NORMAL);

    if (!ret) {
        printf("Create VI[0] succeed.\n");
    }
    //取流
    ret = RK_MPI_VI_StartStream(0, channel);
    if (ret) {
        printf("Start VI[0] failed! ret=%d\n", ret);
        return false;
    }
    printf("Start VI[0] succeed.\n");
    return true;
}

cv::Mat VideoCapture::buffConvertToMat(MEDIA_BUFFER mb) {
    // FILE *file = fopen("/run/usr_workspace/m.jpeg", "w");
    unsigned int  buffer_size = height * width * 3;
    auto *buffer = (unsigned char *) malloc(buffer_size);
    Mat m = Mat::zeros(height, width, CV_8UC3);
    if (pixFormat == IMAGE_TYPE_NV12) {
        NV12ConvertToRGB24((unsigned char *) RK_MPI_MB_GetPtr(mb), buffer, width, height);
        memcpy(m.data, buffer, buffer_size);
        cv::cvtColor(m, m, cv::COLOR_RGBA2BGR);
    } else if (pixFormat == IMAGE_TYPE_BGR888) {
        memcpy(m.data, buffer, buffer_size);
    } else if (pixFormat == IMAGE_TYPE_RGB888) {
        memcpy(m.data, buffer, buffer_size);
        cv::cvtColor(m, m, cv::COLOR_RGB2BGR);
    } else {
        printf("input pixFormat error!\n");
    }
    // std::cout << "get image size is:[" << m.cols << "x" << m.rows << "]" << std::endl; //1920X1080
    return m;
    // cv::cvtColor(m,m,cv::COLOR_BGR2RGB);
    //fwrite(RK_MPI_MB_GetPtr(mb), 1, RK_MPI_MB_GetSize(mb), file);
}

bool VideoCapture::imgWrite(RK_U32 ch_vi, RK_U32 ch_venc) {
    RK_U32 ret;
    ret = RK_MPI_SYS_Init();
    if (ret) {
        printf("Sys Init failed! ret=%d\n", ret);
        return false;
    }

    ret = input(ch_vi, VI_WORK_MODE_NORMAL);
    ret = encode(ch_venc, width, height);

    MPP_CHN_S stEncChn;
    stEncChn.enModId = RK_ID_VENC;
    stEncChn.s32ChnId = 0;
    ret = RK_MPI_SYS_RegisterOutCb(&stEncChn, video_packet_cb);
    if (ret) {
        printf("Register Output callback failed! ret=%d\n", ret);
        return false;
    }

    // The encoder defaults to continuously receiving frames from the previous
    // stage. Before performing the bind operation, set s32RecvPicNum to 0 to
    // make the encoding enter the pause state.
    VENC_RECV_PIC_PARAM_S stRecvParam;
    stRecvParam.s32RecvPicNum = 0;
    RK_MPI_VENC_StartRecvFrame(0, &stRecvParam);

    MPP_CHN_S stSrcChn;
    stSrcChn.enModId = RK_ID_VI;
    stSrcChn.s32ChnId = ch_vi;
    MPP_CHN_S stDestChn;
    stDestChn.enModId = RK_ID_VENC;
    stDestChn.s32ChnId = ch_venc;
    ret = RK_MPI_SYS_Bind(&stSrcChn, &stDestChn);
    if (ret) {
        printf("Bind VI[1] to VENC[0]::JPEG failed! ret=%d\n", ret);
        return false;
    }

    for (int ix = 0; ix < 10; ++ix) {
        usleep(300000);
        stRecvParam.s32RecvPicNum = 1;
        ret = RK_MPI_VENC_StartRecvFrame(0, &stRecvParam);
        if (ret) {
            printf("RK_MPI_VENC_StartRecvFrame failed!\n");
            break;
        }
    }

    RK_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
    RK_MPI_VI_DisableChn(0, ch_vi);
    RK_MPI_VENC_DestroyChn(ch_venc);

    return 0;
}

bool VideoCapture::imgWrite(const char *filename, RK_U32 ch_vi, RK_U32 ch_venc) {
    RK_U32 ret;
    ret = RK_MPI_SYS_Init();
    if (ret) {
        printf("Sys Init failed! ret=%d\n", ret);
        return false;
    }

    ret = input(ch_vi, VI_WORK_MODE_NORMAL);

    if (ret) {
        printf("vi  creat failed\n");
    }
    ret = encode(ch_venc, width, height);
    if (ret) {
        printf("venc  creat failed\n");
    }
    //取流
    ret = RK_MPI_VI_StartStream(0, ch_vi);
    if (ret) {
        printf("Start VI[0] failed! ret=%d\n", ret);
        return false;
    }
    printf("Start VI[0] succeed.\n");
    // start_vi(ch_vi);
    for (int ix : {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}) {
        getToSendBuff(ch_vi, ch_venc);
        MEDIA_BUFFER mb = RK_MPI_SYS_GetMediaBuffer(RK_ID_VENC, ch_venc, 0);
        if (!mb) {
            printf("get venc media buff failed\n");
            continue;
        }
        printf("Get JPEG packet:ptr:%p, fd:%d, size:%zu, mode:%d, channel:%d, "
               "timestamp:%lld\n",
               RK_MPI_MB_GetPtr(mb), RK_MPI_MB_GetFD(mb),
               RK_MPI_MB_GetSize(mb), RK_MPI_MB_GetModeID(mb),
               RK_MPI_MB_GetChannelID(mb), RK_MPI_MB_GetTimestamp(mb));
        FILE *file = fopen(filename, "w");
        if (file) {
            fwrite(RK_MPI_MB_GetPtr(mb), 1, RK_MPI_MB_GetSize(mb), file);
            fclose(file);
        }

        RK_MPI_MB_ReleaseBuffer(mb);
    }
    printf("finish image capture\n");
    return true;
}

bool VideoCapture::vi_rtsp(RK_U32 ch_vi, RK_U32 ch_venc) {
    const char *path = "/live/main_stream";
    printf("init rtsp\n");
    rtsp_demo_handle g_rtsplive = create_rtsp_demo(554);
    // init mpi
    printf("init mpi\n");
    RK_MPI_SYS_Init();
    rtsp_session_handle session = rtsp_new_session(g_rtsplive, path);

    input(ch_vi, VI_WORK_MODE_GOD_MODE);
    encode(ch_venc, width, height, RK_CODEC_TYPE_H264);
    RK_MPI_VI_StartStream(0, ch_vi);

    // rtsp video
    printf("rtsp video\n");
    rtsp_set_video(session, RTSP_CODEC_ID_VIDEO_H264, NULL, 0);
    rtsp_sync_video_ts(session, rtsp_get_reltime(), rtsp_get_ntptime());
    while (true) {
        getToSendBuff(ch_vi, ch_venc);
        // send video buffer
        MEDIA_BUFFER buffer = RK_MPI_SYS_GetMediaBuffer(RK_ID_VENC, ch_venc, 0);
        if (buffer) {
            rtsp_tx_video(session, (const uint8_t *) RK_MPI_MB_GetPtr(buffer),
                          RK_MPI_MB_GetSize(buffer),
                          RK_MPI_MB_GetTimestamp(buffer));
            RK_MPI_MB_ReleaseBuffer(buffer);
        }

        rtsp_do_event(g_rtsplive);
    }
    rtsp_del_demo(g_rtsplive);
}

cv::Mat VideoCapture::imgProcess(Mat img) {
    Mat _image;
    cv::Point2f P1[] = {{180, 187},
                        {430, 187},
                        {534, 381},
                        {76,  381}};
    cv::Point2f P2[] = {{0,   0},
                        {480, 0},
                        {480, 480},
                        {0,   480}};
    Mat m = cv::getPerspectiveTransform(P1, P2);
    cv::warpPerspective(img, _image, m, cv::Size(480, 480));
    cv::resize(_image, _image, cv::Size(224, 224));
    return _image;
}

MEDIA_BUFFER VideoCapture::getMediaBuffer(MOD_ID_E mode, RK_U32 channel) {
    MEDIA_BUFFER mb = RK_MPI_SYS_GetMediaBuffer(mode, channel, -1);

    if (!mb) {
        printf("RK_MPI_SYS_GetMediaBuffer get null buffer!\n");
        return nullptr;
    }
    // printf("Get Frame:ptr:%p, fd:%d, size:%zu, mode:%d, channel:%d, "
    //        "timestamp:%lld\n",
    //        RK_MPI_MB_GetPtr(mb), RK_MPI_MB_GetFD(mb), RK_MPI_MB_GetSize(mb),
    //        RK_MPI_MB_GetModeID(mb), RK_MPI_MB_GetChannelID(mb),
    //        RK_MPI_MB_GetTimestamp(mb));

    return mb;
}

void VideoCapture::NV12ConvertToRGB24(unsigned char *yuvbuffer, unsigned char *buffer,
                                      int width, int height) {
    nv12_to_rgb24(yuvbuffer, buffer, width, height);
}
