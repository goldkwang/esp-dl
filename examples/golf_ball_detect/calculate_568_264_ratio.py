#!/usr/bin/env python3
"""Calculate ROI with 568x264 aspect ratio within 544 pixel limit"""

# Target aspect ratio
target_width = 568
target_height = 264
target_ratio = target_width / target_height

print(f"Target ROI: {target_width}×{target_height}")
print(f"Target aspect ratio: {target_ratio:.3f}:1")
print()

# Calculate options within 544 pixel width limit
print("Options within 544 pixel limit:")
print("="*50)

# Option 1: Maximum width (544)
width1 = 544
height1 = width1 / target_ratio
height1_rounded = round(height1 / 8) * 8  # Round to multiple of 8
print(f"Option 1 - Width 544:")
print(f"  Calculated height: {height1:.1f}")
print(f"  Rounded to 8x: {height1_rounded}")
print(f"  Final ROI: {width1}×{height1_rounded}")
print(f"  Actual ratio: {width1/height1_rounded:.3f}:1")
print()

# Option 2: Try common heights
for h in [256, 248, 240, 232]:
    w = h * target_ratio
    w_rounded = round(w / 8) * 8
    if w_rounded <= 544:
        print(f"Height {h}: Width = {w:.1f} → {w_rounded} (ratio: {w_rounded/h:.3f}:1)")

print()
print("Recommended setting:")
print("="*50)
# 544x256 gives ratio of 2.125:1, very close to 2.152:1
rec_width = 544
rec_height = 256
print(f"ROI: {rec_width}×{rec_height}")
print(f"Aspect ratio: {rec_width/rec_height:.3f}:1 (target: {target_ratio:.3f}:1)")
print(f"Difference: {abs(rec_width/rec_height - target_ratio):.3f}")

# Calculate centered offsets
frame_w = 1280
frame_h = 1024
offset_x = (frame_w - rec_width) // 2
offset_y = (frame_h - rec_height) // 2
offset_x = (offset_x // 8) * 8
offset_y = (offset_y // 8) * 8
print(f"\nCentered offsets: x={offset_x}, y={offset_y}")