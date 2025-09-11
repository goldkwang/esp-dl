# Golf Ball Dataset Bounding Box Analysis Report

## Summary

Analysis of 781 golf ball images in the dataset revealed significant variations in bounding box sizes.

## Key Findings

### 1. Smallest Bounding Box
- **Image**: golf_ball_00161
- **Dimensions**: 0.045 × 0.045 (normalized)
- **Area**: 0.002025
- **Characteristics**: Perfect square, very small golf ball in the image
- **Ideal Square Size**: 0.045 × 0.045 (already square)

### 2. Largest Bounding Box  
- **Image**: golf_ball_00001
- **Dimensions**: 0.995 × 0.995 (normalized)
- **Area**: 0.990025 
- **Characteristics**: Nearly fills the entire image, perfect square
- **Ideal Square Size**: 0.995 × 0.995 (already square)

### 3. Size Ratio
- The largest box is **488.9 times** larger in area than the smallest box
- This indicates extreme variation in golf ball sizes/distances in the dataset

## Top 5 Smallest Boxes
1. golf_ball_00161: 0.045 × 0.045 (area: 0.002025)
2. golf_ball_00136: 0.050 × 0.050 (area: 0.0025)
3. golf_ball_00234: 0.050 × 0.050 (area: 0.0025)
4. golf_ball_00246: 0.050 × 0.050 (area: 0.0025)
5. golf_ball_00412: 0.050 × 0.050 (area: 0.0025)

## Top 5 Largest Boxes
1. golf_ball_00001: 0.995 × 0.995 (area: 0.990025)
2. golf_ball_00061: 0.990 × 0.995 (area: 0.98505)
3. golf_ball_00066: 0.990 × 0.985 (area: 0.97515)
4. golf_ball_00005: 0.975 × 0.980 (area: 0.9555)
5. golf_ball_00123: 0.925 × 0.925 (area: 0.855625)

## Square-ness Analysis

### Most Square Boxes (Aspect Ratio = 1.0)
Many boxes are perfect squares, including:
- golf_ball_00161, 00136, 00234, 00246, 00412

### Least Square Boxes
1. golf_ball_00019: Aspect ratio 1.825 (wide rectangle: 0.173 × 0.095)
2. golf_ball_00188: Aspect ratio 1.750 (wide rectangle: 0.140 × 0.080)
3. golf_ball_00203: Aspect ratio 0.256 (tall rectangle: 0.125 × 0.488)
4. golf_ball_00159: Aspect ratio 0.264 (tall rectangle: 0.150 × 0.569)
5. golf_ball_00160: Aspect ratio 0.264 (tall rectangle: 0.150 × 0.569)

## Ideal Square Box Calculations

For each bounding box, the ideal square size is calculated as:
- **Ideal Square Side = min(width, height)**

This ensures the golf ball remains fully contained within the square box.

Examples:
- Non-square box (0.173 × 0.095) → Ideal square: 0.095 × 0.095
- Non-square box (0.125 × 0.488) → Ideal square: 0.125 × 0.125

## Recommendations

1. **Data Quality**: The extreme variation suggests different capture conditions (distance, zoom levels)
2. **Preprocessing**: Consider normalizing box sizes for training
3. **Square Boxes**: Many boxes are already square, but non-square ones should be converted to squares using the smaller dimension
4. **Outlier Handling**: Very small boxes (< 0.05) might be too small for effective training

## Visualization Files Generated
- `extreme_bounding_boxes.png`: Overview of 5 extreme cases
- `golf_ball_00161_smallest_box.png`: Detailed view of smallest box
- `golf_ball_00001_largest_box.png`: Detailed view of largest box
- Red boxes show original annotations, green dashed boxes show ideal squares