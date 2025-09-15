# Bit Pattern Analysis: ROI Width 544 vs 568

## 1. Binary Calculations

### Width 544 (WORKS)
- Width in pixels: 544
- hsize = 544 / 4 = 136 = 0x88 = 0b10001000
- Bit breakdown:
  - Bit 8: 0
  - Bits 7-0: 0x88

### Width 568 (FAILS)
- Width in pixels: 568  
- hsize = 568 / 4 = 142 = 0x8E = 0b10001110
- Bit breakdown:
  - Bit 8: 0
  - Bits 7-0: 0x8E

## 2. Register Assignments Analysis

### HSIZE Register (0x51)
- Stores lower 8 bits of hsize
- Width 544: 0x88 ✓
- Width 568: 0x8E ✓

### VHYX Register (0x55) - FOUND ISSUE!

#### Current Implementation (INCORRECT):
```c
sensor->set_reg(sensor, REG_VHYX, 0x0c, (hsize >> 6) & 0x0c);
```

- Width 544: (136 >> 6) & 0x0c = 2 & 0x0c = 0x00
- Width 568: (142 >> 6) & 0x0c = 2 & 0x0c = 0x00

#### Official OV2640 Driver:
```c
(max_x >> 5) & 0x08  // For horizontal size bit 8
```

#### The Problem:
The current code uses:
1. Wrong shift: `>> 6` instead of `>> 5`
2. Wrong mask: `0x0c` (bits 2-3) instead of `0x08` (bit 3)

### ZMOW Register (0x5A)
- Stores lower 8 bits of width/4
- Width 544: 0x88 ✓
- Width 568: 0x8E ✓

### ZMHH Register (0x5C) - ANOTHER ISSUE!

#### Current Implementation (INCORRECT):
```c
uint8_t zmhh = (((roi_height / 4) >> 8) & 0x0f) | ((roi_width / 4) & 0xf0);
```

#### Official OV2640 Driver:
```c
{ZMHH, ((h>>6)&0x04)|((w>>8)&0x03)}
```

#### The Problems:
1. Width bits are placed incorrectly (upper nibble vs bits 0-1)
2. Height bit calculation is wrong

## 3. Why 544 Works But 568 Doesn't

The threshold of 544 pixels (hsize = 136) appears to be a hardware limitation in the OV2640 sensor when certain register bits are set incorrectly. 

### Binary Difference Analysis:
```
136 (0x88): 10001000
142 (0x8E): 10001110
Difference:  00000110 (bits 1,2,3 differ)
```

## 4. The Real Issue

After careful analysis, the problem is NOT with bit 8 handling (both 544 and 568 have bit 8 = 0).

The issue appears to be a hardware limitation where:
- **hsize > 136 causes image corruption**
- This translates to **width > 544 pixels**

This might be related to:
1. Internal buffer size limitations in the OV2640
2. Incorrect register configuration causing buffer overflow
3. Missing TEST register configuration (bit 7)

## 5. Potential Solutions

### Option 1: Fix Register Implementations
```c
// Fix VHYX for horizontal size
sensor->set_reg(sensor, REG_VHYX, 0x08, (hsize >> 5) & 0x08);

// Fix ZMHH
sensor->set_reg(sensor, REG_ZMHH, 0xff, ((vsize >> 6) & 0x04) | ((hsize >> 8) & 0x03));
```

### Option 2: Add TEST Register
```c
// Some implementations require TEST register bit 7 for large sizes
sensor->set_reg(sensor, REG_TEST, 0x80, (hsize >> 2) & 0x80);
```

### Option 3: Accept Hardware Limitation
Keep ROI width ≤ 544 pixels as a documented constraint for this sensor configuration.

## 6. Conclusion

The exact cause is likely a combination of:
1. Incorrect VHYX and ZMHH register implementations
2. Missing TEST register configuration
3. OV2640 hardware limitations with certain ROI configurations

The safest approach is to maintain the 544-pixel width limit until the register implementations can be thoroughly tested with the corrected formulas.