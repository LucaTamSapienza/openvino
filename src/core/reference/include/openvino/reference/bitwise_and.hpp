// Copyright (C) 2018-2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include <algorithm>
#include <cstddef>

#include "openvino/reference/autobroadcast_binop.hpp"

namespace ov {
namespace reference {
/**
 * @brief Reference implementation of binary elementwise bitwise AND operator.
 *
 * @param arg0            Pointer to input 0 data.
 * @param arg1            Pointer to input 1 data.
 * @param out             Pointer to output data.
 * @param arg_shape0      Input 0 shape.
 * @param arg_shape1      Input 1 shape.
 * @param broadcast_spec  Broadcast specification mode.
 */
template <class T, typename std::enable_if<std::is_same<typename std::decay<T>::type, char>::value>::type* = nullptr>
// Check for char datatype used by ov::element::boolean
void bitwise_and(const T* arg0,
                 const T* arg1,
                 T* out,
                 const Shape& arg0_shape,
                 const Shape& arg1_shape,
                 const op::AutoBroadcastSpec& broadcast_spec) {
    autobroadcast_binop(arg0, arg1, out, arg0_shape, arg1_shape, broadcast_spec, std::bit_and<bool>());
}
/**
 * @brief Reference implementation of binary elementwise bitwise AND operator.
 *
 * @param arg0            Pointer to input 0 data.
 * @param arg1            Pointer to input 1 data.
 * @param out             Pointer to output data.
 * @param arg_shape0      Input 0 shape.
 * @param arg_shape1      Input 1 shape.
 * @param broadcast_spec  Broadcast specification mode.
 */
template <class T, typename std::enable_if<!std::is_same<typename std::decay<T>::type, char>::value>::type* = nullptr>
void bitwise_and(const T* arg0,
                 const T* arg1,
                 T* out,
                 const Shape& arg0_shape,
                 const Shape& arg1_shape,
                 const op::AutoBroadcastSpec& broadcast_spec) {
    autobroadcast_binop(arg0, arg1, out, arg0_shape, arg1_shape, broadcast_spec, std::bit_and<T>());
}
}  // namespace reference
}  // namespace ov
