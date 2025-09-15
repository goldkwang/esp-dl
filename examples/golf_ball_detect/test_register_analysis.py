#!/usr/bin/env python3

print("=== TEST REGISTER BIT 7 ANALYSIS ===")
print("Official formula: (max_x >> 2) & 0x80")
print()

# Test the boundary values
for width in range(536, 576, 8):
    hsize = width // 4
    
    # Official formula for TEST register bit 7
    test_bit_7 = (hsize >> 2) & 0x80
    
    # When does bit 9 of hsize get set?
    # hsize >> 2 shifts right by 2, then & 0x80 checks bit 7
    # So this is checking bit 9 of original hsize
    bit_9 = (hsize >> 9) & 1
    
    status = "WORKS" if width <= 544 else "FAILS"
    
    print(f"{status}: Width={width:3d}, hsize={hsize:3d} (0x{hsize:03X}), "
          f"binary={hsize:012b}, bit9={bit_9}, TEST_bit7=0x{test_bit_7:02X}")

print("\n=== BIT 9 TRANSITION ===")
# Bit 9 gets set when hsize >= 512
print(f"hsize=511 (0x1FF): bit 9 = {(511 >> 9) & 1}")
print(f"hsize=512 (0x200): bit 9 = {(512 >> 9) & 1}")

print("\n=== WAIT, LET ME CHECK THE CURRENT CODE ===")
# The current code doesn't set TEST register at all!
# This might be the issue

print("\nCurrent code is MISSING the TEST register setting!")
print("It should include:")
print("sensor->set_reg(sensor, REG_TEST, 0x80, (hsize >> 2) & 0x80);")

# But wait, for our range, TEST bit would be 0 anyway
print("\nFor our ROI sizes:")
for w in [544, 568]:
    h = w // 4
    print(f"Width {w}: hsize={h}, TEST bit 7 = {(h >> 2) & 0x80}")

print("\n=== REAL ISSUE FOUND ===")
print("The exact threshold is hsize > 136")
print("This suggests an OV2640 hardware limitation or buffer size")
print("136 * 4 = 544 pixels max width for this specific ROI configuration")