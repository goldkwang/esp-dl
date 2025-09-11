import os

# Image dimensions
IMG_WIDTH = 480
IMG_HEIGHT = 160

# List of images in the "실제 공촬영" folder
target_images = [
    "golf_ball_00092", "golf_ball_00094", "golf_ball_00104", "golf_ball_00126",
    "golf_ball_00131", "golf_ball_00159", "golf_ball_00160", "golf_ball_00165",
    "golf_ball_00172", "golf_ball_00173", "golf_ball_00176", "golf_ball_00177",
    "golf_ball_00178", "golf_ball_00185", "golf_ball_00190", "golf_ball_00193",
    "golf_ball_00194", "golf_ball_00195", "golf_ball_00198", "golf_ball_00200",
    "golf_ball_00203", "golf_ball_00208", "golf_ball_00209", "golf_ball_00211",
    "golf_ball_00212", "golf_ball_00217"
]

labels_dir = r"C:\Users\goldk\ESP-Camera\dataset_clean\labels"

# Store all box sizes
box_sizes = []
image_box_data = {}

# Process each image
for img_name in target_images:
    label_file = os.path.join(labels_dir, f"{img_name}.txt")
    
    if os.path.exists(label_file):
        with open(label_file, 'r') as f:
            lines = f.readlines()
            
        image_boxes = []
        for line in lines:
            parts = line.strip().split()
            if len(parts) >= 5:
                # YOLO format: class_id center_x center_y width height (normalized)
                class_id = int(parts[0])
                center_x = float(parts[1])
                center_y = float(parts[2])
                norm_width = float(parts[3])
                norm_height = float(parts[4])
                
                # Convert to pixel sizes
                pixel_width = norm_width * IMG_WIDTH
                pixel_height = norm_height * IMG_HEIGHT
                
                box_info = {
                    'width': pixel_width,
                    'height': pixel_height,
                    'area': pixel_width * pixel_height
                }
                
                image_boxes.append(box_info)
                box_sizes.append((img_name, pixel_width, pixel_height, pixel_width * pixel_height))
        
        if image_boxes:
            image_box_data[img_name] = image_boxes

# Find statistics
if box_sizes:
    # Sort by area (width * height)
    box_sizes.sort(key=lambda x: x[3])
    
    # Smallest and largest boxes
    smallest = box_sizes[0]
    largest = box_sizes[-1]
    
    # Calculate averages
    avg_width = sum(box[1] for box in box_sizes) / len(box_sizes)
    avg_height = sum(box[2] for box in box_sizes) / len(box_sizes)
    avg_area = sum(box[3] for box in box_sizes) / len(box_sizes)
    
    # Print results
    print("=== Golf Ball Size Analysis ===")
    print(f"Total golf balls analyzed: {len(box_sizes)}")
    print(f"\nImage dimensions: {IMG_WIDTH}x{IMG_HEIGHT} pixels")
    
    print(f"\n--- Smallest Golf Ball ---")
    print(f"Image: {smallest[0]}.jpg")
    print(f"Size: {smallest[1]:.1f} x {smallest[2]:.1f} pixels")
    print(f"Area: {smallest[3]:.1f} square pixels")
    
    print(f"\n--- Largest Golf Ball ---")
    print(f"Image: {largest[0]}.jpg")
    print(f"Size: {largest[1]:.1f} x {largest[2]:.1f} pixels")
    print(f"Area: {largest[3]:.1f} square pixels")
    
    print(f"\n--- Average Sizes ---")
    print(f"Average width: {avg_width:.1f} pixels")
    print(f"Average height: {avg_height:.1f} pixels")
    print(f"Average area: {avg_area:.1f} square pixels")
    
    # Additional statistics
    widths = [box[1] for box in box_sizes]
    heights = [box[2] for box in box_sizes]
    
    print(f"\n--- Size Ranges ---")
    print(f"Width range: {min(widths):.1f} - {max(widths):.1f} pixels")
    print(f"Height range: {min(heights):.1f} - {max(heights):.1f} pixels")
    
    # Show distribution
    print(f"\n--- Size Distribution ---")
    print("Images with multiple golf balls:")
    for img_name, boxes in image_box_data.items():
        if len(boxes) > 1:
            print(f"  {img_name}.jpg: {len(boxes)} golf balls")
    
    # Show all sizes for detailed analysis
    print(f"\n--- All Golf Ball Sizes ---")
    for i, (img_name, width, height, area) in enumerate(box_sizes):
        print(f"{i+1}. {img_name}.jpg: {width:.1f} x {height:.1f} pixels (area: {area:.1f})")
else:
    print("No golf ball labels found for the specified images.")