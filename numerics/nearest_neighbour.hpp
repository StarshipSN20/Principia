#pragma once

#include <utility>
#include <variant>
#include <vector>

#include "base/not_null.hpp"
#include "geometry/frame.hpp"
#include "geometry/grassmann.hpp"
#include "geometry/hilbert.hpp"
#include "geometry/symmetric_bilinear_form.hpp"
#include "quantities/named_quantities.hpp"

namespace principia {
namespace numerics {
namespace internal_nearest_neighbour {

using base::not_null;
using geometry::Bivector;
using geometry::Frame;
using geometry::Hilbert;
using geometry::SymmetricProduct;
using quantities::Difference;

template<typename Value_>
class PrincipalComponentPartitioningTree {
 public:
  using Value = Value_;

  // We stop subdividing a cell when it contains |max_values_per_cell| or less
  // values.
  PrincipalComponentPartitioningTree(std::vector<Value> const& values,
                                     std::int64_t max_values_per_cell);

  void Add(Value const& value);

  Value const& FindNearest(Value const& value) const;

 private:
  // A frame used to compute the principal components.
  using PrincipalComponentsFrame =
      Frame<enum class PrincipalComponentsFrameTag>;

  // A displacement from the centroid.
  using Displacement = Difference<Value>;

  // The type of the norm (and the elements) of |Displacement|.
  using Norm = typename Hilbert<Displacement>::NormType;

  // A unit vector corresponding to |Displacement|.
  using Axis = typename Hilbert<Displacement>::NormalizedType;

  // A form that operates on |Displacement|s.
  // NOTE(phl): We don't have SymmetricProduct for Bivector, so this effectively
  // means that Displacement is a Vector.
  using DisplacementSymmetricBilinearForm =
      decltype(SymmetricProduct(std::declval<Displacement>(),
                                std::declval<Displacement>()));

  // The principal components (a.k.a. eigensystem) of the above form.
  using DisplacementPrincipalComponentsSystem =
      typename DisplacementSymmetricBilinearForm::template Eigensystem<
          PrincipalComponentsFrame>;

  // A unit vector in the principal components frame.
  using PrincipalComponentsAxis = decltype(
      std::declval<DisplacementPrincipalComponentsSystem>().rotation.Inverse()(
          std::declval<Axis>()));

  // The declarations of the tree structure.
  struct Internal;

  using Leaf = std::vector<std::int32_t>;  // Indices in |displacements_|.

  using Node = std::variant<Internal, Leaf>;

  using Children = std::pair<not_null<std::unique_ptr<Node>>,
                             not_null<std::unique_ptr<Node>>>;

  struct Internal {
    Axis principal_axis;
    Displacement anchor;
    Children children;
  };

  // The construction of the tree uses this type, which contains an index in the
  // |displacements_| array and storage for the projection of the corresponding
  // |Displacement| on the current principal axis.  We use 32-bit integers
  // because these objects will be swapped a lot, so the less memory we touch
  // the better.
  struct Index {
    std::int32_t index;
    Norm projection;
  };
  using Indices = std::vector<Index>;

  // Constructs a tree for the displacements given by the index range
  // [begin, end[.  |size| must be equal to |std::distance(begin, end)|, but is
  // passed by the caller for efficiency.
  not_null<std::unique_ptr<Node>> BuildTree(Indices::iterator begin,
                                            Indices::iterator end,
                                            std::int64_t size) const;

  // Returns the symmetric bilinear form that represents the "inertia" of the
  // displacements given by the index range [begin, end[.
  DisplacementSymmetricBilinearForm ComputePrincipalComponentForm(
      Indices::iterator begin,
      Indices::iterator end) const;

  std::int64_t const max_values_per_cell_;

  // The centroid of the values passed at construction.
  Value centroid_;

  // The displacements from the centroid.
  std::vector<Displacement> displacements_;

  std::unique_ptr<Node> root_;
};

}  // namespace internal_nearest_neighbour

using internal_nearest_neighbour::PrincipalComponentPartitioningTree;

}  // namespace numerics
}  // namespace principia

#include "numerics/nearest_neighbour_body.hpp"
