#!/usr/bin/env bash

# execute in directory where you want to postprocess

for f in *_color.png
do
  f2="${f/_color/_scribbles}"
  f3="${f/_color/_color_with_scribbles}"
  f4="${f/_color/_depth_filled_colorization_norm}"
  f5="${f/_color/_depth_with_scribbles}"
  f6="${f/_color/_visualization}"
  f7="${f/_color/_visualization_with_scribbles}"
  if [ -f $f3 ]
  then 
      echo "'$f3' exists"
  else
      convert $f $f2 -transparent white -layers flatten $f3
      echo "'$f3' generated"
  fi
  if [ -f $f5 ]
  then 
      echo "'$f5' exists"
  else
      convert $f4 $f2 -transparent white -layers flatten $f5
      echo "'$f5' generated"
  fi

  if [ -f $f6 ]
  then
      if [ -f $f7 ]
      then 
          echo "'$f7' exists"
      else
          convert $f6 $f2 -transparent white -layers flatten $f7
          echo "'$f7' generated"
      fi
  fi
done
