// Copyright (C) 2018-2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include "op/scatter_elements.hpp"

#include "exceptions.hpp"
#include "openvino/op/scatter_elements_update.hpp"

using namespace ov::op;

namespace ov {
namespace frontend {
namespace onnx {
namespace op {
namespace set_1 {
ov::OutputVector scatter_elements(const ov::frontend::onnx::Node& node) {
    const auto data = node.get_ng_inputs().at(0);
    const auto indices = node.get_ng_inputs().at(1);
    const auto updates = node.get_ng_inputs().at(2);
    const auto axis_node = node.get_attribute_as_constant<std::int64_t>("axis", 0);

    v12::ScatterElementsUpdate::Reduction reduction_ov;
    if (node.has_attribute("reduction")) {
        std::string reduction_onnx = node.get_attribute_value<std::string>("reduction", "none");
        if (reduction_onnx == "none") {
            reduction_ov = v12::ScatterElementsUpdate::Reduction::NONE;
        } else if (reduction_onnx == "add") {
            reduction_ov = v12::ScatterElementsUpdate::Reduction::SUM;
        } else if (reduction_onnx == "mul") {
            reduction_ov = v12::ScatterElementsUpdate::Reduction::PROD;
        } else if (reduction_onnx == "min") {
            reduction_ov = v12::ScatterElementsUpdate::Reduction::MIN;
        } else if (reduction_onnx == "max") {
            reduction_ov = v12::ScatterElementsUpdate::Reduction::MAX;
        } else {
            CHECK_VALID_NODE(node,
                             false,
                             "Unsupported value of attribute: `reduction`. "
                             "Supported modes: `none`, `add`, `mul`, `min`, `max`, got:",
                             reduction_onnx);
        }
    } else {
        reduction_ov = v12::ScatterElementsUpdate::Reduction::NONE;
    }

    return {std::make_shared<v12::ScatterElementsUpdate>(data, indices, updates, axis_node, reduction_ov)};
}
}  // namespace set_1
}  // namespace op
}  // namespace onnx
}  // namespace frontend
}  // namespace ov
