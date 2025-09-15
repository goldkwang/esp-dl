#!/usr/bin/env python3
"""Check if 568x264 values are multiples of 8"""

def check_multiple_of_8(value, name):
    """Check if a value is a multiple of 8"""
    is_multiple = (value % 8) == 0
    print(f"{name}: {value}")
    print(f"  {value} ÷ 8 = {value / 8}")
    print(f"  {value} % 8 = {value % 8}")
    print(f"  Is multiple of 8: {is_multiple}")
    print()
    return is_multiple

print("Checking 568×264 values:")
print("="*40)

# Check dimensions
width_ok = check_multiple_of_8(568, "Width")
height_ok = check_multiple_of_8(264, "Height")

# Also check typical offsets for centering
frame_width = 1280
frame_height = 1024
roi_width = 568
roi_height = 264

offset_x = (frame_width - roi_width) // 2
offset_y = (frame_height - roi_height) // 2

print("Calculated offsets for centering:")
print("="*40)
offset_x_ok = check_multiple_of_8(offset_x, "X Offset")
offset_y_ok = check_multiple_of_8(offset_y, "Y Offset")

# Suggest rounded offsets if needed
if not offset_x_ok:
    offset_x_rounded = (offset_x // 8) * 8
    print(f"Suggested X offset: {offset_x_rounded}")

if not offset_y_ok:
    offset_y_rounded = (offset_y // 8) * 8
    print(f"Suggested Y offset: {offset_y_rounded}")

print("\nSummary:")
print("="*40)
print(f"568×264 dimensions are valid: {width_ok and height_ok}")
print(f"All values are multiples of 8: {width_ok and height_ok and offset_x_ok and offset_y_ok}")

# Final recommended values
offset_x_final = (offset_x // 8) * 8
offset_y_final = (offset_y // 8) * 8
print(f"\nRecommended ROI settings:")
print(f"  Width: {roi_width}")
print(f"  Height: {roi_height}")
print(f"  X Offset: {offset_x_final}")
print(f"  Y Offset: {offset_y_final}")