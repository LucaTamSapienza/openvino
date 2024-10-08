// Copyright (C) 2018-2024 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include "pass_manager.h"
#include "fully_connected_inst.h"
#include "gemm_inst.h"
#include "program_node.h"
#include "intel_gpu/runtime/engine.hpp"
#include "intel_gpu/runtime/itt.hpp"
#include <iostream>

#ifdef ENABLE_ONEDNN_FOR_GPU
#include <oneapi/dnnl/dnnl.hpp>
#include "intel_gpu/runtime/debug_configuration.hpp"
#include "impls/onednn/utils.hpp"
#include "impls/onednn/convolution_onednn.hpp"
#include "impls/onednn/deconvolution_onednn.hpp"
#endif

using namespace cldnn;

void select_preferred_formats::run(program& p) {
    OV_ITT_SCOPED_TASK(ov::intel_gpu::itt::domains::intel_gpu_plugin, "pass::select_preferred_formats");

    auto& engine = p.get_engine();
    const auto& device_info = engine.get_device_info();

    if (!device_info.supports_immad)
        return;

#ifdef ENABLE_ONEDNN_FOR_GPU
    auto& lo = p.get_layout_optimizer();

    auto forcing_map = lo.get_implementation_forcing();

    engine.create_onednn_engine(p.get_config());
    for (auto n : p.get_processing_order()) {
        if (n->is_input() || !layout_optimizer::is_node_suitable_for_onednn(*n)) {
            continue;
        }

        // skip to set preferred_formats if forcing_impl is not onednn.
        if (std::find_if(forcing_map.begin(), forcing_map.end(),
                [&n](std::map<primitive_id, std::pair<format::type, impl_types>>::value_type const& it) {
                    return (it.first == n->id() && it.second.second != impl_types::onednn);
                }) != forcing_map.end())
            continue;

        // Onednn primitive descriptor creation may fail, for example, due to asymmetric weight.
        try {
            if (n->is_type<convolution>()) {
                if (n->as<convolution>().weights_zero_points_term()) {
                    if (n->as<convolution>().weights_zero_points().get_output_layout().count() != 1 ||
                        n->as<convolution>().get_groups() > 1) {
                        // onednn convolution doesn't support per_oc and grouped as weights zero points.
                        continue;
                    }
                }

                auto prim_desc = onednn::get_convolution_primitive_descriptor(*n->get_kernel_impl_params(),
                                                                              dnnl::primitive_attr(),
                                                                              dnnl::memory::format_tag::any);
                lo.select_preferred_formats_for_onednn(*n, *prim_desc);
            } else if (n->is_type<deconvolution>()) {
                auto prim_desc = onednn::get_deconvolution_primitive_descriptor(*n->get_kernel_impl_params(),
                                                                                dnnl::primitive_attr(),
                                                                                dnnl::memory::format_tag::any);
                lo.select_preferred_formats_for_onednn(*n, *prim_desc);
            } else if (n->is_type<fully_connected>() || n->is_type<gemm>()) {
                lo.select_preferred_formats_for_onednn(*n);
            }
        } catch(std::exception &exception) {
            GPU_DEBUG_INFO << "WARNING(select_preferred_formats): " << exception.what() << std::endl;
        }
    }
#endif  // ENABLE_ONEDNN_FOR_GPU
}
