#!/usr/bin/env python3
"""Test ROI register calculations for OV2640"""

def calculate_roi_registers(width, height, offset_x, offset_y):
    """Calculate OV2640 ROI register values"""
    # Divide by 4 for register values
    hsize = width // 4
    vsize = height // 4
    hoff = offset_x // 4
    voff = offset_y // 4
    
    print(f"ROI: {width}x{height} @ ({offset_x}, {offset_y})")
    print(f"hsize = {width}/4 = {hsize} (0x{hsize:02X}) = 0b{hsize:09b}")
    print(f"vsize = {height}/4 = {vsize} (0x{vsize:02X}) = 0b{vsize:09b}")
    print(f"hoff = {offset_x}/4 = {hoff} (0x{hoff:02X}) = 0b{hoff:011b}")
    print(f"voff = {offset_y}/4 = {voff} (0x{voff:02X}) = 0b{voff:011b}")
    print()
    
    # Register values
    print("Register values:")
    print(f"HSIZE = 0x{hsize & 0xff:02X}")
    print(f"VSIZE = 0x{vsize & 0xff:02X}")
    print(f"XOFFL = 0x{hoff & 0xff:02X}")
    print(f"YOFFL = 0x{voff & 0xff:02X}")
    
    # VHYX calculation
    vhyx = ((vsize >> 1) & 0x80) | ((voff >> 4) & 0x70) | ((hsize >> 5) & 0x08) | ((hoff >> 8) & 0x07)
    print(f"\nVHYX calculation:")
    print(f"  vsize[8] = {(vsize >> 8) & 1} -> shifted: 0x{((vsize >> 1) & 0x80):02X}")
    print(f"  voff[10:8] = {(voff >> 8) & 7} -> shifted: 0x{((voff >> 4) & 0x70):02X}")
    print(f"  hsize[8] = {(hsize >> 8) & 1} -> shifted: 0x{((hsize >> 5) & 0x08):02X}")
    print(f"  hoff[10:8] = {(hoff >> 8) & 7} -> shifted: 0x{((hoff >> 8) & 0x07):02X}")
    print(f"  VHYX = 0x{vhyx:02X}")
    
    # ZMHH calculation
    zmhh = ((vsize >> 6) & 0x04) | ((hsize >> 8) & 0x03)
    print(f"\nZMHH calculation:")
    print(f"  vsize[8] = {(vsize >> 8) & 1} -> shifted: 0x{((vsize >> 6) & 0x04):02X}")
    print(f"  hsize[9:8] = {(hsize >> 8) & 3} -> shifted: 0x{((hsize >> 8) & 0x03):02X}")
    print(f"  ZMHH = 0x{zmhh:02X}")
    
    # TEST calculation (WRONG in current code!)
    test_wrong = (hsize >> 2) & 0x80
    test_correct = ((hsize >> 9) & 1) << 7  # bit 9 of hsize goes to bit 7
    print(f"\nTEST calculation:")
    print(f"  Current (WRONG): (hsize >> 2) & 0x80 = ({hsize} >> 2) & 0x80 = {hsize >> 2} & 0x80 = 0x{test_wrong:02X}")
    print(f"  Correct: hsize[9] << 7 = {(hsize >> 9) & 1} << 7 = 0x{test_correct:02X}")
    
    print("\n" + "="*50 + "\n")

# Test cases
print("Current ROI settings:")
calculate_roi_registers(568, 264, 352, 520)

print("Working ROI (544x400):")
calculate_roi_registers(544, 400, 368, 312)

print("Maximum width test cases:")
calculate_roi_registers(512, 264, 384, 520)  # bit 9 = 0
calculate_roi_registers(544, 264, 368, 520)  # bit 9 = 0  
calculate_roi_registers(576, 264, 352, 520)  # bit 9 = 1
calculate_roi_registers(640, 264, 320, 520)  # bit 9 = 1