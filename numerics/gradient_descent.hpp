#pragma once

#include <functional>
#include <optional>

#include "geometry/hilbert.hpp"
#include "quantities/named_quantities.hpp"
#include "quantities/quantities.hpp"

namespace principia {
namespace numerics {
namespace _gradient_descent {
namespace internal {

using namespace principia::geometry::_hilbert;
using namespace principia::quantities::_named_quantities;
using namespace principia::quantities::_quantities;

// In this file |Argument| must be such that its difference belongs to a Hilbert
// space.

// A helper for generating declarations derived from |Scalar| and |Argument|.
// It must declare the type of the gradient and a function returning the inner
// product form thus:
//   using Gradient = ...;
//   static ... InnerProductForm();
template<typename Scalar, typename Argument>
struct Generator;

template<typename Value, typename Argument>
using Field = std::function<Value(Argument const&)>;

template<typename Scalar, typename Argument>
using Gradient = typename Generator<Scalar, Argument>::Gradient;

// Must return InnerProduct(grad_f(argument), direction);
template<typename Scalar, typename Argument>
using GateauxDerivative =
    std::function<Scalar(Argument const& argument,
                         Difference<Argument> const& direction)>;

// Stops when the search displacement is smaller than |tolerance|.  Returns
// |nullopt| if no minimum is found within distance |radius| of
// |start_argument|.
template<typename Scalar, typename Argument>
std::optional<Argument> BroydenFletcherGoldfarbShanno(
    Argument const& start_argument,
    Field<Scalar, Argument> const& f,
    Field<Gradient<Scalar, Argument>, Argument> const& grad_f,
    typename Hilbert<Difference<Argument>>::NormType const& tolerance,
    typename Hilbert<Difference<Argument>>::NormType const& radius =
        Infinity<typename Hilbert<Difference<Argument>>::NormType>);

// Same as above, but the Gateaux derivative of f is passed in addition to its
// gradient.  Useful when the Gateaux derivative is significantly less expensive
// to compute than the full gradient.
template<typename Scalar, typename Argument>
std::optional<Argument> BroydenFletcherGoldfarbShanno(
    Argument const& start_argument,
    Field<Scalar, Argument> const& f,
    Field<Gradient<Scalar, Argument>, Argument> const& grad_f,
    GateauxDerivative<Scalar, Argument> const& gateaux_derivative_f,
    typename Hilbert<Difference<Argument>>::NormType const& tolerance,
    typename Hilbert<Difference<Argument>>::NormType const& radius =
        Infinity<typename Hilbert<Difference<Argument>>::NormType>);

}  // namespace internal

using internal::BroydenFletcherGoldfarbShanno;
using internal::Gradient;
using internal::Field;

}  // namespace _gradient_descent
}  // namespace numerics
}  // namespace principia

#include "numerics/gradient_descent_body.hpp"
