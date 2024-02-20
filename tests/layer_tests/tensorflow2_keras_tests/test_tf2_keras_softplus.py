# Copyright (C) 2022 Intel Corporation
# SPDX-License-Identifier: Apache-2.0

import pytest
import tensorflow as tf
from common.layer_test_class import check_ir_version
from common.tf2_layer_test_class import CommonTF2LayerTest
from unit_tests.utils.graph import build_graph


class TestKerasSoftplus(CommonTF2LayerTest):
    def create_keras_softplus_net(self, input_names, input_shapes, input_type, ir_version):
        """
               Tensorflow2 Keras net:                     IR net:
                      Input               =>               Input
                        |                                    |
                     Softplus                             Softplus
        """
        # create TensorFlow 2 model with Keras Softplus operation
        tf.keras.backend.clear_session()  # For easy reset of notebook state
        x1 = tf.keras.Input(shape=input_shapes[0][1:],
                            name=input_names[0])  # Variable-length sequence of ints
        y = tf.keras.activations.softplus()(x1)
        tf2_net = tf.keras.Model(inputs=[x1], outputs=[y])

        # create reference IR net
        ref_net = None

        if check_ir_version(10, None, ir_version):
            # convert NHWC to NCHW layout if tensor rank greater 3
            converted_input_shape = input_shapes[0].copy()
            if len(converted_input_shape) > 3:
                converted_input_shape[1] = input_shapes[0][-1]
                converted_input_shape[2:] = input_shapes[0][1:-1]
            nodes_attributes = {
                'input1': {'kind': 'op', 'type': 'Parameter'},
                'input1_data': {'shape': converted_input_shape, 'kind': 'data'},
                'softplus': {'kind': 'op', 'type': 'SoftPlus'},
                'softplus_data': {'shape': converted_input_shape, 'kind': 'data'},
                'result': {'kind': 'op', 'type': 'Result'}
            }

            ref_net = build_graph(nodes_attributes,
                                  [('input1', 'input1_data'),
                                   ('input1_data', 'softplus', {'in': 0}),
                                   ('softplus', 'softplus_data'),
                                   ('softplus_data', 'result')])

        return tf2_net, ref_net

    test_data_float32_precommit = [
        dict(input_names=["x1"], input_shapes=[[5, 4, 8, 3, 2]], input_type=tf.float32)]

    @pytest.mark.parametrize("params", test_data_float32_precommit)
    @pytest.mark.precommit
    @pytest.mark.xfail(reason="49516")
    def test_keras_softplus_float32(self, params, ie_device, precision, ir_version, temp_dir,
                                    use_legacy_frontend):
        self._test(*self.create_keras_softplus_net(**params, ir_version=ir_version),
                   ie_device, precision, temp_dir=temp_dir, ir_version=ir_version,
                   use_legacy_frontend=use_legacy_frontend, **params)

    test_data_float32 = [dict(input_names=["x1"], input_shapes=[[5]], input_type=tf.float32),
                         dict(input_names=["x1"], input_shapes=[[5, 4]], input_type=tf.float32),
                         dict(input_names=["x1"], input_shapes=[[5, 4, 8]], input_type=tf.float32),
                         dict(input_names=["x1"], input_shapes=[[5, 4, 8, 3]],
                              input_type=tf.float32),
                         dict(input_names=["x1"], input_shapes=[[5, 4, 8, 3, 2]],
                              input_type=tf.float32)]

    @pytest.mark.parametrize("params", test_data_float32)
    @pytest.mark.nightly
    @pytest.mark.precommit
    @pytest.mark.xfail(reason="49516")
    def test_keras_softplus_float32(self, params, ie_device, precision, ir_version, temp_dir,
                                    use_legacy_frontend):
        self._test(*self.create_keras_softplus_net(**params, ir_version=ir_version),
                   ie_device, precision, temp_dir=temp_dir, ir_version=ir_version,
                   use_legacy_frontend=use_legacy_frontend, **params)
