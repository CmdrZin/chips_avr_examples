/**
 * Slider widget class based on Handles example code.
 */

class Handle {
  
  int x, y;
  int boxx, boxy;
  int stretch;
  int size;
  boolean over;
  boolean press;
  boolean locked = false;
  boolean otherslocked = false;
  Handle[] others;
  
  PShape ks;
  
  //      xpos,   ypos, length,   size,    ref
  Handle(int ix, int iy, int il, int is, Handle[] o, PShape s) {
    x = ix;
    y = iy;
    stretch = il;
    size = is;
    boxx = x+stretch - size/2;
    boxy = y - size/2;
    others = o;
    ks = s;
  }
  
  void update() {
    boxx = x+stretch;
    boxy = y - size/2;
    
    for (int i=0; i<others.length; i++) {
      if (others[i].locked == true) {
        otherslocked = true;
        break;
      } else {
        otherslocked = false;
      }  
    }
    
    if (otherslocked == false) {
      overEvent();
      pressEvent();
    }
    
    if (press) {
      stretch = lock(mouseX-width/2-size/2, 0, width/2-size-1);
    }
  }
  
  void overEvent() {
    if (overRect(boxx, boxy, size, size)) {
      over = true;
    } else {
      over = false;
    }
  }
  
  void pressEvent() {
    if (over && mousePressed || locked) {
      press = true;
      locked = true;
    } else {
      press = false;
    }
  }
  
  void releaseEvent() {
    locked = false;
  }

  void display() {
    shape(ks, boxx, boxy);
  }
/*  
  void display() {
    // Set the fill color to WHITE
    fill(255);
    // Set line color to BLACK
    stroke(0);
    // Draw the box
    rect(boxx, boxy, size, size);
  }
 */
 
/* 
  void display() {
    // Draw a line from the base to the left side of the box.
    line(x, y, x+stretch, y);
    // Set the fill color to WHITE
    fill(255);
    // Set line color to BLACK
    stroke(0);
    // Draw the box
    rect(boxx, boxy, size, size);
    // Draw an X in the box
    if (over || press) {
      line(boxx, boxy, boxx+size, boxy+size);
      line(boxx, boxy+size, boxx+size, boxy);
    }
  }
 */

}
