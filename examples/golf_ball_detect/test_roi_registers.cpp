#include <stdio.h>
#include <stdint.h>

void print_register_values(int width, int height, int offset_x, int offset_y) {
    printf("\n=== ROI Configuration: %dx%d @ (%d, %d) ===\n", width, height, offset_x, offset_y);
    
    // Divide by 4 for register values
    uint16_t hoff = offset_x / 4;
    uint16_t voff = offset_y / 4;
    uint16_t hsize = width / 4;
    uint16_t vsize = height / 4;
    
    printf("After /4: hsize=%d (0x%03X), vsize=%d (0x%03X), hoff=%d (0x%03X), voff=%d (0x%03X)\n",
           hsize, hsize, vsize, vsize, hoff, hoff, voff, voff);
    
    // Individual registers
    printf("\nRegister values:\n");
    printf("HSIZE (0x51): 0x%02X\n", hsize & 0xff);
    printf("VSIZE (0x52): 0x%02X\n", vsize & 0xff);
    printf("XOFFL (0x53): 0x%02X\n", hoff & 0xff);
    printf("YOFFL (0x54): 0x%02X\n", voff & 0xff);
    
    // VHYX calculation - official formula
    uint8_t vhyx = ((vsize >> 1) & 0x80) | ((voff >> 4) & 0x70) | ((hsize >> 5) & 0x08) | ((hoff >> 8) & 0x07);
    printf("VHYX (0x55): 0x%02X = ", vhyx);
    printf("[vsize[8]=%d, voff[10:8]=%d, hsize[8]=%d, hoff[10:8]=%d]\n",
           (vsize >> 8) & 1, (voff >> 8) & 7, (hsize >> 8) & 1, (hoff >> 8) & 7);
    
    // TEST register
    uint8_t test = (hsize >> 2) & 0x80;
    printf("TEST (0x57): 0x%02X [hsize[9]=%d]\n", test, (hsize >> 9) & 1);
    
    // Zoom registers
    printf("ZMOW (0x5A): 0x%02X\n", hsize & 0xff);
    printf("ZMOH (0x5B): 0x%02X\n", vsize & 0xff);
    
    // ZMHH calculation
    uint8_t zmhh = ((vsize >> 6) & 0x04) | ((hsize >> 8) & 0x03);
    printf("ZMHH (0x5C): 0x%02X = [vsize[8]=%d, hsize[9:8]=%d]\n", 
           zmhh, (vsize >> 8) & 1, (hsize >> 8) & 3);
    
    // Binary representation
    printf("\nBinary breakdown:\n");
    printf("hsize = %d = 0b%09b (bit 8=%d, bit 9=%d)\n", 
           hsize, hsize, (hsize >> 8) & 1, (hsize >> 9) & 1);
    printf("vsize = %d = 0b%09b (bit 8=%d)\n", 
           vsize, vsize, (vsize >> 8) & 1);
}

int main() {
    printf("OV2640 ROI Register Analysis\n");
    printf("============================\n");
    
    // Test the working configuration
    print_register_values(544, 400, 368, 312);
    
    // Test the failing configuration
    print_register_values(568, 264, 352, 520);
    
    // Test boundary cases
    print_register_values(1024, 768, 0, 0);
    print_register_values(1280, 1024, 0, 0);
    
    return 0;
}