import cv2
import os
import matplotlib.pyplot as plt
import matplotlib.patches as patches

def visualize_extreme_boxes():
    # Paths
    images_dir = r"C:\Users\goldk\ESP-Camera\dataset_clean\images"
    labels_dir = r"C:\Users\goldk\ESP-Camera\dataset_clean\labels"
    
    # Cases to visualize
    cases = [
        ("00161", "Smallest Box"),
        ("00001", "Largest Box"),
        ("00136", "Small Square Box"),
        ("00019", "Most Non-Square (Wide)"),
        ("00203", "Most Non-Square (Tall)")
    ]
    
    # Create figure with subplots
    fig, axes = plt.subplots(2, 3, figsize=(15, 10))
    axes = axes.flatten()
    
    for idx, (img_num, title) in enumerate(cases):
        if idx >= 6:
            break
            
        # Read image
        img_path = os.path.join(images_dir, f"golf_ball_{img_num}.jpg")
        img = cv2.imread(img_path)
        if img is None:
            continue
            
        img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
        height, width = img.shape[:2]
        
        # Read label
        label_path = os.path.join(labels_dir, f"golf_ball_{img_num}.txt")
        with open(label_path, 'r') as f:
            line = f.readline().strip()
            
        parts = line.split()
        x_center = float(parts[1])
        y_center = float(parts[2])
        box_width = float(parts[3])
        box_height = float(parts[4])
        
        # Convert normalized coordinates to pixel coordinates
        x_center_px = x_center * width
        y_center_px = y_center * height
        box_width_px = box_width * width
        box_height_px = box_height * height
        
        # Calculate top-left corner
        x1 = x_center_px - box_width_px / 2
        y1 = y_center_px - box_height_px / 2
        
        # Plot image
        axes[idx].imshow(img)
        
        # Add bounding box
        rect = patches.Rectangle((x1, y1), box_width_px, box_height_px,
                               linewidth=2, edgecolor='red', facecolor='none')
        axes[idx].add_patch(rect)
        
        # Add ideal square box (using smaller dimension)
        ideal_size = min(box_width_px, box_height_px)
        ideal_x1 = x_center_px - ideal_size / 2
        ideal_y1 = y_center_px - ideal_size / 2
        
        square_rect = patches.Rectangle((ideal_x1, ideal_y1), ideal_size, ideal_size,
                                      linewidth=2, edgecolor='green', facecolor='none',
                                      linestyle='--')
        axes[idx].add_patch(square_rect)
        
        # Set title with box info
        axes[idx].set_title(f"{title}\nImage: {img_num}\n"
                           f"Box: {box_width:.3f} × {box_height:.3f}\n"
                           f"Ideal Square: {min(box_width, box_height):.3f} × {min(box_width, box_height):.3f}",
                           fontsize=10)
        axes[idx].axis('off')
    
    # Remove empty subplot
    if len(cases) < 6:
        axes[-1].axis('off')
    
    plt.tight_layout()
    plt.savefig('extreme_bounding_boxes.png', dpi=150, bbox_inches='tight')
    print("Visualization saved as 'extreme_bounding_boxes.png'")
    
    # Also create individual detailed views
    for img_num, title in cases[:2]:  # Just for smallest and largest
        fig2, ax2 = plt.subplots(1, 1, figsize=(8, 8))
        
        # Read image
        img_path = os.path.join(images_dir, f"golf_ball_{img_num}.jpg")
        img = cv2.imread(img_path)
        img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
        height, width = img.shape[:2]
        
        # Read label
        label_path = os.path.join(labels_dir, f"golf_ball_{img_num}.txt")
        with open(label_path, 'r') as f:
            line = f.readline().strip()
            
        parts = line.split()
        x_center = float(parts[1])
        y_center = float(parts[2])
        box_width = float(parts[3])
        box_height = float(parts[4])
        
        # Convert to pixels
        x_center_px = x_center * width
        y_center_px = y_center * height
        box_width_px = box_width * width
        box_height_px = box_height * height
        
        x1 = x_center_px - box_width_px / 2
        y1 = y_center_px - box_height_px / 2
        
        # Show image
        ax2.imshow(img)
        
        # Add bounding box
        rect = patches.Rectangle((x1, y1), box_width_px, box_height_px,
                               linewidth=3, edgecolor='red', facecolor='none')
        ax2.add_patch(rect)
        
        # Add center point
        ax2.plot(x_center_px, y_center_px, 'ro', markersize=8)
        
        ax2.set_title(f"{title} - golf_ball_{img_num}.jpg\n"
                     f"Normalized Box: {box_width:.4f} × {box_height:.4f}\n"
                     f"Pixel Box: {box_width_px:.1f} × {box_height_px:.1f}\n"
                     f"Area: {box_width * box_height:.6f}",
                     fontsize=12)
        ax2.axis('off')
        
        plt.tight_layout()
        plt.savefig(f'golf_ball_{img_num}_{title.replace(" ", "_").lower()}.png', 
                   dpi=150, bbox_inches='tight')
        plt.close()

if __name__ == "__main__":
    visualize_extreme_boxes()