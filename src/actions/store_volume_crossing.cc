#include "actions/store_volume_crossing.hh"

#include <cstdio>

G4String get_name(const G4StepPoint *p) {
  if  (! p                                 ) return "None";
  if  (! p -> GetTouchable()               ) return "None";
  if  (! p -> GetTouchable() -> GetVolume()) return "None";
  return p -> GetTouchable() -> GetVolume() -> GetName();
}

std::function<void(const G4Step *)>
store_volume_crossing(G4String particle, G4String from, G4String to) {
  return [particle, from, to](const G4Step *step) {
    auto pre  = step -> GetPreStepPoint();
    auto post = step -> GetPostStepPoint();

    auto part_name = step -> GetTrack() -> GetDefinition() -> GetParticleName();
    auto  pre_name = get_name(pre);
    auto post_name = get_name(post);

    if ((particle != "") && (particle != part_name)) return;
    if ((    from != "") && (    from !=  pre_name)) return;
    if ((      to != "") && (      to != post_name)) return;

    auto  pre_pos =  pre -> GetPosition();
    auto post_pos = post -> GetPosition();
    char line[200];
    sprintf( line
           , "Particle %s pre_volume %s post_volume %s pre_x %f pre_y %f pre_z %f post_x %f post_y %f post_z %f"
           , part_name.data(), pre_name.data(), post_name.data()
           ,   pre_pos.x()   ,  pre_pos.y()   ,   pre_pos.z()
           ,  post_pos.x()   , post_pos.y()   ,  post_pos.z()
           );
    std::cout << line << std::endl;
  };
}
