#!/usr/bin/env python3
"""Analyze aspect ratio of ROI settings"""

def analyze_aspect_ratio(width, height):
    """Calculate and display aspect ratio"""
    ratio = width / height
    print(f"ROI: {width}×{height}")
    print(f"Aspect ratio: {ratio:.2f}:1 ({width}/{height})")
    print(f"Width is {ratio:.2f}x the height")
    print()

print("Current ROI Analysis:")
print("=" * 40)

# Current setting
analyze_aspect_ratio(544, 400)

# Target ~2:1 ratio options
print("Options for ~2:1 aspect ratio:")
print("=" * 40)

# Option 1: Keep width at 544
width1 = 544
height1 = width1 // 2
height1 = (height1 // 8) * 8  # Round to multiple of 8
print(f"Option 1 - Keep width 544:")
analyze_aspect_ratio(width1, height1)

# Option 2: Keep height at 400
height2 = 400
width2 = height2 * 2
width2 = (width2 // 8) * 8  # Round to multiple of 8
print(f"Option 2 - Keep height 400:")
analyze_aspect_ratio(width2, height2)

# Option 3: Smaller ROI with 2:1 ratio
print("Option 3 - Smaller ROIs with 2:1 ratio:")
for w in [512, 480, 448, 416]:
    h = w // 2
    analyze_aspect_ratio(w, h)

# Calculate offsets for centering
print("\nCentering calculations (for 1280×1024 frame):")
print("=" * 40)
frame_width = 1280
frame_height = 1024

for w, h in [(544, 272), (512, 256), (480, 240)]:
    offset_x = (frame_width - w) // 2
    offset_y = (frame_height - h) // 2
    # Round to multiple of 8
    offset_x = (offset_x // 8) * 8
    offset_y = (offset_y // 8) * 8
    print(f"{w}×{h}: offset_x={offset_x}, offset_y={offset_y}")