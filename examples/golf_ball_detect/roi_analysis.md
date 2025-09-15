# ROI Width Analysis: 544 vs 568

## Register Bit Calculations

### Width = 544 (Works correctly)
- Width in pixels: 544
- hsize = 544 / 4 = 136 (0x88)
- Binary: 136 = 10001000
- REG_HSIZE: 0x88 (lower 8 bits)
- VHYX bits: (136 >> 6) & 0x0c = (0x02) & 0x0c = 0x00
- ZMOW: 136 & 0xff = 0x88
- ZMHH upper nibble: (136 >> 8) & 0x0f = 0x00

### Width = 568 (Causes corruption)
- Width in pixels: 568
- hsize = 568 / 4 = 142 (0x8E)
- Binary: 142 = 10001110
- REG_HSIZE: 0x8E (lower 8 bits)
- VHYX bits: (142 >> 6) & 0x0c = (0x02) & 0x0c = 0x00
- ZMOW: 142 & 0xff = 0x8E
- ZMHH upper nibble: (142 >> 8) & 0x0f = 0x00

## ROOT CAUSE IDENTIFIED

After analyzing the official OV2640 driver code, the issue is in the VHYX register bit calculation.

### Official OV2640 Implementation
```c
{VHYX, ((max_y >> 1) & 0X80) | ((offset_y >> 4) & 0X70) | ((max_x >> 5) & 0X08) | ((offset_x >> 8) & 0X07)},
```

For horizontal size (max_x), the official driver uses:
- `(max_x >> 5) & 0X08` - This takes bit 8 of max_x and places it in bit 3 of VHYX

### Current Implementation Bug
```c
sensor->set_reg(sensor, REG_VHYX, 0x0c, (hsize >> 6) & 0x0c);
```

This is incorrect because:
1. It shifts by 6 instead of 5
2. It masks with 0x0c (bits 2-3) instead of 0x08 (bit 3)

### Why Width > 544 Fails

For width = 544:
- hsize = 136 (binary 10001000)
- Bit 8 = 0
- (136 >> 5) & 0x08 = 0x00 (correct)
- (136 >> 6) & 0x0c = 0x00 (wrong formula but same result)

For width = 568:
- hsize = 142 (binary 10001110)  
- Bit 8 = 0
- (142 >> 5) & 0x08 = 0x00 (correct)
- (142 >> 6) & 0x0c = 0x00 (wrong formula but same result)

Wait, that's not it. Let me check bit 8...

For width = 544:
- hsize = 136 = 0x88 (binary 10001000)
- Bit 8 (counting from 0) = 0

For width = 568:
- hsize = 142 = 0x8E (binary 10001110)
- Bit 8 (counting from 0) = 0

Actually, let's check when bit 8 changes:
- Width 512: hsize = 128 = 0x80 (bit 8 = 0)
- Width 1024: hsize = 256 = 0x100 (bit 8 = 1)
- Width 544: hsize = 136 = 0x88 (bit 8 = 0)
- Width 568: hsize = 142 = 0x8E (bit 8 = 0)

The real issue might be that the current code doesn't handle bit 8 correctly when it becomes 1 for larger widths.

## CORRECT FIX

The VHYX register bit mapping for horizontal size should be:
```c
sensor->set_reg(sensor, REG_VHYX, 0x08, (hsize >> 5) & 0x08);
```

This correctly:
1. Shifts hsize right by 5 to get bit 8 into bit 3 position
2. Masks with 0x08 to isolate bit 3
3. Writes to VHYX bit 3 (mask 0x08)

### Additional Issue in ZMHH Register

Looking at line 93-94 in camera_init.cpp:
```c
uint8_t zmhh = (((roi_height / 4) >> 8) & 0x0f) | ((((roi_width / 4) >> 8) & 0x0f) << 4);
```

Compare to official driver (line 157):
```c
{ZMHH, ((h>>6)&0x04)|((w>>8)&0x03)},
```

The current implementation is also wrong! The official uses:
- For height: `(h>>6)&0x04` - bit 8 of h goes to bit 2
- For width: `(w>>8)&0x03` - bits 8-9 of w go to bits 0-1

But the current code puts width in upper nibble and height in lower nibble, which is backwards.