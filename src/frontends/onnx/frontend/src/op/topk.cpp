// Copyright (C) 2018-2023 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include "op/topk.hpp"

#include "openvino/frontend/exception.hpp"
#include "openvino/op/topk.hpp"
#include "utils/reshape.hpp"

namespace {
/// \return Return the second input to the TopK node reshaped to a scalar.
ov::Output<ov::Node> get_k(const ov::frontend::onnx::Node& node) {
    auto k_node = node.get_ng_inputs().at(1);
    FRONT_END_GENERAL_CHECK(shape_size(k_node.get_shape()) == 1,
                            "ONNX TopK operator: 'K' parameter must contain a single positive value.",
                            node);

    return ov::frontend::onnx::reshape::interpret_as_scalar(k_node);
}
}  // namespace

using namespace ov::op;

using namespace ov::op;

namespace ov {
namespace frontend {
namespace onnx {
namespace op {
namespace set_1 {
ov::OutputVector topk(const ov::frontend::onnx::Node& node) {
    auto data = node.get_ng_inputs().at(0);
    const auto k_node = node.get_attribute_as_constant<std::int64_t>("k");
    const std::int64_t axis{node.get_attribute_value<std::int64_t>("axis", -1)};

    std::shared_ptr<ov::Node> top_k = std::make_shared<v11::TopK>(data,
                                                                  k_node,
                                                                  axis,
                                                                  v11::TopK::Mode::MAX,
                                                                  v11::TopK::SortType::SORT_VALUES,
                                                                  ov::element::i64);

    return {top_k->output(0), top_k->output(1)};
}
}  // namespace set_1

namespace set_10 {
ov::OutputVector topk(const ov::frontend::onnx::Node& node) {
    auto data = node.get_ng_inputs().at(0);
    auto k = get_k(node);
    const std::int64_t axis{node.get_attribute_value<std::int64_t>("axis", -1)};

    std::shared_ptr<ov::Node> top_k = std::make_shared<v11::TopK>(data,
                                                                  k,
                                                                  axis,
                                                                  v11::TopK::Mode::MAX,
                                                                  v11::TopK::SortType::SORT_VALUES,
                                                                  ov::element::i64);

    return {top_k->output(0), top_k->output(1)};
}
}  // namespace set_10

namespace set_11 {
ov::OutputVector topk(const ov::frontend::onnx::Node& node) {
    // Process inputs
    auto data = node.get_ng_inputs().at(0);
    auto k = get_k(node);

    // Process attributes
    const std::int64_t axis{node.get_attribute_value<std::int64_t>("axis", -1)};
    const auto largest = node.get_attribute_value<std::int64_t>("largest", 1);
    const auto sorted = node.get_attribute_value<std::int64_t>("sorted", 1);

    // Map attribute values to OpenVINO enums
    const auto sort_type = sorted ? v11::TopK::SortType::SORT_VALUES : v11::TopK::SortType::NONE;

    const auto compute_max = static_cast<bool>(largest);
    const auto mode = compute_max ? v11::TopK::Mode::MAX : v11::TopK::Mode::MIN;

    std::shared_ptr<ov::Node> top_k = std::make_shared<v11::TopK>(data, k, axis, mode, sort_type, ov::element::i64);

    return {top_k->output(0), top_k->output(1)};
}
}  // namespace set_11
}  // namespace op
}  // namespace onnx
}  // namespace frontend
}  // namespace ov
