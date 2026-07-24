#ifndef MODEL_H
#define MODEL_H

#include <cstdint>
#include <cstddef>
#include <type_traits>

extern "C" {
#include "ai_platform.h"
}

// Class to abstract STMCUBEAI C API process (Supports only 1 input and output tensor and 1 activation pool)
template<typename MParams>
class Model{
public:
    Model(){}

    ~Model()
    {
        if(network!=AI_HANDLE_NULL)
            MParams::destroy(network);
    }

    //Model Info
    //Tensor info cached at init (avoids calling get_report on every access)
    enum class TensorFormat {
        Float     = 0, // floating-point data. mapped on a 32b float C-type (ai_float or float).
        Quantized = 1, // quantized data, mapped on 8b signed or unsigned integer C-type.
        Bool      = 2,  // boolean, mapped on 8b unsigned integer C-type
        Unknown
    };

    struct QuantInfo{
        bool         has_quant     = false;
        float        scale         = 0.0f;   // válido só se Quantized
        int32_t      zero_point    = 0;      // válido só se Quantized
    };

    struct TensorInfo {
        uint16_t     height        = 0;
        uint16_t     width         = 0;
        uint16_t     ch            = 0;
        uint16_t     size          = 0;
        uint32_t     size_in_bytes = 0;
        TensorFormat format        = TensorFormat::Unknown;
        bool         is_signed     = false;
        uint32_t     bits          = 0;
        QuantInfo    q_info;
    };

    struct Error{
        int code; //ai_error_code
        int type; //ai_error_type
    };

    int init(){

        if(ready_) return 0;

        ai_error err = MParams::create_and_init(&network, data_activations, NULL);
        if (err.type != AI_ERROR_NONE) {
            return -1;
        }

        ai_in_buf_handle  = MParams::inputs_get(network, NULL);
        ai_out_buf_handle = MParams::outputs_get(network, NULL);

        if constexpr (MParams::INPUTS_IN_ACTIVATION)
        {
            //The input tensor will utilize the activation tensor
            for (int t_num=0; t_num < MParams::num_tensors_in; t_num++) {
                p_tensors_in[t_num] = (ai_i8*) ai_in_buf_handle[t_num].data;
            } 
        }
        else
        {
            //Need to create the input tensor
            p_tensors_in[0] = data_in_1.data;

            for (int t_num=0; t_num < MParams::num_tensors_in; t_num++) {
	            ai_in_buf_handle[t_num].data = p_tensors_in[t_num];
            }
        }
        if constexpr (MParams::OUTPUTS_IN_ACTIVATION)
        {
            //The input tensor will utilize the activation tensor
            for (int t_num=0; t_num < MParams::num_tensors_out; t_num++) {
                p_tensors_out[t_num] = (ai_i8*) ai_out_buf_handle[t_num].data;
            } 
        }
        else
        {
            //Need to create the input tensor
            p_tensors_out[0] = data_out_1.data;

            for (int t_num=0; t_num < MParams::num_tensors_out; t_num++) {
	            ai_out_buf_handle[t_num].data = p_tensors_out[t_num];
            }
        }

        ai_network_report report{};
        if (MParams::get_report(network, &report)) {
            in_info_  = describe(&report.inputs[0]);
            out_info_ = describe(&report.outputs[0]);
        }

        ready_ = true;
        return 0; 
    }

    int run(){

        if(!ready_)  return -1;

        ai_i32 batch;

        batch = MParams::run(
            network, 
            ai_in_buf_handle, 
            ai_out_buf_handle);

        if (batch != 1) return -2;   
        return 0;
    }

    Error get_error() {
        Error err;
        ai_error e = MParams::get_error(network);
        err.code = e.code;
        err.type = e.type; 
        return err;
    }

    uint8_t* get_input_tensor(){
        return (uint8_t *) p_tensors_in[0];
    }

    uint8_t* get_output_tensor(){
        return (uint8_t *) p_tensors_out[0];
    }

    const TensorInfo& get_input_tensor_info()  const { return in_info_;  }

    const TensorInfo& get_output_tensor_info() const { return out_info_; }

    //float -> int8, writes into the input tensor
    void quantize_input(const float* src, int n) {
        if (!in_info_.q_info.has_quant) return;

        int8_t* dst = reinterpret_cast<int8_t*>(p_tensors_in[0]);
        const float inv_scale = 1.0f / in_info_.q_info.scale;

        for (int i = 0; i < n; i++) {
            int32_t v = in_info_.q_info.zero_point
                      + (int32_t)(src[i] * inv_scale + (src[i] < 0 ? -0.5f : 0.5f));
            if (v < -128) v = -128;
            if (v >  127) v =  127;
            dst[i] = (int8_t)v;
        }
    }

    // int8 → float, lendo do tensor de saída
    void dequantize_output(float* dst, int n) {
        if (!out_info_.q_info.has_quant) return;

        const int8_t* src = reinterpret_cast<const int8_t*>(p_tensors_out[0]);
        for (int i = 0; i < n; i++)
            dst[i] = out_info_.q_info.scale * ((float)src[i] - out_info_.q_info.zero_point);
    }


private:
    static_assert(MParams::num_tensors_in == 1,  "Model supports only 1 input tensor");
    static_assert(MParams::num_tensors_out == 1, "Model supports only 1 output tensor");
    static_assert(MParams::num_activations == 1, "Model supports only 1 activation pool");

    //AI
    ai_handle network = AI_HANDLE_NULL; // Model handle
    ai_buffer* ai_in_buf_handle  = nullptr;    // Input handle
    ai_buffer* ai_out_buf_handle = nullptr;    // Output handle

    ai_i8* p_tensors_in[MParams::num_tensors_in]   = {nullptr}; // Array of pointers to the input tensors
    ai_i8* p_tensors_out[MParams::num_tensors_out] = {nullptr}; // Array of pointers to the output tensors

    alignas(32)
    uint8_t pool1[MParams::size_activation1]; //pool -> pool of bytes

    //Activation Buffer handle (Increase with more activations)
    ai_handle data_activations[MParams::num_activations] = {pool1};

    //Class control
    bool ready_ = false;

    //Tensor Info
    TensorInfo in_info_;
    TensorInfo out_info_;

    //Class Aligment control 
    static constexpr size_t DCACHE_LINE = 32;
    static constexpr size_t round_up(size_t n, size_t align) {
        return (n + align - 1) & ~(align - 1);
    }
    struct NoBuffer {};
    template<size_t Size>
    struct AlignedTensorBuffer {
        alignas(DCACHE_LINE) ai_i8 data[round_up(Size, DCACHE_LINE)];
    };
    std::conditional_t<MParams::INPUTS_IN_ACTIVATION,
                        NoBuffer,
                        AlignedTensorBuffer<MParams::size_tensor_in1>>  data_in_1{};
    std::conditional_t<MParams::OUTPUTS_IN_ACTIVATION,
                        NoBuffer,
                        AlignedTensorBuffer<MParams::size_tensor_out1>> data_out_1{};

    static TensorFormat map_format(ai_buffer_format fmt) {
        switch (AI_BUFFER_FMT_GET_TYPE(fmt)) {
            case AI_BUFFER_FMT_TYPE_FLOAT: return TensorFormat::Float;
            case AI_BUFFER_FMT_TYPE_Q:     return TensorFormat::Quantized;
            case AI_BUFFER_FMT_TYPE_BOOL:  return TensorFormat::Bool;
            default:                       return TensorFormat::Unknown;
        }
    }

    static TensorInfo describe(const ai_buffer* buf) {
        TensorInfo ti{};
        if (!buf) return ti;

        const ai_buffer_format fmt = AI_BUFFER_FORMAT(buf);

        ti.height        = AI_BUFFER_SHAPE_ELEM(buf, AI_SHAPE_HEIGHT);
        ti.width         = AI_BUFFER_SHAPE_ELEM(buf, AI_SHAPE_WIDTH);
        ti.ch            = AI_BUFFER_SHAPE_ELEM(buf, AI_SHAPE_CHANNEL);
        ti.size          = AI_BUFFER_SIZE(buf);
        ti.size_in_bytes = AI_BUFFER_BYTE_SIZE(ti.size, fmt);
        ti.format        = map_format(fmt);
        ti.is_signed     = AI_BUFFER_FMT_GET_SIGN(fmt) != 0;
        ti.bits          = AI_BUFFER_FMT_GET_BITS(fmt);

        // meta_info só vem populado através do report
        if (buf->meta_info != nullptr) {
            ti.q_info.scale      = AI_BUFFER_META_INFO_INTQ_GET_SCALE(buf->meta_info, 0);
            ti.q_info.zero_point = AI_BUFFER_META_INFO_INTQ_GET_ZEROPOINT(buf->meta_info, 0);
            ti.q_info.has_quant  = (ti.q_info.scale != 0.0f);
        }
        return ti;
    }
};

#endif /* MODEL_H */