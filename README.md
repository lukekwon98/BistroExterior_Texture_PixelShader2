# BistroExterior_Texture_PixelShader2

## OpenGL, GLSL

## Controls
- **Movement:** `s`, `x`, `z`, `c` for forward/backward/left/right  
- **Vertical Movement:** `Space`, `v` for up/down  
- **Zoom:** Ctrl + Scroll  
- **View Control:** Left click and drag to look left/right, right click and drag to look up/down  
- **Reset/View Modes:** `r` for top view, `t` for initial view  

## Features Implemented

### 1. Shading Models
- **Gouraud Shading (Press '1')**  
- **Phong Shading (Press '2')**  
  _Verified by toggling the shading model when facing Iron Man._

### 2. Textured Static/Dynamic Objects
- **Static:** Wooden table texture applied to a static wolf  
- **Dynamic:** Apartment complex scenery texture applied to a moving dragon  

### 3. World-coordinate Lighting
- A purple spotlight near the café, toggled using the '3' key  

### 4. Camera-coordinate Lighting
- A flashlight effect that follows the camera, toggled using the '4' key  

### 5. Object-fixed Lighting
- A blue spotlight follows a moving textured wolf, toggled using the '5' key  

### 6. Transparent Cube
- A rotating black cube that becomes transparent using the '6' key and adjustable transparency with `k` and `l` keys  

### 7. Additional Shader Effects
- Not implemented

## Assets
- **Textures:** `woodentable.jpg`, `danji.jpg` (stored in the Data folder)
