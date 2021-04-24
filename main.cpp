#include "RV_VideoCapture.h"
#include "RV_RKNN.h"
#include <iostream>
#include <chrono>
#include <thread>
#include "opencv2/opencv.hpp"
using namespace std;


int main(){
    MobileNet net("/oem/model/mobilenet.rknn");
    net.model_init();
    RK_U32 ch_vi = 0;
    VideoCapture cap(640, 480, "rkispp_scale0", IMAGE_TYPE_NV12);
    cap.start_vi(ch_vi);
    while (true)
    {
        MEDIA_BUFFER mb =  cap.getMediaBuffer(RK_ID_VI,ch_vi);
        if(!mb){
            printf("media buff is empty\n");
        }
        cv::Mat m = cap.buffConvertToMat(mb);
        m = cap.imgProcess(m);
        net.forward(m);
        if(net.isGlass()){
            printf("is glass\n");
        }
        else{
            printf("not glass\n");
        }
        cap.releaseBuff(mb);
    }
    return 0;
}