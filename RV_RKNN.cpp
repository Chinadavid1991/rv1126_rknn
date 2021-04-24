#include "RV_RKNN.h"
#include <memory.h>

MobileNet::MobileNet(const char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (fp == nullptr) {
        printf("fopen %s fail!\n", filename);
    }
    fseek(fp, 0, SEEK_END);
    int model_len = ftell(fp);
    m_model = (unsigned char *) malloc(model_len);
    fseek(fp, 0, SEEK_SET);
    if (model_len != fread(m_model, 1, model_len, fp)) {
        printf("fread %s fail!\n", filename);
        free(m_model);
    }
    m_model_size = model_len;
    if (fp) {
        fclose(fp);
    }
    printf("read model file succeed\n");
}

bool MobileNet::model_init() {
    int ret = rknn_init(&m_context, m_model, m_model_size, 0);
    if (ret < 0) {
        printf("rknn_init fail! ret=%d\n", ret);
        return false;
    }
    // Get Model Input Output Info
    ret = rknn_query(m_context, RKNN_QUERY_IN_OUT_NUM, &m_io_num, sizeof(m_io_num));
    if (ret != RKNN_SUCC) {
        printf("rknn_query fail! ret=%d\n", ret);
        return false;
    }
    printf("model input num: %d, output num: %d\n", m_io_num.n_input, m_io_num.n_output);

    printf("input tensors:\n");
    rknn_tensor_attr input_attrs[m_io_num.n_input];
    memset(input_attrs, 0, sizeof(input_attrs));
    for (int i = 0; i < m_io_num.n_input; i++) {
        input_attrs[i].index = i;
        ret = rknn_query(m_context, RKNN_QUERY_INPUT_ATTR, &(input_attrs[i]), sizeof(rknn_tensor_attr));
        if (ret != RKNN_SUCC) {
            printf("rknn_query fail! ret=%d\n", ret);
            return false;
        }
        print_tensor(&(input_attrs[i]));
    }

    printf("output tensors:\n");
    rknn_tensor_attr output_attrs[m_io_num.n_output];
    memset(output_attrs, 0, sizeof(output_attrs));
    for (int i = 0; i < m_io_num.n_output; i++) {
        output_attrs[i].index = i;
        ret = rknn_query(m_context, RKNN_QUERY_OUTPUT_ATTR, &(output_attrs[i]), sizeof(rknn_tensor_attr));
        if (ret != RKNN_SUCC) {
            printf("rknn_query fail! ret=%d\n", ret);
            return false;
        }
        print_tensor(&(output_attrs[i]));
    }
    return true;
}

bool MobileNet::forward(Mat img) {
    rknn_input inputs[1];
    memset(inputs, 0, sizeof(inputs));
    inputs[0].index = 0;
    inputs[0].type = RKNN_TENSOR_UINT8;
    inputs[0].size = img.cols * img.rows * img.channels();
    inputs[0].fmt = RKNN_TENSOR_NHWC;
    inputs[0].buf = img.data;

    int ret = rknn_inputs_set(m_context, m_io_num.n_input, inputs);
    if (ret < 0) {
        printf("rknn_input_set fail! ret=%d\n", ret);
        return false;
    }

    // Run
    printf("rknn_run\n");
    ret = rknn_run(m_context, nullptr);
    if (ret < 0) {
        printf("rknn_run fail! ret=%d\n", ret);
        return false;
    }

    // Get Output

    memset(m_outputs, 0, sizeof(m_outputs));
    m_outputs[0].want_float = 1;

    ret = rknn_outputs_get(m_context, 1, m_outputs, NULL);
    if (ret < 0) {
        printf("rknn_outputs_get fail! ret=%d\n", ret);
        return false;
    }
    return true;

}