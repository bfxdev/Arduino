#include <Gamebuino-Meta.h>

// Gamebuino picture
#include "Picture.h"

void setup()
{
  // Starts Gamebuino library and sets target frame rate
  gb.begin();
  gb.setFrameRate(50);
}

// Increasing angle to set observer's position
float angle=0.0;

// Optimization level we are looking at
int part=0;

// Timing variables
unsigned long previousStartTime=millis();

// Defines a union type giving access to the data as 8, 16 or 32 bits integers
union FixedPoint
{
  int32_t  asInt32;
  int16_t  asInt16[2];
  uint16_t asUInt16[2];
  int8_t   asInt8[4];
  uint8_t  asUInt8[4];
};

void loop()
{ 
  /* 
  // Coordinates of the first point to draw
  float originX=10;
  float originY=10;

  // Column increment
  float columnIncrementX = 3.2;
  float columnIncrementY = 1.2;

  // Row increment
  float rowIncrementX = -1.0;
  float rowIncrementY =  2.1;
  */

  // Increments angle
  angle = angle + 2*3.1416/100;

  // Computes zoom level
  float zoom =  2.2 + 2*sin(angle/1.618);
  
  // Increments to go to next column (increment on x on drawn image)
  float columnIncrementX=zoom*2*cos(angle);
  float columnIncrementY=zoom*2*sin(angle);

  // Increments to go to next row (increment on y on drawn image)
  float rowIncrementX=zoom*3*cos(angle+3.1416/2);
  float rowIncrementY=zoom*3*sin(angle+3.1416/2);

  if(part==2)
  {
    // Increments to go to next column (increment on x on drawn image)
    columnIncrementX=4*cos(angle);
    columnIncrementY=4*sin(angle);
  
    // Increments to go to next row (increment on y on drawn image)
    rowIncrementX=4*cos(angle+3.1416/2);
    rowIncrementY=4*sin(angle+3.1416/2);
  }

  // Coordinates of the first point to draw
  float originX=128-40*columnIncrementX-32*rowIncrementX;
  float originY=128-40*columnIncrementY-32*rowIncrementY;
  
  if(part==1)
  {
    originX=10; originY=10; columnIncrementX = 3.2; columnIncrementY = 1.2;
    rowIncrementX = -1.0; rowIncrementY =  2.1;
  }

  // Waits for screen refresh
  while(!gb.update());

  // Gets the time at start of the computation
  unsigned long startTime = millis();

  switch(part)
  {

    case 0:
      gb.display.drawImage(0, 0, Picture);
      break;

    case 1: case 2: case 3:// No optimization
    {
      // Inits the counters and perform loop
      float startX=originX, startY=originY, X, Y;
      for(int py=0; py<64; py++)
      {
        X=startX;
        Y=startY;
        Color background = gb.createColor(0,0,py*4);
        for(int px=0; px<80; px++)
        {
          int intx = (int)X;
          int inty = (int)Y;
          if(intx<0 || intx>255 || inty<0 || inty>255)
            gb.display.drawPixel(px, py, background);
          else
            gb.display.drawPixel(px, py, Picture.getPixelColor(intx, inty));
    
          X+=columnIncrementX;
          Y+=columnIncrementY;
        }
        startX+=rowIncrementX;
        startY+=rowIncrementY;
      }
      break;
    }

    case 4: // getPixelColor optimized
    {
      // Inits the counters and perform loop
      float startX=originX, startY=originY, X, Y;
      for(int py=0; py<64; py++)
      {
        X=startX;
        Y=startY;
        Color background = gb.createColor(0,0,py*4);
        for(int px=0; px<80; px++)
        {
          int intx = (int)X;
          int inty = (int)Y;
          if(intx<0 || intx>255 || inty<0 || inty>255)
            gb.display.drawPixel(px, py, background);
          else
            gb.display.drawPixel(px, py, (Color)PictureData[6+intx+(inty<<8)]);
    
          X+=columnIncrementX;
          Y+=columnIncrementY;
        }
        startX+=rowIncrementX;
        startY+=rowIncrementY;
      }
      break;
    }

    case 5: // drawPixel optimized
    {
      // Inits the counters and perform loop
      float startX=originX, startY=originY, X, Y;
      int p=0;
      for(int py=0; py<64; py++)
      {
        X=startX;
        Y=startY;
        Color background = gb.createColor(0,0,py*4);
        for(int px=0; px<80; px++)
        {
          int intx = (int)X;
          int inty = (int)Y;
          if(intx<0 || intx>255 || inty<0 || inty>255)
            gb.display._buffer[p++] = (uint16_t)background;
          else
            gb.display._buffer[p++] = PictureData[6+intx+(inty<<8)];
    
          X+=columnIncrementX;
          Y+=columnIncrementY;
        }
        startX+=rowIncrementX;
        startY+=rowIncrementY;
      }
      break;
    }

    case 6: // Floats replaced by 32-bits integers
    {
      // Defines the increments as FixedPoint types and convert values from floats
      FixedPoint columnIncrementFPX, columnIncrementFPY, rowIncrementFPX, rowIncrementFPY;
      columnIncrementFPX.asInt32 = (int32_t)round(columnIncrementX*65536.0);
      columnIncrementFPY.asInt32 = (int32_t)round(columnIncrementY*65536.0);
      rowIncrementFPX.asInt32 = (int32_t)round(rowIncrementX*65536.0);
      rowIncrementFPY.asInt32 = (int32_t)round(rowIncrementY*65536.0);

      // Inits the counters as FixedPoint types and converts values from floats
      FixedPoint startX, startY, X, Y;
      startX.asInt32 = (int32_t)round(originX*65536.0);
      startY.asInt32 = (int32_t)round(originY*65536.0);

      // Performs loop
      int p=0;
      for(int py=0; py<64; py++)
      {
        X.asInt32 = startX.asInt32;
        Y.asInt32 = startY.asInt32;
        Color background = gb.createColor(0,0,py*4);
        for(int px=0; px<80; px++)
        {
          int intx = X.asInt16[1];
          int inty = Y.asInt16[1];
          if(intx<0 || intx>255 || inty<0 || inty>255)
            gb.display._buffer[p++] = (uint16_t)background;
          else
            gb.display._buffer[p++] = PictureData[6+intx+(inty<<8)];
    
          X.asInt32 += columnIncrementFPX.asInt32;
          Y.asInt32 += columnIncrementFPY.asInt32;
        }
        startX.asInt32 += rowIncrementFPX.asInt32;
        startY.asInt32 += rowIncrementFPY.asInt32;
      }
      break;
    }

    case 7: // 32-bits integers as bytes
    {
      // Defines the increments as FixedPoint types and convert values from floats
      FixedPoint columnIncrementFPX, columnIncrementFPY, rowIncrementFPX, rowIncrementFPY;
      columnIncrementFPX.asInt32 = (int32_t)round(columnIncrementX*65536.0);
      columnIncrementFPY.asInt32 = (int32_t)round(columnIncrementY*65536.0);
      rowIncrementFPX.asInt32 = (int32_t)round(rowIncrementX*65536.0);
      rowIncrementFPY.asInt32 = (int32_t)round(rowIncrementY*65536.0);

      // Inits the counters as FixedPoint types and converts values from floats
      FixedPoint startX, startY, X, Y;
      startX.asInt32 = (int32_t)round(originX*65536.0);
      startY.asInt32 = (int32_t)round(originY*65536.0);

      // Performs loop
      int p=0;
      for(int py=0; py<64; py++)
      {
        X.asInt32 = startX.asInt32;
        Y.asInt32 = startY.asInt32;
        Color background = gb.createColor(0,0,py*4);
        for(int px=0; px<80; px++)
        {
          if(X.asInt8[3]!=0 || Y.asInt8[3]!=0)
            gb.display._buffer[p++] = (uint16_t)background;
          else
            gb.display._buffer[p++] = PictureData[6+X.asUInt8[2]+(Y.asUInt8[2]<<8)];
    
          X.asInt32 += columnIncrementFPX.asInt32;
          Y.asInt32 += columnIncrementFPY.asInt32;
        }
        startX.asInt32 += rowIncrementFPX.asInt32;
        startY.asInt32 += rowIncrementFPY.asInt32;
      }
      break;
    }

    case 8: // Infinite picture
    {
      // Defines the increments as FixedPoint types and convert values from floats
      FixedPoint columnIncrementFPX, columnIncrementFPY, rowIncrementFPX, rowIncrementFPY;
      columnIncrementFPX.asInt32 = (int32_t)round(columnIncrementX*65536.0);
      columnIncrementFPY.asInt32 = (int32_t)round(columnIncrementY*65536.0);
      rowIncrementFPX.asInt32 = (int32_t)round(rowIncrementX*65536.0);
      rowIncrementFPY.asInt32 = (int32_t)round(rowIncrementY*65536.0);

      // Inits the counters as FixedPoint types and converts values from floats
      FixedPoint startX, startY, X, Y;
      startX.asInt32 = (int32_t)round(originX*65536.0);
      startY.asInt32 = (int32_t)round(originY*65536.0);

      // Performs loop
      int p=0;
      for(int py=0; py<64; py++)
      {
        X.asInt32 = startX.asInt32;
        Y.asInt32 = startY.asInt32;
        Color background = gb.createColor(0,0,py*4);
        for(int px=0; px<80; px++)
        {
          gb.display._buffer[p++] = PictureData[6+X.asUInt8[2]+(Y.asUInt8[2]<<8)];

          X.asInt32 += columnIncrementFPX.asInt32;
          Y.asInt32 += columnIncrementFPY.asInt32;
        }
        startX.asInt32 += rowIncrementFPX.asInt32;
        startY.asInt32 += rowIncrementFPY.asInt32;
      }
      break;
    }

    case 9: // 3D display
    {
      // Defines the counters
      FixedPoint X, Y, columnIncrementFPX, columnIncrementFPY;

      // Computes constant values for the draw
      float s=100, h=2+5*zoom, w=40;
      float a = angle/10;
      float Ox = 128+30*cos(a-3);
      float Oy = 128-30*sin(a-3);
      float sina = sin(a), cosa=cos(a);
      float Ax=h*(s*cosa-w*sina); 
      float Ay=h*(-s*sina-w*cosa);

      // Performs loop
      int p=0;
      for(int py=-20; py<(64-20); py++)
      {
        // Creates background color
        Color background = gb.createColor(0,0,(20+py)*4);

        // First lines are with background to avoid division by zero and moire effect
        if(py<1)
          for(int px=0; px<80; px++) gb.display._buffer[p++] = (uint16_t)background;
        else
        {
          originX = Ox + Ax/(float)py;
          originY = Oy + Ay/(float)py;
          columnIncrementX = sina*h/(float)py;
          columnIncrementY = cosa*h/(float)py;
  
          columnIncrementFPX.asInt32 = (int32_t)round(columnIncrementX*65536.0);
          columnIncrementFPY.asInt32 = (int32_t)round(columnIncrementY*65536.0);
          X.asInt32 = (int32_t)round(originX*65536.0);
          Y.asInt32 = (int32_t)round(originY*65536.0);
          
          for(int px=0; px<80; px++)
          {
            if(X.asInt8[3]!=0 || Y.asInt8[3]!=0)
              gb.display._buffer[p++] = (uint16_t)background;
            else
              gb.display._buffer[p++] = PictureData[6+X.asUInt8[2]+(Y.asUInt8[2]<<8)];
            X.asInt32 += columnIncrementFPX.asInt32;
            Y.asInt32 += columnIncrementFPY.asInt32;
          }
        }
      }
      
      break;
    }

    default: break;
  }

  // Computes real FPS
  int realFPS = (int)floor(1000.0/(startTime-previousStartTime));
  previousStartTime = startTime;

  // Computes and displays duration
  unsigned long duration = millis()-startTime;
  gb.display.clearTextVars();
  gb.display.setColor(WHITE, BLACK);
  gb.display.print(duration); gb.display.print(" ms");
  
  // Displays FPS and optimization level if B button is pressed
  if(gb.buttons.repeat(BUTTON_MENU, 0))
  {
    gb.display.print(" - ");
    gb.display.print(realFPS);  gb.display.println(" FPS");
    gb.display.println(part);
  }

  // Goes to next part if A button is pressed
  if(gb.buttons.pressed(BUTTON_A))
    part = (part+1) % 10;
  // Goes to previous part if B button is pressed
  if(gb.buttons.pressed(BUTTON_B))
    part = (part+9) % 10;
    
}
