#!/usr/bin/env python3

def check_pattern(width):
    """Check specific pattern that might cause issues"""
    hsize = width // 4
    
    # Check various bit patterns
    bit_7 = (hsize >> 7) & 1
    low_4_bits = hsize & 0x0F
    
    # Pattern observation
    return {
        'width': width,
        'hsize': hsize,
        'hex': f"0x{hsize:02X}",
        'binary': f"{hsize:08b}",
        'bit_7': bit_7,
        'low_4_bits': f"0x{low_4_bits:01X}",
        'modulo_16': hsize % 16,
    }

print("=== PATTERN ANALYSIS ===")
print("Looking for pattern differences between working/non-working widths")
print()

# Test around the threshold
for w in range(528, 584, 8):
    result = check_pattern(w)
    status = "WORKS" if w <= 544 else "FAILS"
    print(f"{status}: Width={result['width']:3d}, hsize={result['hsize']:3d} ({result['hex']}), "
          f"binary={result['binary']}, bit7={result['bit_7']}, "
          f"low4bits={result['low_4_bits']}, mod16={result['modulo_16']}")

# Check if it's related to specific values
print("\n=== CHECKING SPECIFIC THRESHOLDS ===")

# 544/4 = 136 = 0x88
# 552/4 = 138 = 0x8A 
print(f"544/4 = {544//4} = 0x{544//4:02X}")
print(f"552/4 = {552//4} = 0x{552//4:02X}")
print(f"Difference: {552//4 - 544//4}")

# Let me check if it's exactly at 137
print("\n=== TESTING EXACT BOUNDARY ===")
for hsize in range(135, 145):
    width = hsize * 4
    status = "WORKS" if width <= 544 else "FAILS"
    print(f"hsize={hsize:3d} (0x{hsize:02X}) -> width={width:3d} : {status}")

# Check OV2640 specific limitation
print("\n=== OV2640 WINDOW SIZE ANALYSIS ===")
# The TEST register (0x57) uses bit 7 for extended HSIZE
# Could be related to a specific internal buffer or processing limitation

for w in [544, 548, 552]:
    hsize = w // 4
    test_reg_bit = (hsize >> 2) & 0x80  # From official code: (max_x >> 2) & 0X80
    print(f"Width {w}: hsize={hsize} (0x{hsize:02X}), TEST reg bit 7 = 0x{test_reg_bit:02X}")