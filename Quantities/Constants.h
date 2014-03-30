﻿// Constants.h

#pragma once

#include "Quantities.h"
#include "NamedQuantities.h"
#include "SI.h"

namespace Quantities {
Speed const SpeedOfLight = 299792458 * (SI::Metre / SI::Second);
Permeability const MagneticConstant = 4e-7 * Numbers::π *
                                      SI::Steradian * SI::Henry / SI::Metre;
Permittivity const ElectricConstant = 1 /
  (MagneticConstant * SpeedOfLight * SpeedOfLight);
// We use the 2010 CODATA recommended values. We do not support uncertainties.
Action const ReducedPlanckConstant = 1.054571726e-34 * (SI::Joule * SI::Second);
auto const GravitationalConstant =
  6.67384e-11 * SI::Newton * SI::Metre.Pow<2>() / SI::Kilogram.Pow<2>();

Entropy const BoltzmannConstant = 1.3806488e-23 * (SI::Joule / SI::Kelvin);
Inverse<Amount> const AvogadroConstant = 6.02214129 * (1 / SI::Mole);

Mass   const ElectronMass     = 9.10938291e-31 * SI::Kilogram;
Mass   const ProtonMass       = 1.672621777e-27 * SI::Kilogram;
Charge const ElementaryCharge = 1.602176565e-19 * SI::Coulomb;

Dimensionless const FineStructureConstant = 7.2973525698e-3;
}
