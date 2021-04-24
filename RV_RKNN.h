#ifndef _RV_RKNN_H_
#define _RV_RKNN_H_

#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <ctime>
#include "opencv2/opencv.hpp"
#include "rknn/rknn_api.h"

class MobileNet {
    typedef cv::Mat Mat;

private:
    unsigned char *m_model;
    int m_model_size;
    rknn_context m_context;
    rknn_input_output_num m_io_num;
    rknn_output m_outputs[1];

public:
    explicit MobileNet(const char *filename);

    static void print_tensor(rknn_tensor_attr *attr) {
        printf("index=%d name=%s n_dims=%d dims=[%d %d %d %d] n_elems=%d size=%d fmt=%d type=%d qnt_type=%d fl=%d zp=%d scale=%f\n",
               attr->index, attr->name, attr->n_dims, attr->dims[3], attr->dims[2], attr->dims[1], attr->dims[0],
               attr->n_elems, attr->size, 0, attr->type, attr->qnt_type, attr->fl, attr->zp, attr->scale);
    }

    bool model_init();

    bool forward(Mat img);

    bool isGlass() {
        float prob = ((float *) m_outputs[0].buf)[1];
        printf("grass probality is [%f]\n", prob);
        return prob > 0.5;
    }

    ~MobileNet() {
        delete m_model;
        // Release
        if (m_context >= 0) {
            rknn_destroy(m_context);
        }

        // Release rknn_outputs
        rknn_outputs_release(m_context, 1, m_outputs);
    };
};

#endif