#pragma once
#include "boost\variant.hpp"

#include "Color.h"
#include "StringTable.h"

enum class UIBackgroundType : unsigned int
{
   Solid = 0,
   Gradient,
   Image,
   COUNT
};

typedef boost::variant<
   float, 
   bool, 
   UIBackgroundType, 
   Colorf,
   InternString> UIOptionVar;

enum class UIOption : unsigned int
{
   //Type: Colorf (Gradient type will make color 1.0 - 0.15 top to botom, Image will color the image!)
   BackgroundColor = 0,

   //Type: UIBackgroundType
   BackgroundType,

   //Type: InternString (Texture Name)
   BackgroundImage,
   
   //Type: Colorf
   BorderColor,

   //Border Widths, Type: float
   //These only affect elements with the Bordered decorator!
   BorderLeft,
   BorderRight,
   BorderTop,
   BorderBottom,

   //Margin Widths, Type: float
   MarginLeft,
   MarginRight,
   MarginTop,
   MarginBottom,

   //Padding Widths, Type: float
   PaddingLeft,
   PaddingRight,
   PaddingTop,
   PaddingBottom,
   IgnorePadding,

   //Anchors, distance from each of the parent's relative edges
   //Type: float
   Left,
   Right,
   Top,
   Bottom,

   //Type: bool
   //X not valid if right or left anchors set, Same for Y and top/bottom
   CenterX,
   CenterY,

   //Type: float
   //These will be used if:
   // -Anchored to one side but not the other
   // -Centered
   Height,
   Width,

   //Type: bool
   //Will set bounds to encompass the relative bounds of all children, not clipped
   //This has the same effect of setting Height or Width when used with Anchoring/Centering
   FitToChildren,

   //Type: bool
   //Only valid for elements with the Bordered Decorator!
   Resizeable,

   //DONT USE
   COUNT
};