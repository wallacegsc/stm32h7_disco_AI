/**
  ******************************************************************************
  * @file    hellof32_data_params.h
  * @author  AST Embedded Analytics Research Platform
  * @date    2026-07-24T14:33:42-0400
  * @brief   AI Tool Automatic Code Generator for Embedded NN computing
  ******************************************************************************
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  ******************************************************************************
  */

#ifndef HELLOF32_DATA_PARAMS_H
#define HELLOF32_DATA_PARAMS_H

#include "ai_platform.h"

/*
#define AI_HELLOF32_DATA_WEIGHTS_PARAMS \
  (AI_HANDLE_PTR(&ai_hellof32_data_weights_params[1]))
*/

#define AI_HELLOF32_DATA_CONFIG               (NULL)


#define AI_HELLOF32_DATA_ACTIVATIONS_SIZES \
  { 128, }
#define AI_HELLOF32_DATA_ACTIVATIONS_SIZE     (128)
#define AI_HELLOF32_DATA_ACTIVATIONS_COUNT    (1)
#define AI_HELLOF32_DATA_ACTIVATION_1_SIZE    (128)



#define AI_HELLOF32_DATA_WEIGHTS_SIZES \
  { 1284, }
#define AI_HELLOF32_DATA_WEIGHTS_SIZE         (1284)
#define AI_HELLOF32_DATA_WEIGHTS_COUNT        (1)
#define AI_HELLOF32_DATA_WEIGHT_1_SIZE        (1284)



#define AI_HELLOF32_DATA_ACTIVATIONS_TABLE_GET() \
  (&g_hellof32_activations_table[1])

extern ai_handle g_hellof32_activations_table[1 + 2];



#define AI_HELLOF32_DATA_WEIGHTS_TABLE_GET() \
  (&g_hellof32_weights_table[1])

extern ai_handle g_hellof32_weights_table[1 + 2];


#endif    /* HELLOF32_DATA_PARAMS_H */
