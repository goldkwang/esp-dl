#!/usr/bin/env python3
"""Analyze OV2640 TEST register and identify the real issue with 568x264 ROI"""

def analyze_test_register():
    """Analyze the TEST register bit mapping for OV2640"""
    print("OV2640 TEST Register (0x57) Analysis")
    print("=====================================")
    print()
    
    # From various OV2640 datasheets and drivers:
    # TEST[7] is used for zoom window horizontal size bit 9
    # But the calculation varies in different implementations
    
    print("Different TEST register implementations found:")
    print()
    
    print("1. ArduCAM driver: TEST[7] = hsize[2]")
    print("   This means: bit 2 of hsize goes to bit 7 of TEST")
    print("   Formula: (hsize & 0x04) << 5")
    print()
    
    print("2. Some drivers: TEST[7] = hsize[9]")
    print("   This means: bit 9 of hsize goes to bit 7 of TEST")
    print("   Formula: (hsize >> 2) & 0x80")
    print()
    
    print("3. ESP32 camera driver: No TEST register setting")
    print("   Many working implementations don't set TEST at all")
    print()
    
    # Test our ROI values
    width = 568
    hsize = width // 4  # 142
    
    print(f"For width={width}, hsize={hsize} (0x{hsize:02X}):")
    print(f"Binary: 0b{hsize:010b}")
    print()
    
    # Different calculations
    test1 = (hsize & 0x04) << 5  # bit 2 to bit 7
    test2 = (hsize >> 2) & 0x80  # Current (wrong)
    test3 = ((hsize >> 9) & 1) << 7  # bit 9 to bit 7
    
    print(f"Method 1 (bit 2 -> bit 7): 0x{test1:02X}")
    print(f"Method 2 (current wrong): 0x{test2:02X}")
    print(f"Method 3 (bit 9 -> bit 7): 0x{test3:02X}")
    print()
    
    # Check if 544 vs 568 difference
    print("Comparing 544 vs 568 pixel widths:")
    for w in [544, 568]:
        h = w // 4
        print(f"  Width {w}: hsize={h} (0x{h:02X}), binary: 0b{h:010b}")
        print(f"    bit 2: {(h >> 2) & 1}")
        print(f"    bit 7: {(h >> 7) & 1}")
        print(f"    bit 8: {(h >> 8) & 1}")
        print(f"    bit 9: {(h >> 9) & 1}")
    print()
    
    # Real issue analysis
    print("REAL ISSUE ANALYSIS:")
    print("===================")
    print("544 pixels = 136 (0x88) = 0b010001000")
    print("568 pixels = 142 (0x8E) = 0b010001110")
    print()
    print("Difference: bits 1,2,3 are different")
    print("544: ...01000 (bits 3-0: 0x8)")
    print("568: ...01110 (bits 3-0: 0xE)")
    print()
    print("The issue might not be TEST register but something else!")

if __name__ == "__main__":
    analyze_test_register()