# Affine3D

This tutorial follows the first [Affine tutorial](https://gamebuino.com/community/topic/affine-full-screen-picture-zoom-and-rotation), where we saw how to display a full-screen picture with zoom and rotation.

Here we will see how to re-use the same concepts to display a picture with a 3D effect, then do the same with a tile-map and finally design a mini kart game.

## Introduction

The [Mode 7](https://en.wikipedia.org/wiki/Mode_7) is famous in the world of gaming. With this feature, Nintendo introduced a hardware-based image transformation engine able to support a wide range of 3D effects [used in various games](https://www.youtube.com/watch?v=Z_3clFDpXrg). The idea here is to re-implement something similar on the Gamebuino.

The first Affine tutorial was limited to true [affine transformations](https://en.wikipedia.org/wiki/Affine_transformation) allowing zoom, rotation and shear.

The trick in this tutorial is to **display an image through an affine transformation but change the transformation parameters for each row of pixels**.

Let's start.

## Part 1: Setup

Like in the first tutorial we will start with an image of 256x256 pixels:

![picture](assets/Picture.bmp)

So create a new sketch and use the [image transcoder](https://gamebuino.com/creations/image-transcoder) to transform it into code in mode RGB565, calling it "`Picture`". At the beginning of the .ino file, you should now have something like this (shortened here):

```C++
#include <Gamebuino-Meta.h>

const uint16_t PictureData[] = {256,256,1, 1, 0, 0, 0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0, .....
```

After that, we add some basic code like in the Affine tutorial to:

- Init everything with `gb.begin()` and framerate setting in the `setup()` function
- Wait for screen update, measure the duration of the draw code and display this duration in microseconds on the screen, in the `loop()` function. Note that printing the string `\xB5` displays actually the greek Mu letter.

```C++
void setup()
{
  // Starts Gamebuino library and sets target frame rate
  gb.begin();
  gb.setFrameRate(50);
}

// Time measurement variable
unsigned long previousStartTime = micros();

void loop()
{
  // Waits for screen refresh
  while(!gb.update());

  // Gets the time at start of the computation
  unsigned long startTime = micros();

  // [ DRAW CODE GOES HERE ]

  // Computes and displays duration
  unsigned long duration = micros()-startTime;
  gb.display.clearTextVars();
  gb.display.setColor(WHITE, BLACK);
  gb.display.print(duration); gb.display.print(" \xB5s");
}
```

## Part 2: Fixed-point arithmetic macros

In order to improve the readability of the code, we will use macros to perform [fixed-point arithmetic](https://en.wikipedia.org/wiki/Fixed-point_arithmetic). Refer to the [Affine tutorial](https://gamebuino.com/community/topic/affine-full-screen-picture-zoom-and-rotation) for fixed-point arithmetic basics.

This code goes at the top of the sketch file.

First of all, we define a new data type for our base fixed-point type, i.e. we will use `FP32` as type for the variable carying a fixed-point value. Unlike in the Affine tutorial, this basic type is not a `union`:

```C++
// Definition of 32-bits fixed-point type for better readability
typedef int32_t FP32;
```

Then we define a macro for conversion from a float to a fixed-point value (instead of writing the same syntax again and again). We use 32-bits fixed-point values, with 16 bits before the dot and 16 bits after the dot. The transformation if done by multiplying the float value by the 2^16=65536 in order to shift the bits to the left, and then to convert the result to an integer to keep only the integer value:

```C++
// FP32 macro to convert from float to FP32
#define FP32_FROM_FLOAT(a) ( (FP32) (65536.0*(a)) )
```

ATTENTION: A macro is not a function. A macro is treated before compilation by the [C preprocessor](https://en.wikipedia.org/wiki/C_preprocessor) and can be seen as a smart text search/replace feature. As an example, here, each time the character string `FP32_FROM_FLOAT(something)` is found in the code, then it gets _replaced_ by `( (FP32) (65536.0*(something)) )`. There is no type or syntax check. It is advisable to use lots of parenthesis in a macro definition to be sure that the replaced string is not interpreted together with surrounding text, leading to strange compilation or run-time errors.

The main advantage of using macros instead of functions is that, after pre-processing, a complex expression remains largely self-contained and does not depend on function calls. Usually, the compiler can better optimize the code if all computation steps are in the same complex expression.

Like in the Affine tutorial, we will need to access different parts of the 32-bits fixed-point values. So we re-use the `union` to access these parts:

```C++
// Defines a union type giving access to the data as 8, 16 or 32 bits integers
union FixedPoint
{
  int32_t  asInt32;
  int16_t  asInt16[2];
  uint16_t asUInt16[2];
  int8_t   asInt8[4];
  uint8_t  asUInt8[4];
};
```

As a side note, it would be possible to access the different parts using bit-shifts and AND/OR operations, but measurements show a better performance by using such a `union` type.

Actually we will not need all members of the `union`. We only need to access the integer part of the fixed-point value, i.e. the 16 most significant bits, and we need to access them in the form of bytes.

So first we define a macro to retrieve the most significant byte of the integer part (taking endianness into account and the strange cast to a `union`):

```C++
// FP32 macro to extract most significant signed byte of the integer part of an FP32 value
//#define FP32_MSBYTE(A) ( (int8_t) ( ((int8_t*)&(A))[3]) )
#define FP32_MSBYTE(A) ( (int8_t) ( (FixedPoint({(A)})).asInt8[3] ) )
```

Second we define a macro to retrieve the least significant byte of the integer part as unsigned byte:

```C++
// FP32 macro to extract least significant unsigned byte of the integer part of an FP32 value
//#define FP32_LSBYTE(A) ( (uint8_t) ( (uint8_t*)&(A))[2] )
#define FP32_LSBYTE(A) ( (uint8_t) ( (FixedPoint({(A)})).asUInt8[2] ) )
```

At this stage a small example is certainly useful. So let's take the value `a=500.5`:

- `a*65536 = A = 32800768`, so `FP32_FROM_FLOAT(500.5)` gives the value `A=32800768` or `0x01F48000` in hexadecimal.
- The hexadecimal view shows clearly the integer part of this value equal to `0x01F4` or `500` in decimal.
- In turn, the most significant byte of the integer part is `0x01` so `FP32_MSBYTE(A)` return `0x01` or `1` in decimal.
- And the least significant byte of the integer part is `0xF4` so `FP32_LSBYTE(A)` return `0xF4` or `244` in decimal (unsigned).

I can imagine that this part is hard to follow. It is not necessary to understand completely the bits and bytes here to understand the rest of the tutorial. Just use the macros definition as provided.

## Implementation of the row draw procedure

Now that we have a comprehensive set of macros and type definitions, we can re-write the function that draws one row of pixels with an affine transformation.

As a reminder, the draw principle for each row of pixels on the screen, given a source picture, is as following:

- The pixels row is drawn from left to right, and the color of each pixel is read from a straight line on the source picture starting at a given position `(x,y)`, where `x` and `y` are fixed-point values.
- For each pixel to draw on the screen row, its color is read from the source picture at a position equal the the integer parts of `x` and `y`.
- Once the pixel is written on the screen, we go to the next pixel to draw, and the values of `x` and `y` are incremented by the fixed-point values `dx` and `dy`, i.e. `x=x+dx` and `y=y+dy`

That's all! It is really just like: take the color at `(x,y)` on the source picture, write it on the screen, then go to the next position on the source picture by adding `(dx,dy)` to `(x,y)`, and so on. The constant "row increment" `(dx,dy)` leads naturally to a straight line on the source picture.

We will access the screen and the source picture directly in memory for performance. Following the [good remarks of Alban](https://gamebuino.com/@Alban), let's just define 2 pointers on the pixel buffers `Color* source` and `Color* destination`, which we will use like array of Color types (i.e. 16-bits integers as each RGB565 pixel is encoded on 2 bytes).















## Part 2: Non-affine deformation



## Part 4: Non-optimized 3D projection

Let's now try to do something more useful with the draw function: simulate a 3D view of the source picture.

So we consider an observer at **O** looking down to the picture through a **screen** such that the top of the screen is located at the same height as the observer, here seen from the right:

![right](projection-right.png)

The observer will see the first line of the picture at **A** and the last line at **B**. From A to B, due to the perspective, the distance decreases between successive drawn lines (i.e. objects appear smaller when they are farer). So the _row increment_, as named in the first Affine tutorial, is not constant during the draw.

Please note that, at the very top of the screen, the observer looks to the infinite and its line of sight will not cross the source picture. Furthermore, the first lines at the top of the screen will show the picture at such a distance that it is no use  drawing them. A kind of "moire" effect can be expected here. A consequence of these remarks is that we will skip the top lines during the draw.

We define the following parameters (all positive):

- _s_ : Distance between the observer and the screen
- _h_ : Height of the observer, i.e. distance between the observer and the plane containing the source picture
- _y_ : Coordinate on the screen of the row of pixels being drawn
- _d_ : Distance between observer and point in the source picture being drawn (projected in the plane containing the source picture)

We can derive a first equation from this view. It is noticeable that the line from observer to top of screen and the plane containing the source picture are parallel, and the line OA crosses them. As a result, the triangles formed by a segment of the line OA, one of the parallel horizontal lines and one vertical lines are right triangles and are all similar. The ratios of the length of their edges are equal, in particular:

    s/y = d/h

Seen from the top, the rotating observer at **O** sees the picture through the screen at **C** on the source picture when looking at the middle of the screen, and at **D** when looking at the left edge of the screen:

![top](projection-top.png)

We define the following additional parameters (all positive):

- _w_ : Half-width of the screen
- _c_ : Half-length of a straight-line read from the source picture
- _a_ : Angle of the view direction of the observer in the plane containing the source picture

Here again we can notice similar triangles in OCD and equal ratios, in particular:

    w/s = c/d

This equation shows very clearly the proportional relationship between pixel coordinates on a row and the positions on the line DC. In other words, it shows that the "column increment" is constant during the draw and that we can use a row draw function with constant column increment.

Now, the next step is to determine the position in the source picture of the start point of each row to draw, and the corresponding column increment. The start point is D on the picture, and the column increment can be deduced from DC.

Classically, we use trigonometry to calculate the coordinates of D by projecting the components on each axis. If O has the coordinates _(Ox,Oy)_ and D has _(Dx,Dy)_, using the parameters as previously defined and the (inverted) coordinate system shown on the diagram, we go from O to D through the vectors OC and CD:

    Dx = Ox + d*cos(a) - c*sin(a)
    Dy = Oy - d*sin(a) - c*cos(a)

The column increment with coordinates _(Ix,Iy)_ can be calculated as the vector DC divided by _w_, number of pixels to draw:

    Ix = c*sin(a)/w
    Iy = c*cos(a)/w

We have now the basic formulas but _c_ and _d_ are still unknown, so we need to re-use the equations found previously.

    d = s*h/y
    c = d*w/s = (s*h/y)*w/s = w*h/y

And finally:

    Dx = Ox + s*h*cos(a)/y - w*h*sin(a)/y
    Dy = Oy - s*h*sin(a)/y - w*h*cos(a)/y
    Ix = h*sin(a)/y
    Iy = h*cos(a)/y



## Part 4: Optimization with fixed-point computation

As a reminder, to encode a floating-point value into a fixed-point integer value we shift the bits to the left, or multiply by a power of 2, such that the fractional part moves to the integer part. Then we convert to an integer, removing the fractional part. For example,  if `aFP` is the encoded value of `a`, with 16 bits precision after the dot:

```C++
aFP.asInt32 = (int32_t)(65536.0*a);
```

In this section we will use capital letters for fixed-point encoded values, and small letters for their float equivalents. We will also consider that the remaining fractional part, cut while converting to an integer, is neglectible. As a result, we do not need to take the integer conversion into account.

So if we name `F` the factor power of 2 used for conversion (here equal to 65536), we can define the 3 variables encoded as fixed-point values:

    A = a*F
    B = b*F
    C = c*F

Let's look at the **Multiplication of fixed-point values**. We search `C`, encoded value of `c`, such that:

    c = a * b

After replacements:

    c = C/F = A/F * B/F

Or:

    C = A * B / F

It means that after multiplying the 2 integer values, we need to shift the result to re-align it to the dot. This implies that the result of `A * B` is encoded on 64 bits.

There is a trick to perform just a 32-bits multiplication, even if we lose some precision: before the multiplication, we can pre-shift `A` and `B`. I used that in Fractalino. Let's define `G` as the factor representing a half-shift (here it is equal to 256), i.e. such that:

    F = G*G

Replacing it in the multiplication formula:

    C = (A/G) * (B/G)

Mathematically it makes no difference, but allows to first shift our values to 16-bits integers, losing the most-significant bits that would anyway overflow, and then multiply and get directly the 32-bits value.

Now let's look at the **division of fixed-point values**. We search `C`, encoded value of `c`, such that:

    c = a / b

After replacements:

    c = C/F = A/B

Or:

    C = (A * F) / B

It means that before dividing, we need to pre-shift `A` on 64 bits.





