import os

# Image dimensions
IMG_WIDTH = 480
IMG_HEIGHT = 160

# List of image numbers to analyze
image_numbers = [92, 94, 126, 131, 159, 160, 165, 172, 173, 176, 177, 178, 185, 190, 193, 194, 195, 198, 200, 203, 208, 209, 211, 212, 217]

# Dataset path
dataset_path = r"C:\Users\goldk\ESP-Camera\dataset_clean"
labels_path = os.path.join(dataset_path, "labels")

# Store all box sizes
box_widths = []
box_heights = []
box_areas = []

print("Analyzing golf ball bounding box sizes...")
print(f"Image dimensions: {IMG_WIDTH} x {IMG_HEIGHT}")
print(f"Analyzing {len(image_numbers)} images\n")

# Process each label file
for num in image_numbers:
    label_file = os.path.join(labels_path, f"golf_ball_{num:05d}.txt")
    
    if os.path.exists(label_file):
        with open(label_file, 'r') as f:
            lines = f.readlines()
            
        print(f"\nImage {num:05d}:")
        for line in lines:
            parts = line.strip().split()
            if len(parts) == 5:
                class_id, center_x, center_y, width, height = map(float, parts)
                
                # Convert normalized coordinates to pixels
                pixel_width = width * IMG_WIDTH
                pixel_height = height * IMG_HEIGHT
                pixel_area = pixel_width * pixel_height
                
                box_widths.append(pixel_width)
                box_heights.append(pixel_height)
                box_areas.append(pixel_area)
                
                print(f"  Box: {pixel_width:.1f} x {pixel_height:.1f} pixels (area: {pixel_area:.1f})")
    else:
        print(f"\nLabel file not found: {label_file}")

# Calculate statistics
if box_widths:
    print("\n" + "="*50)
    print("BOUNDING BOX STATISTICS (in pixels):")
    print("="*50)
    
    print(f"\nWidth statistics:")
    print(f"  Minimum: {min(box_widths):.1f}")
    print(f"  Maximum: {max(box_widths):.1f}")
    print(f"  Average: {sum(box_widths)/len(box_widths):.1f}")
    
    print(f"\nHeight statistics:")
    print(f"  Minimum: {min(box_heights):.1f}")
    print(f"  Maximum: {max(box_heights):.1f}")
    print(f"  Average: {sum(box_heights)/len(box_heights):.1f}")
    
    print(f"\nArea statistics:")
    print(f"  Minimum: {min(box_areas):.1f}")
    print(f"  Maximum: {max(box_areas):.1f}")
    print(f"  Average: {sum(box_areas)/len(box_areas):.1f}")
    
    print(f"\nOverall box size range:")
    print(f"  Smallest box: {min(box_widths):.1f} x {min(box_heights):.1f}")
    print(f"  Largest box: {max(box_widths):.1f} x {max(box_heights):.1f}")
    
    # Find most common size ranges
    print("\nSize distribution analysis:")
    
    # Width ranges
    width_ranges = {
        "10-20": 0,
        "20-30": 0,
        "30-40": 0,
        "40-50": 0,
        "50-60": 0,
        "60-70": 0,
        "70+": 0
    }
    
    for w in box_widths:
        if w < 20:
            width_ranges["10-20"] += 1
        elif w < 30:
            width_ranges["20-30"] += 1
        elif w < 40:
            width_ranges["30-40"] += 1
        elif w < 50:
            width_ranges["40-50"] += 1
        elif w < 60:
            width_ranges["50-60"] += 1
        elif w < 70:
            width_ranges["60-70"] += 1
        else:
            width_ranges["70+"] += 1
    
    print("\nWidth distribution:")
    for range_name, count in width_ranges.items():
        percentage = (count / len(box_widths)) * 100
        print(f"  {range_name} pixels: {count} boxes ({percentage:.1f}%)")
    
    # Height ranges
    height_ranges = {
        "10-20": 0,
        "20-30": 0,
        "30-40": 0,
        "40-50": 0,
        "50-60": 0,
        "60+": 0
    }
    
    for h in box_heights:
        if h < 20:
            height_ranges["10-20"] += 1
        elif h < 30:
            height_ranges["20-30"] += 1
        elif h < 40:
            height_ranges["30-40"] += 1
        elif h < 50:
            height_ranges["40-50"] += 1
        elif h < 60:
            height_ranges["50-60"] += 1
        else:
            height_ranges["60+"] += 1
    
    print("\nHeight distribution:")
    for range_name, count in height_ranges.items():
        percentage = (count / len(box_heights)) * 100
        print(f"  {range_name} pixels: {count} boxes ({percentage:.1f}%)")
    
    print(f"\nTotal boxes analyzed: {len(box_widths)}")
else:
    print("\nNo valid bounding boxes found in the specified images.")