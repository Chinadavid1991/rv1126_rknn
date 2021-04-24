// Copyright 2020 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <assert.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "common/sample_common.h"
#include "rkmedia_api.h"
#include "rkmedia_venc.h"

void video_packet_cb(MEDIA_BUFFER mb) {
  printf("Get JPEG packet:ptr:%p, fd:%d, size:%zu, mode:%d, channel:%d, "
         "timestamp:%lld\n", RK_MPI_MB_GetPtr(mb), RK_MPI_MB_GetFD(mb),
         RK_MPI_MB_GetSize(mb), RK_MPI_MB_GetModeID(mb),
         RK_MPI_MB_GetChannelID(mb), RK_MPI_MB_GetTimestamp(mb));

  char jpeg_path[64];
  sprintf(jpeg_path, "./test.jpeg");
  FILE *file = fopen(jpeg_path, "w");
  if (file) {
    fwrite(RK_MPI_MB_GetPtr(mb), 1, RK_MPI_MB_GetSize(mb), file);
    fclose(file);
  }

  RK_MPI_MB_ReleaseBuffer(mb);
}



int main(int argc, char *argv[]) {
  RK_S32 ret;
  RK_U32 u32SrcWidth = 640;
  RK_U32 u32SrcHeight = 480;
  RK_U32 u32DstWidth = 640;
  RK_U32 u32DstHeight = 480;
  IMAGE_TYPE_E enPixFmt = IMAGE_TYPE_NV12;
  const RK_CHAR *pcVideoNode = "rkispp_scale0";


  ret = RK_MPI_SYS_Init();
  if (ret) {
    printf("Sys Init failed! ret=%d\n", ret);
    return -1;
  }

  VI_CHN_ATTR_S vi_chn_attr;
  vi_chn_attr.pcVideoNode = pcVideoNode;
  vi_chn_attr.u32BufCnt = 4;
  vi_chn_attr.u32Width = u32SrcWidth;
  vi_chn_attr.u32Height = u32SrcHeight;
  vi_chn_attr.enPixFmt = enPixFmt;
  vi_chn_attr.enWorkMode = VI_WORK_MODE_NORMAL;
  ret = RK_MPI_VI_SetChnAttr(0, 1, &vi_chn_attr);
  ret |= RK_MPI_VI_EnableChn(0, 1);
  if (ret) {
    printf("Create Vi failed! ret=%d\n", ret);
    return -1;
  }

  VENC_CHN_ATTR_S venc_chn_attr;
  memset(&venc_chn_attr, 0, sizeof(venc_chn_attr));
  venc_chn_attr.stVencAttr.enType = RK_CODEC_TYPE_JPEG;
  venc_chn_attr.stVencAttr.imageType = enPixFmt;
  venc_chn_attr.stVencAttr.u32PicWidth = u32SrcWidth;
  venc_chn_attr.stVencAttr.u32PicHeight = u32SrcHeight;
  venc_chn_attr.stVencAttr.u32VirWidth = u32SrcWidth;
  venc_chn_attr.stVencAttr.u32VirHeight = u32SrcHeight;
  venc_chn_attr.stVencAttr.stAttrJpege.u32ZoomWidth = u32DstWidth;
  venc_chn_attr.stVencAttr.stAttrJpege.u32ZoomHeight = u32DstHeight;
  venc_chn_attr.stVencAttr.stAttrJpege.u32ZoomVirWidth = u32DstWidth;
  venc_chn_attr.stVencAttr.stAttrJpege.u32ZoomVirHeight = u32DstHeight;
  // venc_chn_attr.stVencAttr.enRotation = VENC_ROTATION_90;
  ret = RK_MPI_VENC_CreateChn(0, &venc_chn_attr);
  if (ret) {
    printf("Create Venc failed! ret=%d\n", ret);
    return -1;
  }

  MPP_CHN_S stEncChn;
  stEncChn.enModId = RK_ID_VENC;
  stEncChn.s32ChnId = 0;
  ret = RK_MPI_SYS_RegisterOutCb(&stEncChn, video_packet_cb);
  if (ret) {
    printf("Register Output callback failed! ret=%d\n", ret);
    return -1;
  }

  // The encoder defaults to continuously receiving frames from the previous
  // stage. Before performing the bind operation, set s32RecvPicNum to 0 to
  // make the encoding enter the pause state.
  VENC_RECV_PIC_PARAM_S stRecvParam;
  stRecvParam.s32RecvPicNum = 0;
  RK_MPI_VENC_StartRecvFrame(0, &stRecvParam);

  MPP_CHN_S stSrcChn;
  stSrcChn.enModId = RK_ID_VI;
  stSrcChn.s32ChnId = 1;
  MPP_CHN_S stDestChn;
  stDestChn.enModId = RK_ID_VENC;
  stDestChn.s32ChnId = 0;
  ret = RK_MPI_SYS_Bind(&stSrcChn, &stDestChn);
  if (ret) {
    printf("Bind VI[1] to VENC[0]::JPEG failed! ret=%d\n", ret);
    return -1;
  }

 

  for(int ix = 0;ix < 10;++ix) {
    usleep(30000);
    stRecvParam.s32RecvPicNum = 1;
    ret = RK_MPI_VENC_StartRecvFrame(0, &stRecvParam);
    if (ret) {
      printf("RK_MPI_VENC_StartRecvFrame failed!\n");
      break;
    }
  }

  printf("%s exit!\n", __func__);
  RK_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
  RK_MPI_VI_DisableChn(0, 1);
  RK_MPI_VENC_DestroyChn(0);

  return 0;
}
