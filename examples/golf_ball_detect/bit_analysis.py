#!/usr/bin/env python3

def analyze_width(width):
    """Analyze bit patterns for different widths"""
    hsize = width // 4
    print(f"\nWidth: {width} pixels")
    print(f"hsize: {hsize} (0x{hsize:02X}) = {bin(hsize)}")
    
    # Current wrong implementation
    wrong_vhyx = (hsize >> 6) & 0x0c
    print(f"Current (wrong): (hsize >> 6) & 0x0c = 0x{wrong_vhyx:02X}")
    
    # Correct implementation 
    correct_vhyx = (hsize >> 5) & 0x08
    print(f"Correct: (hsize >> 5) & 0x08 = 0x{correct_vhyx:02X}")
    
    # ZMHH calculations
    zmhh_wrong = ((0 >> 8) & 0x0f) | (((hsize >> 8) & 0x0f) << 4)
    zmhh_correct = ((0 >> 6) & 0x04) | ((hsize >> 8) & 0x03)
    print(f"ZMHH wrong: 0x{zmhh_wrong:02X}")
    print(f"ZMHH correct: 0x{zmhh_correct:02X}")
    
    # Check bit patterns
    print(f"Bit 8 of hsize: {(hsize >> 8) & 1}")
    print(f"Bit 9 of hsize: {(hsize >> 9) & 1}")

# Test various widths
print("=== ANALYZING WIDTH PATTERNS ===")

# Working widths
analyze_width(512)
analyze_width(536)  
analyze_width(544)

# Non-working widths
analyze_width(552)
analyze_width(560)
analyze_width(568)
analyze_width(576)

# Larger widths
analyze_width(1024)
analyze_width(1280)

# Find threshold
print("\n=== FINDING THRESHOLD ===")
for w in range(544, 576, 8):
    hsize = w // 4
    correct_vhyx = (hsize >> 5) & 0x08
    print(f"Width {w}: hsize={hsize} (0x{hsize:02X}), VHYX bit 3 = {correct_vhyx >> 3}")