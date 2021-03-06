#include <c10/core/dispatch/Dispatcher.h>
#include <c10/core/opschema/layer_norm.h>
#include <torch/csrc/jit/custom_operator.h>
#include <torch/csrc/autograd/variable.h>

using c10::C10Tensor;

namespace {
// TODO Return tuple<Tensor, Tensor, Tensor> instead of vector<Tensor>
std::vector<at::Tensor> layer_norm(
    at::Tensor input,
    int64_t axis,
    double epsilon) {

  // TODO This code is currently written specifically for LayerNorm, but it is
  //      *not* the plan to have to write this manually for each operation.
  //      This is just a proof of concept. To expand this to all operators,
  //      we'd ideally not need any per-operator code (possibly thanks to boxing
  //      or templates). If that's not possible, then we should at least offer
  //      a macro that takes this burden so that we only need to write one line
  //      for each operation we want to support (i.e. the macro invocation).

  // TODO This currently only handles tensors with requires_grad==False correctly.
  //      It should also handle autograd.

  if (input.requires_grad()) {
    throw std::runtime_error("Autograd not yet supported for c10 ops.");
  }
  c10::core::opschema::LayerNorm::Cache cache;
  C10Tensor c10_input(torch::autograd::Variable(std::move(input)).data());
  C10Tensor c10_output(at::empty({0}));
  C10Tensor c10_output_mean(at::empty({0}));
  C10Tensor c10_output_stdev(at::empty({0}));
  c10::Dispatcher<c10::core::opschema::LayerNorm>::call(c10_input, c10_output, c10_output_mean, c10_output_stdev, (int)axis, (float)epsilon, &cache);
  return {
    torch::autograd::make_variable(at::Tensor(std::move(c10_output)), false),
    torch::autograd::make_variable(at::Tensor(std::move(c10_output_mean)), false),
    torch::autograd::make_variable(at::Tensor(std::move(c10_output_stdev)), false)
  };
}
}

static auto registry =
  torch::jit::RegisterOperators("caffe2::layer_norm_dont_use_this_op_yet", &layer_norm);
