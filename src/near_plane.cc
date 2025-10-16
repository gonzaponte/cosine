#include "near_plane.hh"
#include "ptfe.hh"
#include "silicon.hh"

#include <n4-geometry.hh>

G4LogicalVolume* build_near_plane(G4double sipm_size, G4double sipm_thick, G4double sipm_gap, G4int n_sipm_side) {
  auto odd          = n_sipm_side % 2 == 1;
  auto pitch        = sipm_size + sipm_gap;
  auto support_size = pitch * n_sipm_side + sipm_gap;
  auto offset       = odd ? 0. : pitch / 2;

  auto silicon = si_with_properties();
  auto ptfe    = ptfe_with_properties();

  auto support = n4::box("near_plane_support").xy(support_size).z(2*sipm_thick).volume(ptfe);
  auto sipm    = n4::box("sipm").xy(sipm_size).z(sipm_thick).volume(silicon);

  for   (auto i = 0; i < n_sipm_side; i++) {
    for (auto j = 0; j < n_sipm_side; j++) {

    }
  }
}
