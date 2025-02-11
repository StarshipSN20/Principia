// .\Release\x64\benchmarks.exe --benchmark_repetitions=3 --benchmark_filter=OrbitalElements --benchmark_min_time=30  // NOLINT(whitespace/line_length)

#include <limits>

#include "astronomy/epoch.hpp"
#include "astronomy/frames.hpp"
#include "astronomy/orbital_elements.hpp"
#include "base/not_null.hpp"
#include "benchmark/benchmark.h"
#include "geometry/instant.hpp"
#include "integrators/methods.hpp"
#include "integrators/symmetric_linear_multistep_integrator.hpp"
#include "physics/body_centred_non_rotating_reference_frame.hpp"
#include "physics/degrees_of_freedom.hpp"
#include "physics/discrete_trajectory.hpp"
#include "physics/discrete_trajectory_segment.hpp"
#include "physics/ephemeris.hpp"
#include "physics/kepler_orbit.hpp"
#include "physics/massive_body.hpp"
#include "physics/massless_body.hpp"
#include "physics/oblate_body.hpp"
#include "physics/solar_system.hpp"
#include "quantities/quantities.hpp"
#include "quantities/si.hpp"

namespace principia {
namespace astronomy {

using namespace principia::astronomy::_epoch;
using namespace principia::astronomy::_frames;
using namespace principia::astronomy::_orbital_elements;
using namespace principia::base::_not_null;
using namespace principia::geometry::_instant;
using namespace principia::integrators::_methods;
using namespace principia::integrators::_symmetric_linear_multistep_integrator;
using namespace principia::physics::_body_centred_non_rotating_reference_frame;
using namespace principia::physics::_degrees_of_freedom;
using namespace principia::physics::_discrete_trajectory;
using namespace principia::physics::_discrete_trajectory_segment;
using namespace principia::physics::_ephemeris;
using namespace principia::physics::_kepler_orbit;
using namespace principia::physics::_massive_body;
using namespace principia::physics::_massless_body;
using namespace principia::physics::_oblate_body;
using namespace principia::physics::_solar_system;
using namespace principia::quantities::_quantities;
using namespace principia::quantities::_si;

class OrbitalElementsBenchmark : public benchmark::Fixture {
 protected:
  static void SetUpFixture() {
    solar_system_ = std::make_unique<SolarSystem<ICRS>>(
        SOLUTION_DIR / "astronomy" / "sol_gravity_model.proto.txt",
        SOLUTION_DIR / "astronomy" /
            "sol_initial_state_jd_2451545_000000000.proto.txt").release();
    ephemeris_ =
        solar_system_
            ->MakeEphemeris(
                /*accuracy_parameters=*/{/*fitting_tolerance=*/1 * Milli(Metre),
                                         /*geopotential_tolerance=*/0x1p-24},
                Ephemeris<ICRS>::FixedStepParameters(
                    SymmetricLinearMultistepIntegrator<
                        QuinlanTremaine1990Order12,
                        Ephemeris<ICRS>::NewtonianMotionEquation>(),
                    /*step=*/10 * Minute))
            .release();
    earth_ = dynamic_cast_not_null<OblateBody<ICRS> const*>(
        solar_system_->massive_body(*ephemeris_, "Earth"));
  }

  void SetUp(benchmark::State&) override {
    static int const set_up_fixture = []() {
      SetUpFixture();
      return 0;
    }();
  }

  static not_null<std::unique_ptr<DiscreteTrajectory<GCRS>>>
  EarthCentredTrajectory(
      KeplerianElements<GCRS> const& initial_osculating_elements,
      Instant const& initial_time,
      Instant const& final_time) {
    BodyCentredNonRotatingReferenceFrame<ICRS, GCRS> gcrs{ephemeris_, earth_};
    DiscreteTrajectory<ICRS> icrs_trajectory;
    icrs_trajectory.segments().front().SetDownsampling(
        DiscreteTrajectorySegment<ICRS>::DownsamplingParameters{
            .max_dense_intervals = 10'000,
            .tolerance = 1 * Milli(Metre),
        });
    KeplerOrbit<GCRS> const initial_osculating_orbit{
        *earth_,
        MasslessBody{},
        initial_osculating_elements,
        initial_time};
    CHECK_OK(icrs_trajectory.Append(
        initial_time,
        gcrs.FromThisFrameAtTime(initial_time)(
            DegreesOfFreedom<GCRS>{GCRS::origin, GCRS::unmoving} +
            initial_osculating_orbit.StateVectors(initial_time))));
    auto instance = ephemeris_->NewInstance(
        {&icrs_trajectory},
        Ephemeris<ICRS>::NoIntrinsicAccelerations,
        Ephemeris<ICRS>::FixedStepParameters(
            SymmetricLinearMultistepIntegrator<
                Quinlan1999Order8A,
                Ephemeris<ICRS>::NewtonianMotionEquation>(),
            /*step=*/10 * Second));
    CHECK_OK(ephemeris_->FlowWithFixedStep(final_time, *instance));
    auto result = make_not_null_unique<DiscreteTrajectory<GCRS>>();
    for (auto const& [time, degrees_of_freedom] : icrs_trajectory) {
      CHECK_OK(result->Append(
          time, gcrs.ToThisFrameAtTime(time)(degrees_of_freedom)));
    }
    return result;
  }

  static SolarSystem<ICRS>* solar_system_;
  static Ephemeris<ICRS>* ephemeris_;
  static OblateBody<ICRS> const* earth_;
};

SolarSystem<ICRS>* OrbitalElementsBenchmark::solar_system_ = nullptr;
Ephemeris<ICRS>* OrbitalElementsBenchmark::ephemeris_ = nullptr;
OblateBody<ICRS> const* OrbitalElementsBenchmark::earth_ = nullptr;

BENCHMARK_F(OrbitalElementsBenchmark, ComputeOrbitalElementsEquatorial)(
    benchmark::State& state) {
  Time const mission_duration = 180 * Day;
  Instant const final_time = J2000 + mission_duration;
  CHECK_OK(ephemeris_->Prolong(final_time));

  KeplerianElements<GCRS> initial_osculating;
  initial_osculating.semimajor_axis = 7000 * Kilo(Metre);
  initial_osculating.eccentricity = 1e-6;
  initial_osculating.inclination = 10 * Milli(ArcSecond);
  initial_osculating.longitude_of_ascending_node = 10 * Degree;
  initial_osculating.argument_of_periapsis = 20 * Degree;
  initial_osculating.mean_anomaly = 30 * Degree;
  auto const trajectory =
      EarthCentredTrajectory(initial_osculating, J2000, final_time);
  for (auto _ : state) {
    benchmark::DoNotOptimize(
        OrbitalElements::ForTrajectory(*trajectory, *earth_, MasslessBody{}));
  }
}

BENCHMARK_F(OrbitalElementsBenchmark, ComputeOrbitalElementsInclined)(
    benchmark::State& state) {
  Time const mission_duration = 180 * Day;
  Instant const final_time = J2000 + mission_duration;
  CHECK_OK(ephemeris_->Prolong(final_time));

  KeplerianElements<GCRS> initial_osculating;
  initial_osculating.semimajor_axis = 7000 * Kilo(Metre);
  initial_osculating.eccentricity = 1e-6;
  initial_osculating.inclination = 60 * Degree;
  initial_osculating.longitude_of_ascending_node = 10 * Degree;
  initial_osculating.argument_of_periapsis = 20 * Degree;
  initial_osculating.mean_anomaly = 30 * Degree;
  auto const trajectory =
      EarthCentredTrajectory(initial_osculating, J2000, final_time);
  for (auto _ : state) {
    benchmark::DoNotOptimize(
        OrbitalElements::ForTrajectory(*trajectory, *earth_, MasslessBody{}));
  }
}

}  // namespace astronomy
}  // namespace principia
