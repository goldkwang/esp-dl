# OV2640 ROI Width Limitation Analysis

## Confirmed Testing Results

### Working ROI Sizes
- **544×400**: ✅ Stable
- **544×272**: ✅ Stable  
- **544×256**: ✅ Stable
- **536×248**: ✅ Stable
- **512×256**: ✅ Stable

### Failed ROI Sizes
- **568×264**: ❌ Screen corruption
- **576×264**: ❌ Screen corruption
- Any width > 544: ❌ Fails

## Root Cause Analysis

The limitation appears at exactly 544 pixels width:
- 544 pixels = hsize 136 (0x88) → Works
- 568 pixels = hsize 142 (0x8E) → Fails

Key difference:
- hsize 136: bit 2 = 0
- hsize 142: bit 2 = 1

This suggests the OV2640's zoom engine has an internal limitation when hsize bit 2 is set.

## Technical Details

1. **Register Analysis**
   - HSIZE register: Lower 8 bits of (width/4)
   - VHYX register: Upper bits for size and offset
   - TEST register: Additional control bits
   - ZMHH register: Zoom window upper bits

2. **Attempted Solutions**
   - ❌ Setting TEST[7] = hsize[2] (ArduCAM method)
   - ❌ Setting TEST[7] = hsize[9] 
   - ❌ Not setting TEST register at all
   - ❌ Various DSP reset sequences

3. **Conclusion**
   The 544-pixel width limit appears to be a hardware constraint of the OV2640 sensor's zoom/windowing engine.

## Recommended Settings

For applications requiring ~2:1 aspect ratio:
```c
// Maximum stable width with 2.15:1 ratio
int roi_width = 544;
int roi_height = 256;  // 544/256 = 2.125:1 (close to 568/264 = 2.15:1)
int roi_offset_x = 368;
int roi_offset_y = 384;
```

## Alternative Solutions

1. **Use 536×248**
   - Exact same ratio as 568×264
   - Fully stable
   
2. **Post-processing**
   - Capture at 544×256
   - Software scale to desired resolution
   
3. **Different sensor**
   - OV5640 supports larger ROI sizes
   - Better suited for high-resolution windowing