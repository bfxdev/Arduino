/*******************************************************************************
 *                                 Fractalino                                  *
 *                                 ----------                                  *
 *                                                                             * 
 *                                 2019 - bfx                                  *
 *                                                                             *
 * This program displays fractals defined as a function in the complex plane.  * 
 * Currently, only the Mandelbrot set with simple coloring is supported.       * 
 * Internal computation is performed either with 32-bits fixed point numbers   *
 * (for maximum performance) or with 64 bits fixed point numbers, with an      * 
 * automatic switch of precision depending on the zoom level.                  * 
 * The interactive viewer allows to pan and zoom in the plane using the        *
 * gamepad and the buttons. It uses the gb.tft directly for high resolution    *
 * pictures. It is designed primarily for the Gamebuino but could be ported to *
 * other Arduino hardware with color display.                                  *
 *                                                                             *
 * The program has noticeable features:                                        *
 *  - Largely object-oriented design, using e.g. C++ operators overloading     *
 *  - Multi-precision fixed-point computation                                  *
 *  - Direct use of the Gamebuino TFT screen in high-resolution                *
 *                                                                             *
 * Changelog:                                                                  *
 *  2019-01 - 0.1 : Initial release                                            *
 *  2019-01 - 0.2 : Replaced double type by FP64 (long long int) in Complex    *
 *  2019-01 - 0.3 : Re-factoring and introduction of Screen and Viewer objects *
 *  2019-01 - 1.0 : Deep re-design, new template-based Fixed Point object,     *
 *                  adaptive render resolution for constant fps, new fractal *
 *                  formulae, cross/circle orbit traps, new buttons mapping     *
 *                                                                             *
 0       1         2         3         4         5         6         7         8 
 2345678901234567890123456789012345678901234567890123456789012345678901234567890
 */

const char FractalinoVersion[] = "1.0";

#include <Gamebuino-Meta.h>

// The color gradients are given by a single picture 256x41
#include "Gradients.h"

class PictureMap
{
  public:
    bool next()
      { return true; }
} pictureMap;


class ColorMap
{
  private:
    Image gradients = Image(gradientsData);
    int num;
    bool reverse=false;
  public:
    ColorMap(int num): num(num) {}
    Color get(int level, int maximum)
    {
      if(reverse)
        return gradients.getPixelColor((maximum-level)*255/maximum, num);
      else
        return gradients.getPixelColor(level*255/maximum, num);
    }

    bool next()
    {
      bool res = false;
      if(reverse)
      {
        reverse = false;
        num = (num+1) % 41;
        res = (num==0);
      }
      else
        reverse = true;

      print();
  
      return res;
    }

    void print()
    {
      SerialUSB.print("this="); SerialUSB.println((int)this);
      SerialUSB.print("num="); SerialUSB.println(num);
      SerialUSB.print("reverse="); SerialUSB.println(reverse);
    }  
        
} colorMap1(3), colorMap2(7);

template <typename T, int bits> class FixedPointNumber
{
  private:
    T data;

  public:
    // Default constructor without parameters, do nothing (assumed data=0)
    explicit FixedPointNumber() {}
    // Constructor used in operators 
    explicit FixedPointNumber(T t): data(t) {}
    // Constructor from double
    //explicit FixedPointNumber(double d) {set(d);}
    // Assignment/cast operators
    void operator=(double d) {set(d);}
    void operator=(int i) {set(i);}
    operator double() {return asDouble();}
    operator int() {return asInteger();}

    // Set/get functions
    void set(int i)    { data = (T)(((T)i)<<(sizeof(T)*8-bits)); }
    void set(double d) { data = (T)(d*(((T)(1LL))<<(sizeof(T)*8-bits))); }
    T raw()            { return data; }
    int asInteger()    { return (int)(data>>(sizeof(T)*8-bits)); }
    double asDouble()  { return ((double)data)/(((T)(1LL))<<(sizeof(T)*8-bits));}

    // Addition operators overloading
    friend FixedPointNumber operator+(FixedPointNumber fpn1, FixedPointNumber fpn2)
      {return FixedPointNumber( fpn1.data + fpn2.data );}
    friend FixedPointNumber operator+(FixedPointNumber fpn, int i)
      {return FixedPointNumber( fpn.data + (((T)i)<<(sizeof(T)*8-bits)) );}
    friend FixedPointNumber operator+(int i, FixedPointNumber fpn)
      {return fpn + i;}
    friend FixedPointNumber operator+(FixedPointNumber fpn, double d)
      {return FixedPointNumber( fpn + FixedPointNumber(d) );}
    friend FixedPointNumber operator+(double d, FixedPointNumber fpn)
      {return fpn + d;}

    // Subtraction operators overloading
    friend FixedPointNumber operator-(FixedPointNumber fpn)
      { return FixedPointNumber(-fpn.data);}
    friend FixedPointNumber operator-(FixedPointNumber fpn1, FixedPointNumber fpn2)
      {return fpn1 + (-fpn2);}
    friend FixedPointNumber operator-(FixedPointNumber fpn, int i)
      {return fpn + (-i);}
    friend FixedPointNumber operator-(int i, FixedPointNumber fpn)
      {return i + (-fpn);}
    friend FixedPointNumber operator-(FixedPointNumber fpn1, double d)
      {return fpn1 + (-d);}
    friend FixedPointNumber operator-(double d, FixedPointNumber fpn)
      {return d + (-fpn);}
    
    // Multiplication operators overloading
    friend FixedPointNumber operator*(FixedPointNumber fpn1, FixedPointNumber fpn2)
      {return FixedPointNumber( (fpn1.data>>((sizeof(T)*8-bits)/2)) * (fpn2.data>>((sizeof(T)*8-bits)/2)) );}
    friend FixedPointNumber operator*(FixedPointNumber fpn, int i)
      {return FixedPointNumber( (T)fpn.data * (T)i );}
    friend FixedPointNumber operator*(int i, FixedPointNumber fpn)
      {return fpn * i;}
    friend FixedPointNumber operator*(FixedPointNumber fpn, double d)
      {return FixedPointNumber( fpn * FixedPointNumber(d) );}
    friend FixedPointNumber operator*(double d, FixedPointNumber fpn)
      {return fpn * d;}

    // Division operators overloading (only through integer)
    friend FixedPointNumber operator/(FixedPointNumber fpn, int i)
      {return FixedPointNumber( fpn / (T)i );}
};

void testFixedPointNumber()
{
  FixedPointNumber<int16_t, 6> a16, b16, c16, d16;

  a16.set(-1.1);
  b16.set(2.2);
  c16.set(-1.5);

  d16 = a16+b16+c16;
  SerialUSB.print("a16.set(-1.1); a16.raw()="); SerialUSB.println(a16.raw());
  SerialUSB.print("a16.asInteger()="); SerialUSB.println(a16.asInteger());
  SerialUSB.print("a16.asDouble()="); SerialUSB.println(a16.asDouble());
  SerialUSB.print("1.1+2.2-1.5="); SerialUSB.println(d16.asDouble());
}


// Wrapper for screen functions
class Screen
{
  public:
    int width, height, frameRate;
    Screen(): width(160), height(128), frameRate(10) {}

    // Initializes screen (to be called once in setup()
    void init()
    {
      gb.display.init(0, 0, ColorMode::rgb565);
      gb.setFrameRate(frameRate);
      gb.tft.clear();
    }

    void drawSquare(int x, int y, int size, Color color)
    {
      gb.tft.setColor(color);
      
      // Draws pixel or rectangle
      if(size<=1) gb.tft.drawPixel(x, y);
      else        gb.tft.fillRect(x, y, size, size);
    }
} screen;

// Virtual plane function to define interface, not precision-dependent
// Computation of position in the complex plane is defined as well in this class
// It is a virtual objects, i.e. methods must be implemented in child classes
// The PlaneFunctionViewer object relies only on the functions of this class
class PlaneFunction
{
  protected:
    // Iteration limit set at refresh
    int limit;
    // Rendering indices
    int rendering1, rendering2;
    // Color provider
    ColorMap& colorMap1;
    ColorMap& colorMap2;
    // Picture provider
    PictureMap& pictureMap;
    // True for dual (Julia)
    bool dual;
    
  public:
    // Constructor setting references to static objects and default values
    PlaneFunction(ColorMap& colorMap1, ColorMap& colorMap2, PictureMap& pictureMap):
      colorMap1(colorMap1), colorMap2(colorMap2), pictureMap(pictureMap),
      limit(255), rendering1(0), rendering2(0), dual(false) {}

    // Init function to be called if the precision or a parameter changes (before refresh)
    // dualcx and dualcyy are not stored here, but in child class implementation
    virtual void init(int limit, int rendering1=0, int rendering2=0, bool dual=false, double dualcx=0, double dualcy=0)
    { this->limit=limit; this->rendering1=rendering1; this->rendering2=rendering2; this->dual=dual; }

    // Returns 0 for 16 bits precision, 1 for 32, 2 for 64 given a zoom value
    virtual int getRequiredPrecision(double zoom, bool dual)
      { return zoom < 120 ? 0 : (zoom < 20000 ? 1 : 2); }

    virtual void getPreset(int preset, bool dual, double& centerx, double& centery, double& zoom)
    { centerx=0; centery=0; zoom=20; }
    
    // Starts a pass, i.e. inits internal counters (to be defined for given precision)
    virtual void initPass(double startx, double starty, double step){};

    // Jumps to next point (to be defined for given precision)
    virtual void nextPoint(){};
    
    // Jumps to next line (to be defined for given precision)
    virtual void nextLine(){};

    // Gets the color at the current point for the current parameters
    virtual Color getColor(){};

};

// Basis class for other functions, here with classical Mandelbrot formula: C_n+1 = (C_n)^2 + p
// A "dual" setting is defined, that draws the Julia set
template <typename T> class MandelbrotPlaneFunction : public PlaneFunction
{
  protected:
    // Internal counters with type T
    T startx, starty, step, px, py;
    // Reference center for dual at given precision
    T dualcx, dualcy;

  public:

    // Constructor setting references to static objects and default values
    MandelbrotPlaneFunction(): PlaneFunction() {}
    MandelbrotPlaneFunction(ColorMap& colorMap1, ColorMap& colorMap2, PictureMap& pictureMap):
      PlaneFunction(colorMap1, colorMap2, pictureMap) {}

    void init(int limit, int rendering1=0, int rendering2=0, bool dual=false, double dualcx=0, double dualcy=0) override
    {
      //this->limit=limit; this->rendering1=rendering1; this->rendering2=rendering2;
      PlaneFunction::init(limit, rendering1, rendering2, dual);
      this->dualcx.set(dualcx); this->dualcy.set(dualcy);
      /*if(SerialUSB)
      {
        SerialUSB.println("\n-------------------------------------------------");
        SerialUSB.println(rendering1);
        SerialUSB.println(rendering2);
      }*/
    }

    void initPass(double startx, double starty, double step)
    {
      this->startx.set(startx); this->starty.set(starty); this->step.set(step);
      px = this->startx; py = this->starty;
    }

    void nextPoint() { px = px + step; }

    void nextLine()  { py = py - step; px = startx; }

    // Main function to draw the normal set, the dual set and the different renderings
    Color getColor()
    {
      // Used for computation
      T x(px), y(py), newx, newy, l;
      
      // Used for orbit traps: minimums of |x|, |y| and squared length 
      T minx, miny, minl, ax, ay;
      minx.set((double)0.1); miny.set((double)0.1); minl.set((double)0.1);

      int i;
      for(i=0; i<limit; i++)
      {
        // Calls formula
        formula(x, y, newx, newy);

        // Stops loop if the current point is out of the disc of radius 2
        // Computes squared length of the norm of the current point (avoiding sqrt)
        l = newx*newx + newy*newy;
        if( l.asInteger() > 4 )
          break;
        // Otherwise continues computation and update mins
        else
        {
          // Stores mins for orbit traps
          if(rendering1!=0 || rendering2!=0)
          {
            // Computes absolute values
            ax = (newx.raw() >= 0) ? newx : -newx;
            ay = (newy.raw() >= 0) ? newy : -newy;

            // Stores mins of absolute values
            if (ax.raw() < minx.raw()) minx = ax;
            if (ay.raw() < miny.raw()) miny = ay;

            // Stores min for circle orbit trap
            if(l.raw() < minl.raw())
              minl = l;
          }
            
          // Prepares for next step
          x=newx; y=newy;
        }
      }

      // Chooses rendering1 if point diverged, rendering2 if inside set
      int rendering = (i<limit) ? rendering1 : rendering2;
      if(rendering>0)
      {
        double m, width=(310.0-(double)limit)/5000.0, radius=width*10.0;
        int r;
        
        // Cross Orbit Trap on X
        if((r=rendering&3)>0 && (m=minx.asDouble())<width)
          switch(r)
          {
            case 1: return colorMap1.get((int)( 256-m*60.0/width ), 256);
            case 2: return colorMap2.get((int)( m*60.0/width ), 256);
            case 3: return colorMap2.get((int)( 256-m*60.0/width ), 256);
            default: break;
          }
        rendering = rendering >> 2;

        // Cross Orbit Trap on X
        if((r=rendering&3)>0 && (m=miny.asDouble())<width)
          switch(r)
          {
            case 1: return colorMap1.get((int)( 256-m*60.0/width ), 256);
            case 2: return colorMap2.get((int)( m*60.0/width ), 256);
            case 3: return colorMap2.get((int)( 256-m*60.0/width ), 256);
            default: break;
          }
        rendering = rendering >> 2;

        // Circle Orbit Trap only for inside
        if(i==limit)
        {
          if((r=rendering&3)>0 && (m=sqrt(minl.asDouble()))<radius)
            switch(r)
            {
              case 1: return colorMap1.get((int)( 256-m*256.0/radius ), 256);
              case 2: return colorMap2.get((int)( m*60.0/radius ), 256);
              case 3: return colorMap2.get((int)( 256-m*60.0/radius ), 256);
              default: break;
            }
          //rendering = rendering >> 2;
        }

      }
      // Default case: no rendering, then returns color 2 if point is in set otherwise escape color
      return (i<limit) ? colorMap1.get(i, limit) : colorMap2.get(0, limit);
    }

    void virtual formula(T x, T y, T& newx, T& newy)
    {
      // Mandelbrot computation
      // c_n+1 = c_n * c_n + px + i*py
      // c_n+1 = (x+i*y)*(x+i*y) + px + i*py
      // c_n+1 = x*x - y*y + px + i*( 2*x*y + py )
      newx = x*x - y*y + (dual?dualcx:px);
      newy = ((int)2)*x*y + (dual?dualcy:py);
      
    }

    void getPreset(int preset, bool dual, double& centerx, double& centery, double& zoom) override
    {
      if(dual)
        { centerx = 0; centery = 0; zoom = 40; }
      else switch(preset%4)
      {
        case 0: centerx = -0.7; centery = 0; zoom = 50; break;
        case 1: centerx = -0.170337; centery = -1.06506; zoom = 50000; break;
        case 2: centerx = 0.42884; centery = -0.231345; zoom = 50000; break;
        case 3: centerx = -1.62917; centery = -0.0203968; zoom = 50000; break;
      }
    }

};

MandelbrotPlaneFunction<FixedPointNumber<int16_t, 6>> mandelbrot16(colorMap1, colorMap2, pictureMap);
MandelbrotPlaneFunction<FixedPointNumber<int32_t, 6>> mandelbrot32(colorMap1, colorMap2, pictureMap);
MandelbrotPlaneFunction<FixedPointNumber<int64_t, 6>> mandelbrot64(colorMap1, colorMap2, pictureMap);

// Fractal formula: C_n+1 = (C_n)^3 + p
template <typename T> class MandelbrotCubePlaneFunction : public MandelbrotPlaneFunction<T>
{
  public:
    // Constructor setting references to static objects and default values
    MandelbrotCubePlaneFunction(): MandelbrotPlaneFunction<T>() {}
    MandelbrotCubePlaneFunction(ColorMap& colorMap1, ColorMap& colorMap2, PictureMap& pictureMap):
    MandelbrotPlaneFunction<T>(colorMap1, colorMap2, pictureMap) {}

    void formula(T x, T y, T& newx, T& newy) override
    {
      // Mandelbrot cube computation
      // c_n+1 = c_n * c_n * c_n + px + i*py
      // c_n+1 = (x+i*y)*(x+i*y)*(x+i*y) + px + i*py
      // c_n+1 = (x+i*y)*(x*x - y*y + i*2*x*y) + px + i*py
      // c_n+1 = x*(x*x-y*y) - 2*x*y*y + px + i*( y*(x*x-y*y) + 2*x*x*y + py )
      // Necessary to use 'this->' to access parent-parent class members, unclear why
      newx = x*(x*x-y*y) - ((int)2)*x*y*y + (this->dual?this->dualcx:this->px);
      newy = y*(x*x-y*y) + ((int)2)*x*x*y + (this->dual?this->dualcy:this->py);
    }

    void getPreset(int preset, bool dual, double& centerx, double& centery, double& zoom) override
    {
      if(dual)
      { centerx = 0; centery = 0; zoom = 40; }
      else switch(preset%4)
      {
        case 0: centerx = -0.5; centery = 0; zoom = 40; break;
        case 1: centerx = -0.26283550; centery = 1.08394748; zoom = 12000; break;
        case 2: centerx = -0.19123354; centery = 1.15631155; zoom = 22000; break;
        case 3: centerx = -0.06659353; centery = 1.19763646; zoom = 5230; break;
      }
    }

  int getRequiredPrecision(double zoom, bool dual) override
    { return dual ? 2 : (zoom < 10000 ? 1 : 2); }

};

MandelbrotCubePlaneFunction<FixedPointNumber<int16_t, 8>> mandelbrotCube16(colorMap1, colorMap2, pictureMap);
MandelbrotCubePlaneFunction<FixedPointNumber<int32_t, 8>> mandelbrotCube32(colorMap1, colorMap2, pictureMap);
MandelbrotCubePlaneFunction<FixedPointNumber<int64_t, 8>> mandelbrotCube64(colorMap1, colorMap2, pictureMap);

// Used in the Polynomial class, but needs to be defined here (static members in templates will not be found by linker)
double coeffs[4] = {1,0,0,0};

// Formula somehow based on C_n+1 = a_4*(C_n)^4 + a_3*(C_n)^3 + a_2*(C_n)^2 + a_1*(C_n) + p
template <typename T> class PolynomialPlaneFunction : public MandelbrotPlaneFunction<T>
{
  private:
    T tcoeffs[4];
  public:
    // Constructor setting references to static objects and default values
    PolynomialPlaneFunction(): MandelbrotPlaneFunction<T>() {}
    PolynomialPlaneFunction(ColorMap& colorMap1, ColorMap& colorMap2, PictureMap& pictureMap):
    MandelbrotPlaneFunction<T>(colorMap1, colorMap2, pictureMap) {}

    void init(int limit, int rendering1=0, int rendering2=0, bool dual=false, double dualcx=0, double dualcy=0) override 
    {
      MandelbrotPlaneFunction<T>::init(limit, rendering1, rendering2, dual, dualcx, dualcy);
      for(int i=0; i<4; i++)
        tcoeffs[i] = coeffs[i];
    }

    void formula(T x, T y, T& newx, T& newy) override
    {
      T tx(x), ty(y), t;
      // Computes a complex polynomial with real coefficients

      // Power 0
      newx = (this->dual?this->dualcx:this->px);
      newy = (this->dual?this->dualcy:this->py);

      // Power 1
      newx = newx + ty;
      newy = newy + tx;

      // Power 2
      t = tx*x - ty*y; ty = tx*y + ty*x; tx = t;
      newx = newx + tcoeffs[2]*tx;
      newx = newx + tcoeffs[2]*ty;

      // Power 3
      t = tx*x - ty*y; ty = tx*y + ty*x; tx = t;
      newx = newx + tcoeffs[1]*ty;
      newx = newx + tcoeffs[1]*tx;

      // Power 4
      t = tx*x - ty*y; ty = tx*y + ty*x; tx = t;
      newx = newx + tx;
      newx = newx + ty;
    }

    void getPreset(int preset, bool dual, double& centerx, double& centery, double& zoom) override
    {
      // Initializes function coefficients
      randomSeed(preset);
      if(!dual)
        for(int i; i<4; i++)
          coeffs[i] = (double)((((double)random(0,10000))/5000.0-1.0)*2.0);
        
      centerx = 0; centery = 0; zoom = 40;
    }

    int getRequiredPrecision(double zoom, bool dual) override
    { return dual ? 2 : (zoom < 10000 ? 1 : 2); }


};

PolynomialPlaneFunction<FixedPointNumber<int16_t, 10>> polynom16(colorMap1, colorMap2, pictureMap);
PolynomialPlaneFunction<FixedPointNumber<int32_t, 10>> polynom32(colorMap1, colorMap2, pictureMap);
PolynomialPlaneFunction<FixedPointNumber<int64_t, 10>> polynom64(colorMap1, colorMap2, pictureMap);

#define NUM_FUNCTIONS 3
PlaneFunction* functions[] = {&mandelbrot16, &mandelbrot32, &mandelbrot64,
                              &mandelbrotCube16, &mandelbrotCube32, &mandelbrotCube64,
                              &polynom16, &polynom32, &polynom64};

class PlaneFunctionViewer
{
  private:
    // Screen to draw squares
    Screen& screen;
    // The fractal functions array, and current function
    PlaneFunction** functions;
    PlaneFunction* function;

    // Boundaries, current position and step of the current pass in pixels
    int startx, endx, starty, endy, x, y, step;

    // Coordinates of the current view in complex plane coordinates
    double centerx, centery, zoom;

    // Current limit exponent, limit=2^(limitexp+1)-1
    int limit;

    // Initial step (adapted for performance)
    int initstep;

    // Count/current function in array
    int funcnum, funcindex = 0;

    // Dual parameters
    bool dual;
    double dualcx, dualcy;

    // References to provider objects (as well referred to in PlaneFunction) and rendering
    ColorMap& colorMap1;
    ColorMap& colorMap2;
    PictureMap& pictureMap;
    int rendering1, rendering2, preset;
  
  public:

    PlaneFunctionViewer(Screen &screen, ColorMap& colorMap1, ColorMap& colorMap2, PictureMap& pictureMap,
                        int funcnum, PlaneFunction** functions):
      screen(screen), colorMap1(colorMap1), colorMap2(colorMap2), pictureMap(pictureMap), startx(0), starty(0),
      endx(screen.width), endy(screen.height), funcnum(funcnum), functions(functions), dual(false),
      rendering1(0), rendering2(0), preset(0) { reset(); }

    // Resets the view and re-inits everything
    void reset()
    {
      functions[funcindex*3]->getPreset(preset, dual, centerx, centery, zoom);
      limit = 61; initstep=16;
      init();
    }

    // Inits the computation, to be called each time rendering/limit parameters are changed
    void init()
      {
        int precision = functions[funcindex*3]->getRequiredPrecision(zoom, dual);
        function = functions[funcindex*3+precision];
        function->init(limit, rendering1, rendering2, dual, dualcx, dualcy);
        refresh();
      }

    // Setups the next draw pass to start at given new step (-1 to re-init function and use default step)
    void refresh(int newstep=-1)
    {
      // Takes new step of given value, starts from initial step if nothing was given
      step = newstep==-1 ? step = initstep : newstep;

      // Sets pixel position to top-left
      x=startx; y=starty;

      // Sets increments to jump to next block
      double cstep = (double)step/zoom;

      // Sets start of current line to middle of top-left block
      double cstartx = centerx - (double)(screen.width)/2.0/zoom  + cstep/2.0;
      double cstarty = centery + (double)(screen.height)/2.0/zoom - cstep/2.0;

      // Inits internal counters for next pass
      function->initPass(cstartx, cstarty, cstep);
    }

    // Draws the given number of blocks, updates all internal variables accordingly
    // tries to fill screen in first pass if no num is given
    // Returns true if the draw was started with step equal to initial step
    bool draw(int num=-1)
    {
      // Determines returned value
      bool res = step==initstep;
      
      // Default number of drawn blocks to refresh 1/4 screen at initial step 
      if(num==-1)
        num = (screen.width/initstep+1)*(screen.height/initstep+1)/4;
      
      for(int i=0; i<num && step>=1; i++)
      {
        // Draws pixel/square with color given by the main computation function
        screen.drawSquare(x, y, step, function->getColor());

        // Update coordinates to the next pixel/square and checks end of horizontal line
        x += step;
        if(x<=endx)
          function->nextPoint();
        else
        {
          // Goes to next line and checks end of pass
          x = startx;
          y += step;
          if(y<=endy)
            function->nextLine();
          else
          {
            // Starts next pass at half step
            refresh(step/2);
            
            // Breaks here to synchronize with adaptive mechanism 
            break;
          } 
        }
      }

     return res; 
    }

    // Moves observer by given number of pixels
    void move(int dx, int dy)
    { centerx+= (double)dx/zoom; centery+= (double)dy/zoom; refresh();}

    // Magnifies (i.e. increase zoom) by given factor
    void magnify(double factor)
      { zoom *= factor; init();}

    // Changes inital step values
    void increaseInitialStep()
      { initstep = min(40, initstep + 1); }
    void decreaseInitialStep()
      { initstep = max(1,  initstep - 1); }

    // Changes limit value
    void nextLimit()
      { limit = 1 + 20*((limit/20 + 1)%10); init(); }

    // Goes to next fractal, returns true if next cycle
    bool nextFractal()
    {
      funcindex = (funcindex + 1) % funcnum;
      reset();
      return funcindex==0;
    }

    // Goes to next preset
    bool nextPreset()
    {
      preset++;
      reset();
      return false;
    }

    // Toggle between Dual (e.g. Julia) and Normal (e.g. Mandelbrot) sets
    bool toggleDual()
    {
      dual = !dual;
      dualcx = centerx;
      dualcy = centery; 
      reset();
      return !dual;
    }

    // Goes to next color and rendering
    bool nextColor1()
      { bool res = colorMap1.next(); refresh(); return res; }
    bool nextColor2()
      { bool res = colorMap2.next(); refresh(); return res; }
    bool nextRendering1()
      { bool res = 0 == (rendering1 = ((rendering1+1) % 16)); init(); return res; }
    bool nextRendering2()
      { bool res = 0 == (rendering2 = ((rendering2+1) % 64)); init(); return res; }

    void print()
    {
      SerialUSB.printf("\ncenterx*1000000=");  SerialUSB.println(centerx*1000000);
      SerialUSB.printf("centery*1000000=");  SerialUSB.println(centery*1000000);
      SerialUSB.printf("zoom=");     SerialUSB.println(zoom);
      SerialUSB.print("limit="); SerialUSB.println(limit);
      SerialUSB.print("step="); SerialUSB.println(step);
      SerialUSB.print("initstep="); SerialUSB.println(initstep);
      SerialUSB.print("dual="); SerialUSB.println(dual);
      SerialUSB.print("dualcy*1000000="); SerialUSB.println(dualcy*1000000);
      SerialUSB.print("dualcy*1000000="); SerialUSB.println(dualcy*1000000);
      SerialUSB.print("rendering1="); SerialUSB.println(rendering1);
      SerialUSB.print("rendering2="); SerialUSB.println(rendering2);
      
    }  
} viewer(screen, colorMap1, colorMap2, pictureMap, NUM_FUNCTIONS, &functions[0]);


// Includes a couple of fonts for the splash screen
extern const byte font3x3[]; //a really tiny font
extern const byte font3x5[]; //a small but efficient font (default)
extern const byte font5x7[]; //a large, comfy font


void setup()
{
  // Starts Gamebuino library but disables standard gb.display object (not used)
  gb.begin();
  screen.init();
  gb.tft.clearTextVars();

  // Starts SerialUSB for debugging
  SerialUSB.begin(9600);

  // Displays welcome message
  // TODO: re-factor to center automatically the text
  // TODO: add splash effect
  gb.tft.setFont(font5x7); gb.tft.setFontSize(2);
  gb.tft.setCursor(23, 0); gb.tft.print("Fractalino");

  gb.tft.setFont(font3x5); gb.tft.setFontSize(1);
  gb.tft.setCursor(35, 20); gb.tft.print("Version ");
  gb.tft.print(FractalinoVersion); gb.tft.print(" - 2019 - bfx");

  gb.tft.setFont(font5x7); gb.tft.setFontSize(1);
  gb.tft.setCursor(0,45);  gb.tft.print("Pad/A/B : Move and zoom");
  gb.tft.setCursor(0,55);  gb.tft.print("Menu+A  : Color 1");
  gb.tft.setCursor(0,65);  gb.tft.print("Menu+B  : Fractal/preset");
  gb.tft.setCursor(0,75);  gb.tft.print("Menu+L  : Color 2");
  gb.tft.setCursor(0,85);  gb.tft.print("Menu+R  : Render inside");
  gb.tft.setCursor(0,95);  gb.tft.print("Menu+U  : Limit/peaks");
  gb.tft.setCursor(0,105); gb.tft.print("Menu+D  : Render outside");
  
  gb.tft.setCursor(33,120); gb.tft.print("Press A to start");

  // Wait for key press
  while(true)
  {
    while(!gb.update());
    if(gb.buttons.pressed(BUTTON_A)) break;
  }

  // testFixedPointNumber();
}


void loop()
{
  // Tries to draw several times until update is true
  // Determines the number of draws and if it includes a draw at initial step 
  int count=0;
  bool isInitialStep=false;
  while(!gb.update())
  {
    bool res = viewer.draw();
    isInitialStep = isInitialStep || res;
    count++;
  }

  // The pass at initial step is the first pass
  if(isInitialStep)
  {
    // The draw pass is tweaked such that 4 full draws are possible in one frame
    // Adapt initial step to reach 4 draws or more when we are at initial step
    if(count>=4) viewer.decreaseInitialStep();
    else        viewer.increaseInitialStep();

    if(true)
    {
      // Prints debug info
      SerialUSB.println("\nViewer:");
      viewer.print();
      //SerialUSB.println("\nColor map 1:");
      //colorMap1.print();
      //SerialUSB.println("\nColor map 2:");
      //colorMap2.print();
    }
  }
  
  // Checks buttons with menu
  if(gb.buttons.pressed(BUTTON_MENU) || gb.buttons.repeat(BUTTON_MENU, 0))
  {
    // Menu+A: change color 1
    if(gb.buttons.pressed(BUTTON_A))
      viewer.nextColor1();

    // Menu+B: change fractal/preset
    else if(gb.buttons.pressed(BUTTON_B))
    {
      if(viewer.toggleDual())
        if(viewer.nextFractal())
          viewer.nextPreset();
    }

    // Menu+Left: change color 2
    else if(gb.buttons.pressed(BUTTON_LEFT))
      viewer.nextColor2();

    // Menu+Right: change rendering inside
    else if(gb.buttons.pressed(BUTTON_RIGHT))
      viewer.nextRendering2();

    // Menu+Down: change rendering outside
    else if(gb.buttons.pressed(BUTTON_DOWN))
      viewer.nextRendering1();

    // Menu+Up: change limit
    else if(gb.buttons.repeat(BUTTON_UP, 0))
      viewer.nextLimit();
  }
  else
  {
    // Checks buttons for Zoom/Pan
    if(gb.buttons.repeat(BUTTON_RIGHT, 0)) {viewer.move(+10, 0);}
    if(gb.buttons.repeat(BUTTON_LEFT, 0))  {viewer.move(-10, 0);}
    if(gb.buttons.repeat(BUTTON_UP, 0))    {viewer.move(0, +10);}
    if(gb.buttons.repeat(BUTTON_DOWN, 0))  {viewer.move(0, -10);}
    if(gb.buttons.repeat(BUTTON_A, 0))     {viewer.magnify(1.05);}
    if(gb.buttons.repeat(BUTTON_B, 0))     {viewer.magnify(0.95);}
  }
}
