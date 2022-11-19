#pragma once

#include <functional>
#include <random>
#include <vector>

#include "geometry/hilbert.hpp"
#include "numerics/nearest_neighbour.hpp"
#include "quantities/named_quantities.hpp"
#include "quantities/quantities.hpp"

namespace principia {
namespace numerics {
namespace internal_global_optimization {

using geometry::Hilbert;
using quantities::Cube;
using quantities::Difference;
using quantities::Length;
using quantities::Product;
using quantities::Quotient;

// In this file |Argument| must be such that its difference belongs to a Hilbert
// space.

template<typename Scalar, typename Argument>
using Field = std::function<Scalar(Argument const&)>;

template<typename Scalar, typename Argument>
using Gradient =
    Product<Scalar,
            Quotient<Difference<Argument>,
                     typename Hilbert<Difference<Argument>>::Norm²Type>>;

// NOTE(phl): This could nearly be a self-standing function (it doesn't have
// much state) but having the type |Box| floating around would be unpleasant.
// Plus, that would be too many parameters in that function.
template<typename Scalar, typename Argument>
class MultiLevelSingleLinkage {
 public:
  using NormType = typename Hilbert<Difference<Argument>>::NormType;

  // A parallelepiped defined by its centre and the displacements of three
  // vertices.  Random points are uniformly distributed in the box.
  struct Box {
    Argument centre;
    std::array<Difference<Argument>, 3> vertices;

    Cube<NormType> Measure() const;
  };

  MultiLevelSingleLinkage(
      Box const& box,
      Field<Scalar, Argument> const& f,
      Field<Gradient<Scalar, Argument>, Argument> const& grad_f);

  std::vector<Argument> FindGlobalMinima(std::int64_t points_per_round,
                                         std::int64_t number_of_rounds,
                                         NormType local_search_tolerance);

 private:
  // Returns true iff the given |stationary_point| is sufficiently far from the
  // ones already in |stationary_point_neighbourhoods|.
  static bool IsNewStationaryPoint(
      Argument const& stationary_point,
      PrincipalComponentPartitioningTree<Argument> const&
          stationary_point_neighbourhoods,
      NormType local_search_tolerance);

  // Returns a vector of size |values_per_round|.  The points are in |box_|.
  std::vector<Argument> RandomArguments(std::int64_t values_per_round);

  // Returns the radius rₖ from [RT87a], eqn. 35, specialized for 3 dimensions.
  NormType CriticalRadius(double σ, std::int64_t kN);

  Box const box_;
  Cube<NormType> const box_measure_;
  Field<Scalar, Argument> const f_;
  Field<Gradient<Scalar, Argument>, Argument> const grad_f_;

  std::mt19937_64 random_;
  std::uniform_real_distribution<> distribution_;
};

}  // namespace internal_global_optimization

using internal_global_optimization::MultiLevelSingleLinkage;

}  // namespace numerics
}  // namespace principia

#include "numerics/global_optimization_body.hpp"
