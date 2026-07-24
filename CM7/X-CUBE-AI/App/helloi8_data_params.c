/**
  ******************************************************************************
  * @file    helloi8_data_params.c
  * @author  AST Embedded Analytics Research Platform
  * @date    2026-07-24T14:33:58-0400
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

#include "helloi8_data_params.h"


/**  Activations Section  ****************************************************/
ai_handle g_helloi8_activations_table[1 + 2] = {
  AI_HANDLE_PTR(AI_MAGIC_MARKER),
  AI_HANDLE_PTR(NULL),
  AI_HANDLE_PTR(AI_MAGIC_MARKER),
};




/**  Weights Section  ********************************************************/
AI_ALIGNED(32)
const ai_u64 s_helloi8_weights_array_u64[53] = {
  0x636273684739caf7U, 0x565f44ae197fe640U, 0x0U, 0xffffea75ffffeac2U,
  0xfffffa24fffffab8U, 0xffffffacffffefc8U, 0xd44U, 0xffffea33000007bdU,
  0xffffe4cc00000000U, 0xffffe3cf00000d4fU, 0x2d2dc0309ed1af4U, 0xda14fe10ebdd27fbU,
  0x24220a230b0715e4U, 0x2f4f90624f42119U, 0xf1f3f81c1a05e4dbU, 0xe603271efe1a1b04U,
  0xf1effaffbcef21e0U, 0x9f3e2ef1224fc17U, 0xc0ff240321de01ebU, 0xe127d3ef19faf5f7U,
  0xe3ede9e3e41eef15U, 0xf1fcf81103ec0be6U, 0xfdfbdde0f50c11U, 0x3e71cef0a1d2125U,
  0xd7e91fe2d8fc1aeaU, 0x17112614e51707f3U, 0x30216e020dc1ddbU, 0xc6dde2cad8c40013U,
  0xea11f1e7dcfaede8U, 0x2e09182137e3fa01U, 0x3e04e7130dfed9ecU, 0x404260207fb23e8U,
  0xb14e5d9161af10bU, 0xc7140100d6d82019U, 0x1ad606f51515e7dbU, 0xef130208fe2209dcU,
  0xdd14f3fd09e21e19U, 0xeef7f9e30fd920daU, 0xe216070029e624e9U, 0xfa14f7ddd3230d1eU,
  0xb0f080921262208U, 0x26e558dc7ff412e0U, 0x7a2fffffd27U, 0x262U,
  0xfffffe29000000f1U, 0xfffffc9dffffffddU, 0x2450000023bU, 0xf67000010a4U,
  0x24fU, 0xffffec11fffffc87U, 0xdddee01c15273bd9U, 0x7ff9dd12d7c51b0fU,
  0x1adU,
};


ai_handle g_helloi8_weights_table[1 + 2] = {
  AI_HANDLE_PTR(AI_MAGIC_MARKER),
  AI_HANDLE_PTR(s_helloi8_weights_array_u64),
  AI_HANDLE_PTR(AI_MAGIC_MARKER),
};

