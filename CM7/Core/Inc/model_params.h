#ifndef MODEL_PARAMS_H
#define MODEL_PARAMS_H

extern "C" {
#include "ai_platform.h"
#include "hellof32_data_params.h"
#include "hellof32.h"
#include "helloi8_data_params.h"
#include "helloi8.h"
}

struct Hellof32 {

    static constexpr bool INPUTS_IN_ACTIVATION = true;
    static constexpr bool OUTPUTS_IN_ACTIVATION = true;

    // Activation config
    static constexpr int num_activations  = AI_HELLOF32_DATA_ACTIVATIONS_COUNT;
    static constexpr int size_activation1 = AI_HELLOF32_DATA_ACTIVATION_1_SIZE;

    // Input tensors
    static constexpr int num_tensors_in  = AI_HELLOF32_IN_NUM;
    static constexpr int size_tensor_in1 = AI_HELLOF32_IN_1_SIZE_BYTES;

    // Output tensors
    static constexpr int num_tensors_out  = AI_HELLOF32_OUT_NUM;
    static constexpr int size_tensor_out1 = AI_HELLOF32_OUT_1_SIZE_BYTES;

    // API wrappers
    static ai_error create_and_init(ai_handle* net,
                                    const ai_handle activations[],
                                    const ai_handle weights[]) {
        return ai_hellof32_create_and_init(net, activations, weights);
    }

    static ai_handle destroy(ai_handle net) {
        return ai_hellof32_destroy(net);
    }

    static ai_buffer* inputs_get(ai_handle net, ai_u16* n_buffer) {
        return ai_hellof32_inputs_get(net, n_buffer);
    }

    static ai_buffer* outputs_get(ai_handle net, ai_u16* n_buffer) {
        return ai_hellof32_outputs_get(net, n_buffer);
    }

    static ai_i32 run(ai_handle net, const ai_buffer* in, ai_buffer* out) {
        return ai_hellof32_run(net, in, out);
    }

    static ai_bool get_report(ai_handle net, ai_network_report* report) {
        return ai_hellof32_get_report(net, report);
    }

    static ai_error get_error(ai_handle net) {
        return ai_hellof32_get_error(net);
    }
};

struct Helloi8 {

    static constexpr bool INPUTS_IN_ACTIVATION = true;
    static constexpr bool OUTPUTS_IN_ACTIVATION = true;

    // Activation config
    static constexpr int num_activations  = AI_HELLOI8_DATA_ACTIVATIONS_COUNT;
    static constexpr int size_activation1 = AI_HELLOI8_DATA_ACTIVATION_1_SIZE;

    // Input tensors
    static constexpr int num_tensors_in  = AI_HELLOI8_IN_NUM;
    static constexpr int size_tensor_in1 = AI_HELLOI8_IN_1_SIZE_BYTES;

    // Output tensors
    static constexpr int num_tensors_out  = AI_HELLOI8_OUT_NUM;
    static constexpr int size_tensor_out1 = AI_HELLOI8_OUT_1_SIZE_BYTES;

    // API wrappers
    static ai_error create_and_init(ai_handle* net,
                                    const ai_handle activations[],
                                    const ai_handle weights[]) {
        return ai_helloi8_create_and_init(net, activations, weights);
    }

    static ai_handle destroy(ai_handle net) {
        return ai_helloi8_destroy(net);
    }

    static ai_buffer* inputs_get(ai_handle net, ai_u16* n_buffer) {
        return ai_helloi8_inputs_get(net, n_buffer);
    }

    static ai_buffer* outputs_get(ai_handle net, ai_u16* n_buffer) {
        return ai_helloi8_outputs_get(net, n_buffer);
    }

    static ai_i32 run(ai_handle net, const ai_buffer* in, ai_buffer* out) {
        return ai_helloi8_run(net, in, out);
    }

    static ai_bool get_report(ai_handle net, ai_network_report* report) {
        return ai_helloi8_get_report(net, report);
    }

    static ai_error get_error(ai_handle net) {
        return ai_helloi8_get_error(net);
    }
};

#endif /* MODEL_PARAMS_H */