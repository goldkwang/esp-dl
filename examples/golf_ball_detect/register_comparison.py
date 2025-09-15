#!/usr/bin/env python3

def analyze_registers(width):
    """Analyze register values for a given ROI width"""
    hsize = width // 4
    
    # Current (incorrect) implementation
    hsize_reg = hsize & 0xff
    vhyx_current = (hsize >> 6) & 0x0c
    zmow_reg = hsize & 0xff
    zmhh_current = (hsize & 0xf0)  # Width portion only
    
    # Official OV2640 implementation
    vhyx_official = (hsize >> 5) & 0x08
    zmhh_official = (hsize >> 8) & 0x03
    
    # TEST register (if needed)
    test_bit7 = (hsize >> 2) & 0x80
    
    return {
        'width': width,
        'hsize': hsize,
        'hsize_binary': f'{hsize:012b}',
        'hsize_reg': f'0x{hsize_reg:02X}',
        'vhyx_current': f'0x{vhyx_current:02X}',
        'vhyx_official': f'0x{vhyx_official:02X}',
        'zmow_reg': f'0x{zmow_reg:02X}',
        'zmhh_current': f'0x{zmhh_current:02X}',
        'zmhh_official': f'0x{zmhh_official:02X}',
        'test_bit7': f'0x{test_bit7:02X}',
        'bit8': (hsize >> 8) & 1,
        'bit9': (hsize >> 9) & 1
    }

print("=== REGISTER COMPARISON: CURRENT vs OFFICIAL IMPLEMENTATION ===\n")

# Test critical widths
widths = [512, 536, 544, 552, 560, 568, 576, 1024]

print(f"{'Width':>5} | {'hsize':>5} | {'Binary':>12} | {'HSIZE':>5} | {'VHYX':>12} | {'ZMHH':>12} | {'TEST':>4} | {'Status':>8}")
print(f"{'':>5} | {'':>5} | {'':>12} | {'':>5} | {'Curr/Off':>12} | {'Curr/Off':>12} | {'B7':>4} | {'':>8}")
print("-" * 85)

for width in widths:
    r = analyze_registers(width)
    status = "WORKS" if width <= 544 else "FAILS"
    vhyx_comp = f"{r['vhyx_current']}/{r['vhyx_official']}"
    zmhh_comp = f"{r['zmhh_current']}/{r['zmhh_official']}"
    
    print(f"{r['width']:>5} | {r['hsize']:>5} | {r['hsize_binary']:>12} | "
          f"{r['hsize_reg']:>5} | {vhyx_comp:>12} | {zmhh_comp:>12} | "
          f"{r['test_bit7']:>4} | {status:>8}")

print("\n=== KEY FINDINGS ===\n")

# Check bit transitions
print("1. Bit 8 transition (affects VHYX and ZMHH):")
for w in [1020, 1024, 1028]:
    h = w // 4
    bit8 = (h >> 8) & 1
    print(f"   Width {w}: hsize={h}, bit 8 = {bit8}")

print("\n2. Bit 9 transition (affects TEST register):")
for w in [2044, 2048, 2052]:
    h = w // 4
    bit9 = (h >> 9) & 1
    test = (h >> 2) & 0x80
    print(f"   Width {w}: hsize={h}, bit 9 = {bit9}, TEST bit 7 = 0x{test:02X}")

print("\n3. Current vs Official Implementation Differences:")
print("   - VHYX: Current uses (hsize >> 6) & 0x0c, Official uses (hsize >> 5) & 0x08")
print("   - ZMHH: Current uses width bits 4-7, Official uses width bits 8-9")

print("\n4. The 544 Pixel Threshold:")
print(f"   - Width 544: hsize = 136 = 0x88")
print(f"   - Width 568: hsize = 142 = 0x8E")
print(f"   - Difference: Only lower 3 bits differ (0x88 vs 0x8E)")
print(f"   - This suggests a hardware limitation at hsize > 136")

# Additional analysis - what changes at the boundary?
print("\n=== BOUNDARY ANALYSIS ===\n")
for width in range(536, 576, 8):
    hsize = width // 4
    status = "✓" if width <= 544 else "✗"
    print(f"{status} Width {width}: hsize={hsize:3d} (0x{hsize:02X}) = {hsize:08b}")

print("\nConclusion: The OV2640 appears to have a hardware limitation where")
print("ROI width cannot exceed 544 pixels (hsize 136) in this configuration.")
print("This might be related to internal buffer sizes or undocumented constraints.")