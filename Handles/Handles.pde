/**
 * Handles. 
 * 
 * Click and drag the white boxes to change their position. 
 * Add custom shape.
 */
 
/* System variables set by size(w,h) function.
 * width = w    height = h
 */
 
PShape knob;
PFont f;

//Handle[] handles;
Slider[] handles;

void setup() {
  // w, h
  size(640, 460);
 
  // Slider parameters
  int num = 15;        // number of sliders to make for panel
  handles = new Slider[num];
  int px = 100;        // origin x of panel
  int py = 100;        // origin y of panel
  int sOffset = 30;    // spacing of sliders in x direction 
  for (int i = 0; i < handles.length; i++) {
    handles[i] = new Slider(px+(i*sOffset), py, handles);
  }
}

void draw() {
  background(153);
  
  for (int i = 0; i < handles.length; i++) {
    handles[i].update();
    handles[i].display();
  }
}

void mouseReleased()  {
  for (int i = 0; i < handles.length; i++) {
    handles[i].releaseEvent();
  }
}


// Defines a HIT box for the mouse pointer
boolean overRect(int x, int y, int bw, int bh) {
  if (mouseX >= x && mouseX <= x+bw && 
      mouseY >= y && mouseY <= y+bh) {
    return true;
  } else {
    return false;
  }
}

int lock(int val, int minv, int maxv) { 
  return  min(max(val, minv), maxv); 
} 
