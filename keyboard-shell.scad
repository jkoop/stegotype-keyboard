cap_w = 19;
switch_w = 14;
kb_l = cap_w * 10 + (cap_w - switch_w);
kb_w = cap_w * 3.5 + (cap_w - switch_w);
kb_h = 10;
thickness = 1.5;

difference() {
  // keyboard shell
  cube([kb_l, kb_w, kb_h], center = true);

  // key switches
  for (i = [0: 9]) {
    translate([(i - 4.5) * cap_w, cap_w * 1.25, 0]) {
      cube(switch_w, center = true);
    }
    translate([(i - 4.5) * cap_w, cap_w * 0.25, 0]) {
      cube(switch_w, center = true);
    }
  }
  for (i = [0: 1]) {
    translate([i * cap_w + 0.25 * cap_w, cap_w * -1.25, 0]) {
      cube(switch_w, center = true);
    }
    translate([(-i - 1) * cap_w - 0.25 * cap_w, cap_w * -1.25, 0]) {
      cube(switch_w, center = true);
    }
  }

  // make the keyboard shell hollow
  translate([0, 0, -thickness]) {
    cube([kb_l - thickness, kb_w - thickness, kb_h], center = true);
  }
}
