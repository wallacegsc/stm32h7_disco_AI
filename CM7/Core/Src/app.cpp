// app.cpp
#include "main.h"
#include <cstdio>
#include "sys.h"
#include "usart.h"

#include "model.h"
#include "model_params.h"


extern "C" void app_main(void)
{
    sys::UartSerial uart{huart1};
    sys::Logger log{uart};

    static Model<Hellof32> model;
    static Model<Helloi8>   modelQ;

    if (model.init() != 0) {
        auto e = model.get_error();
        log.log_error("Model init falhou: type=0x%02x code=0x%02x", e.type, e.code);
        while (true) { sys::delay(1000); }
    }

    if (modelQ.init() != 0) {
        auto e = modelQ.get_error();
        log.log_error("ModelQ init falhou: type=0x%02x code=0x%02x", e.type, e.code);
        while (true) { sys::delay(1000); }
    }

    // Confere se a quantizacao foi descoberta no init
    const auto& qin  = modelQ.get_input_tensor_info();
    const auto& qout = modelQ.get_output_tensor_info();
    log.log_info("Q in : bits=%lu scale=%.6f zp=%ld quant=%d",
                 qin.bits, qin.q_info.scale, qin.q_info.zero_point, qin.q_info.has_quant);
    log.log_info("Q out: bits=%lu scale=%.6f zp=%ld quant=%d",
                 qout.bits, qout.q_info.scale, qout.q_info.zero_point, qout.q_info.has_quant);

    float* input_tensor  = (float*) model.get_input_tensor();
    float* output_tensor = (float*) model.get_output_tensor();


    while (true) {
        for(float x_model = 0; x_model < 3.0f ;x_model += 0.10f)
        {

        // modelo float
        input_tensor[0] = x_model;
        float y_float = 0.0f;
        bool ok_f = (model.run() == 0);
        if (ok_f) y_float = output_tensor[0];

        // modelo quantizado
        float y_quant = 0.0f;
        modelQ.quantize_input(&x_model, 1);
        bool ok_q = (modelQ.run() == 0);
        if (ok_q) modelQ.dequantize_output(&y_quant, 1);

        // float y_real = std::sin(x_model);
        float y_real = 0;

        if (ok_f && ok_q) {
            log.log_info("x=%.3f  real=%+.4f  f32=%+.4f (%+.4f)  int8=%+.4f (%+.4f)",
                         x_model, y_real,
                         y_float, y_float - y_real,
                         y_quant, y_quant - y_real);
        } else {
            log.log_error("Inference fail: f32=%d int8=%d", ok_f, ok_q);
        }

        sys::delay(200);
        }
    }
}
