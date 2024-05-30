// for your adjustment; xy tolerance
tolerance = 0.25;

cap_w = 19;
switch_w = 14;
switch_t = 1.4; // thickness of the "plate" that the switches clip into
kb_l = cap_w * 10 + (cap_w - switch_w);
kb_w = cap_w * 3.5 + (cap_w - switch_w);
kb_h = 10;
thickness = 3;

switch_positions = [
  // top row
  [-4.5, 1.25],
  [-3.5, 1.25],
  [-3.5, 1.25],
  [-2.5, 1.25],
  [-1.5, 1.25],
  [-0.5, 1.25],
  [ 0.5, 1.25],
  [ 1.5, 1.25],
  [ 2.5, 1.25],
  [ 3.5, 1.25],
  [ 4.5, 1.25],
  
  // second row
  [-4.5, 0.25],
  [-3.5, 0.25],
  [-3.5, 0.25],
  [-2.5, 0.25],
  [-1.5, 0.25],
  [-0.5, 0.25],
  [ 0.5, 0.25],
  [ 1.5, 0.25],
  [ 2.5, 0.25],
  [ 3.5, 0.25],
  [ 4.5, 0.25],

  // thumb row
  [-2.25, -1.25],
  [-1.25, -1.25],
  [ 0.25, -1.25],
  [ 1.25, -1.25],
];

difference() {
  // keyboard shell
  cube([kb_l, kb_w, kb_h], center = true);

  // key switches
  for (pos = switch_positions) {
    // the hole for the switch
    translate([pos[0] * cap_w, pos[1] * cap_w, 0]) {
      cube(switch_w + tolerance * 2, center = true);
    }

    // the thin spots in the switch plate
    translate([pos[0] * cap_w, pos[1] * cap_w, 0]) {
      cube([5 + tolerance * 2, switch_w + 2 + tolerance * 2, kb_h - switch_t * 2], center = true);
    }
  }

  // make the keyboard shell hollow
  translate([0, 0, -thickness]) {
    cube([kb_l - thickness, kb_w - thickness, kb_h], center = true);
  }
}
