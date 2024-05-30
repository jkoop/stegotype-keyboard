cap_w = 19;
switch_h = 6; // distance from top of switch plate to top of switch (excl. stem)
stem_h = 4; // distance from top of switch to top of stem
stem_w = 4; // maximum distance across the stem
stem_t = 1; // thickness of the lines in the stem's "plus" shape
thickness_x = 1;
thickness_z = 2;

cap_h = switch_h + thickness_z;

translate([0, 0, -cap_h / 2 + thickness_z]) {
  difference() {
    cube([cap_w, cap_w, cap_h], center = true);
    translate([0, 0, -thickness_z]) {
      cube([
          cap_w - thickness_x,
          cap_w - thickness_x,
          cap_h,
        ], center = true);
    }
  }
}

translate([0, 0, -stem_h / 2]) {
  difference() {
    cube([stem_w, stem_w, stem_h], center = true);
    cube([stem_t, stem_w + 1, stem_h + 1], center = true);
    cube([stem_w + 1, stem_t, stem_h + 1], center = true);
  }
}