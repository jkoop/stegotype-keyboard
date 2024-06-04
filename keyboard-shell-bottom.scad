cap_w = 19;
switch_w = 14;
kb_l = cap_w * 10 + (cap_w - switch_w);
kb_w = cap_w * 3.5 + (cap_w - switch_w);
kb_h = 10;
thickness = 3;
batt_w = 14 + 2;
batt_l = 50 + 2;
batt_x = 4;
batt_y = 18;

difference() {
  // keyboard shell
  cube([kb_l, kb_w, kb_h]);

  // make the keyboard shell hollow
  translate([thickness / 2, thickness / 2, thickness]) {
    cube([kb_l - thickness, kb_w - thickness, kb_h - thickness]);
  }

  // battery door
  translate([batt_x, batt_y, 0]) {
    cube([batt_l, batt_w, thickness]);
  }
}