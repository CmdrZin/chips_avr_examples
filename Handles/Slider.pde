/**
 * Slider widget class based on Handles example code.
 */

class Slider {

  int x, y;
  int boxx, boxy;
  int stretch;
  int size;
  boolean over;
  boolean press;
  boolean locked = false;
  boolean otherslocked = false;
  Slider[] others;

  PShape ks;
  int ox, oy;
  int bw, bh;

  //  initial xpos, initial ypos, initial length,   size,    ref
  Slider(int ix, int iy, Slider[] o) {
    bw = 10;
    bh = 200;
    x = ix;
    y = iy;
    stretch = iy + (bh/2);
    size = 10;          // used in mouse-over detect.
    boxx = x;
    boxy = y;
    others = o;
    // Save original x,y
    ox = x;
    oy = y-5;    // half size h
    // set base dimentions

    // Create a custom knob
    knob = createShape();
    knob.beginShape();
    knob.fill(0, 0, 255);    // set fill color RGB
    knob.stroke(0);        // BLACK outline.

    // make a double trapazoid
    //          x, y
    knob.vertex(5, 0);
    knob.vertex(15, 0);
    knob.vertex(20, 5);
    knob.vertex(15, 10);
    knob.vertex(5, 10);
    knob.vertex(0, 5);
    knob.vertex(5, 0);
    knob.endShape(CLOSE);
    /*
  // make a right pointing triangle
     //          x, y
     knob.vertex(0,0);
     knob.vertex(10,5);
     knob.vertex(0,10);
     knob.vertex(0,0);
     knob.endShape(CLOSE);
     */

    ks = knob;
  }

  void update() {
    boxx = x-5;             // center of knob
    boxy = stretch-5;       // center of knob

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
      stretch = lock(mouseY, oy, oy+bh);
    }
  }

  void overEvent() {
    if (overRect(boxx, boxy, size*2, size)) {
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
    // Just at outline of base.
    noFill();
    stroke(0);
    // Draw base x,y,w,h at original position.
    rect(ox, oy, bw, bh);
    // Draw knob shape.
    shape(ks, boxx, boxy);
    // Draw text box
    fill(255);                  // text color
    text(String.valueOf(stretch-oy), ox-5, oy+bh+15);
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
