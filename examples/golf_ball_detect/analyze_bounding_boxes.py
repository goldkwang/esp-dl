import os
import glob

def analyze_bounding_boxes():
    labels_dir = r"C:\Users\goldk\ESP-Camera\dataset_clean\labels"
    
    # Variables to track min and max boxes
    min_area = float('inf')
    max_area = 0
    min_box_info = None
    max_box_info = None
    
    # Dictionary to store all box info for ideal square calculation
    all_boxes = {}
    
    # Process all label files
    label_files = glob.glob(os.path.join(labels_dir, "*.txt"))
    
    for label_file in label_files:
        filename = os.path.basename(label_file)
        image_num = int(filename.replace("golf_ball_", "").replace(".txt", ""))
        
        with open(label_file, 'r') as f:
            lines = f.readlines()
            
        for line in lines:
            parts = line.strip().split()
            if len(parts) >= 5:
                class_id = int(parts[0])
                x_center = float(parts[1])
                y_center = float(parts[2])
                width = float(parts[3])
                height = float(parts[4])
                
                # Calculate area
                area = width * height
                
                # Store box info
                box_info = {
                    'image_num': image_num,
                    'filename': filename,
                    'width': width,
                    'height': height,
                    'area': area,
                    'x_center': x_center,
                    'y_center': y_center
                }
                
                if image_num not in all_boxes:
                    all_boxes[image_num] = []
                all_boxes[image_num].append(box_info)
                
                # Check for min/max
                if area < min_area:
                    min_area = area
                    min_box_info = box_info
                    
                if area > max_area:
                    max_area = area
                    max_box_info = box_info
    
    # Print results
    print("BOUNDING BOX ANALYSIS RESULTS")
    print("=" * 60)
    
    print("\nSMALLEST BOUNDING BOX:")
    print(f"  Image: golf_ball_{min_box_info['image_num']:05d}")
    print(f"  Width: {min_box_info['width']:.6f}")
    print(f"  Height: {min_box_info['height']:.6f}")
    print(f"  Area: {min_box_info['area']:.8f}")
    print(f"  Center: ({min_box_info['x_center']:.6f}, {min_box_info['y_center']:.6f})")
    
    # Calculate ideal square size for min box
    min_ideal_square = min(min_box_info['width'], min_box_info['height'])
    print(f"  Ideal square size: {min_ideal_square:.6f} x {min_ideal_square:.6f}")
    
    print("\nLARGEST BOUNDING BOX:")
    print(f"  Image: golf_ball_{max_box_info['image_num']:05d}")
    print(f"  Width: {max_box_info['width']:.6f}")
    print(f"  Height: {max_box_info['height']:.6f}")
    print(f"  Area: {max_box_info['area']:.8f}")
    print(f"  Center: ({max_box_info['x_center']:.6f}, {max_box_info['y_center']:.6f})")
    
    # Calculate ideal square size for max box
    max_ideal_square = min(max_box_info['width'], max_box_info['height'])
    print(f"  Ideal square size: {max_ideal_square:.6f} x {max_ideal_square:.6f}")
    
    # Calculate statistics
    print("\nOVERALL STATISTICS:")
    print(f"  Total images analyzed: {len(all_boxes)}")
    print(f"  Area ratio (max/min): {max_area/min_area:.2f}")
    
    # Find top 5 smallest and largest
    all_box_list = []
    for image_num, boxes in all_boxes.items():
        for box in boxes:
            all_box_list.append(box)
    
    all_box_list.sort(key=lambda x: x['area'])
    
    print("\nTOP 5 SMALLEST BOXES:")
    for i, box in enumerate(all_box_list[:5]):
        print(f"  {i+1}. golf_ball_{box['image_num']:05d} - Area: {box['area']:.8f} (W: {box['width']:.6f}, H: {box['height']:.6f})")
    
    print("\nTOP 5 LARGEST BOXES:")
    for i, box in enumerate(all_box_list[-5:]):
        print(f"  {i+1}. golf_ball_{box['image_num']:05d} - Area: {box['area']:.8f} (W: {box['width']:.6f}, H: {box['height']:.6f})")
    
    # Additional analysis for square-ness
    print("\nSQUARE-NESS ANALYSIS:")
    aspect_ratios = []
    for box in all_box_list:
        aspect_ratio = box['width'] / box['height'] if box['height'] > 0 else 0
        aspect_ratios.append({
            'image_num': box['image_num'],
            'aspect_ratio': aspect_ratio,
            'width': box['width'],
            'height': box['height']
        })
    
    # Find most square boxes (aspect ratio closest to 1.0)
    aspect_ratios.sort(key=lambda x: abs(x['aspect_ratio'] - 1.0))
    
    print("\nMOST SQUARE BOXES (aspect ratio closest to 1.0):")
    for i, box in enumerate(aspect_ratios[:5]):
        print(f"  {i+1}. golf_ball_{box['image_num']:05d} - Aspect ratio: {box['aspect_ratio']:.4f} (W: {box['width']:.6f}, H: {box['height']:.6f})")
    
    print("\nLEAST SQUARE BOXES (aspect ratio furthest from 1.0):")
    aspect_ratios.sort(key=lambda x: abs(x['aspect_ratio'] - 1.0), reverse=True)
    for i, box in enumerate(aspect_ratios[:5]):
        print(f"  {i+1}. golf_ball_{box['image_num']:05d} - Aspect ratio: {box['aspect_ratio']:.4f} (W: {box['width']:.6f}, H: {box['height']:.6f})")

if __name__ == "__main__":
    analyze_bounding_boxes()