// Filename: radixSort.h
// 
// Copyright © James Acres
// http://www.jamesacres.com
// http://github.com/jacres
// @jimmyacres
// 
// Created: Wed Jan 21 16:19:25 2015 (-0500)
// Last-Updated: Tue Feb  3 14:18:03 2015 (-0500)

#pragma once

#include <string.h>

namespace lb
{
// 32 and 64 bit radix sorts
// 2 versions: key only sort, and key/value sort
//
// some implementation ideas taken from:
// https://github.com/bkaradzic/bx/blob/master/include/bx/radixsort.h

// sorts to 2 bytes
static void RadixSort16( uint16_t * __restrict keys, uint16_t * __restrict tempKeys, uint16_t num )
{
  uint16_t histogram[256];
  uint16_t offsetTable[256];

  memset( offsetTable, 0, 256 * sizeof( uint16_t ) );

  uint16_t * __restrict currKeys = keys;
  uint16_t * __restrict lastKeys = tempKeys;

  int pass = 0;

  for ( ; pass < 2; ++pass )
  {
    memset( histogram, 0, 256 * sizeof( uint16_t ) );

    uint16_t key = currKeys[ 0 ];
    uint16_t prevKey = key;
    bool sortComplete = true;

    uint8_t shiftBits = pass << 3;
    
    for ( uint16_t i = 0; i < num; ++i )
    {
      key = currKeys[ i ];
      uint8_t rad = ( key >> shiftBits ) & 0xFF;
      ++histogram[ rad ]; 

      // check while constructing histogram if this LSD is already sorted
      sortComplete &= prevKey <= key;
      prevKey = key;
    }
  
    if ( true == sortComplete )
    {
      goto done;    
    }

    offsetTable[0] = 0;
    uint16_t * offset = &offsetTable[0];
    uint16_t * hist = &histogram[0];
  
    for ( uint16_t i = 1; i < 256; ++i, ++offset, ++hist )
    {
      offsetTable[ i ] = *offset + *hist;  
    }
   
    for ( uint16_t i = 0; i < num; ++i )
    {
      key = currKeys[ i ];
      uint8_t rad = ( key >> shiftBits ) & 0xFF;
      uint16_t index = offsetTable[ rad ]++;
  
      lastKeys[ index ] = key;
    }
  
    uint16_t * swapKeys = lastKeys; 
    lastKeys = currKeys;
    currKeys = swapKeys;
  }

 done:

  if ( 0 != ( pass & 1 ) ) // we ended up on an odd numbered pass, need to end up with result in dest
  {
    memcpy( keys, tempKeys, num * sizeof( uint16_t ) );
  }
}

template <typename Type>
static void RadixSort16( uint16_t * __restrict keys, uint16_t * __restrict tempKeys, Type * __restrict values, Type * __restrict tempValues, uint16_t num )
{
  uint16_t histogram[256];
  uint16_t offsetTable[256];

  memset( offsetTable, 0, 256 * sizeof( uint16_t ) );

  uint16_t * __restrict currKeys = keys;
  uint16_t * __restrict lastKeys = tempKeys;

  Type * __restrict currValues = values;
  Type * __restrict lastValues = tempValues;

  int pass = 0;

  for ( ; pass < 2; ++pass )
  {
    memset( histogram, 0, 256 * sizeof( uint16_t ) );

    uint16_t key = currKeys[ 0 ];
    uint16_t prevKey = key;
    bool sortComplete = true;

    uint8_t shiftBits = pass << 3;
    
    for ( uint16_t i = 0; i < num; ++i )
    {
      key = currKeys[ i ];
      uint8_t rad = ( key >> shiftBits ) & 0xFF;
      ++histogram[ rad ]; 

      // check while constructing histogram if this LSD is already sorted
      sortComplete &= prevKey <= key;
      prevKey = key;
    }
  
    if ( true == sortComplete )
    {
      goto done;    
    }

    offsetTable[0] = 0;
    uint16_t * offset = &offsetTable[0];
    uint16_t * hist = &histogram[0];
  
    for ( uint16_t i = 1; i < 256; ++i, ++offset, ++hist )
    {
      offsetTable[ i ] = *offset + *hist;  
    }
   
    for ( uint16_t i = 0; i < num; ++i )
    {
      key = currKeys[ i ];
      uint8_t rad = ( key >> shiftBits ) & 0xFF;
      uint16_t index = offsetTable[ rad ]++;
  
      lastKeys[ index ] = key;
      lastValues[ index ] = currValues[ i ];
    }
  
    uint16_t * swapKeys = lastKeys; 
    lastKeys = currKeys;
    currKeys = swapKeys;

    Type * swapValues = lastValues;
    lastValues = currValues;
    currValues = swapValues;
  }

 done:

  if ( 0 != ( pass & 1 ) ) // we ended up on an odd numbered pass, need to end up with result in dest
  {
    memcpy( keys, tempKeys, num * sizeof( uint16_t ) );
    memcpy( values, tempValues, num * sizeof( Type ) );
  }
}


// sorts to 3 bytes
static void RadixSort32( uint32_t * __restrict keys, uint32_t * __restrict tempKeys, uint32_t num )
{
  uint32_t histogram[256];
  uint32_t offsetTable[256];

  memset( offsetTable, 0, 256 * sizeof( uint32_t ) );

  uint32_t * __restrict currKeys = keys;
  uint32_t * __restrict lastKeys = tempKeys;

  int pass = 0;

  for ( ; pass < 3; ++pass )
  {
    memset( histogram, 0, 256 * sizeof( uint32_t ) );

    uint32_t key = currKeys[ 0 ];
    uint32_t prevKey = key;
    bool sortComplete = true;

    uint8_t shiftBits = pass << 3;
    
    for ( uint32_t i = 0; i < num; ++i )
    {
      key = currKeys[ i ];
      uint8_t rad = ( key >> shiftBits ) & 0xFF;
      ++histogram[ rad ]; 

      // check while constructing histogram if this LSD is already sorted
      sortComplete &= prevKey <= key;
      prevKey = key;
    }
  
    if ( true == sortComplete )
    {
      goto done;    
    }

    offsetTable[0] = 0;
    uint32_t * offset = &offsetTable[0];
    uint32_t * hist = &histogram[0];
  
    for ( uint32_t i = 1; i < 256; ++i, ++offset, ++hist )
    {
      offsetTable[ i ] = *offset + *hist;  
    }
   
    for ( uint32_t i = 0; i < num; ++i )
    {
      key = currKeys[ i ];
      uint8_t rad = ( key >> shiftBits ) & 0xFF;
      uint32_t index = offsetTable[ rad ]++;
  
      lastKeys[ index ] = key;
    }
  
    uint32_t * swapKeys = lastKeys; 
    lastKeys = currKeys;
    currKeys = swapKeys;
  }

 done:

  if ( 0 != ( pass & 1 ) ) // we ended up on an odd numbered pass, need to end up with result in dest
  {
    memcpy( keys, tempKeys, num * sizeof( uint32_t ) );
  }
}

template <typename Type>
static void RadixSort32( uint32_t * __restrict keys, uint32_t * __restrict tempKeys, Type * __restrict values, Type * __restrict tempValues, uint32_t num )
{
  uint32_t histogram[256];
  uint32_t offsetTable[256];

  memset( offsetTable, 0, 256 * sizeof( uint32_t ) );

  uint32_t * __restrict currKeys = keys;
  uint32_t * __restrict lastKeys = tempKeys;

  Type * __restrict currValues = values;
  Type * __restrict lastValues = tempValues;

  int pass = 0;

  for ( ; pass < 3; ++pass )
  {
    memset( histogram, 0, 256 * sizeof( uint32_t ) );

    uint32_t key = currKeys[ 0 ];
    uint32_t prevKey = key;
    bool sortComplete = true;

    uint8_t shiftBits = pass << 3;
    
    for ( uint32_t i = 0; i < num; ++i )
    {
      key = currKeys[ i ];
      uint8_t rad = ( key >> shiftBits ) & 0xFF;
      ++histogram[ rad ]; 

      // check while constructing histogram if this LSD is already sorted
      sortComplete &= prevKey <= key;
      prevKey = key;
    }
  
    if ( true == sortComplete )
    {
      goto done;    
    }

    offsetTable[0] = 0;
    uint32_t * offset = &offsetTable[0];
    uint32_t * hist = &histogram[0];
  
    for ( uint32_t i = 1; i < 256; ++i, ++offset, ++hist )
    {
      offsetTable[ i ] = *offset + *hist;  
    }
   
    for ( uint32_t i = 0; i < num; ++i )
    {
      key = currKeys[ i ];
      uint8_t rad = ( key >> shiftBits ) & 0xFF;
      uint32_t index = offsetTable[ rad ]++;
  
      lastKeys[ index ] = key;
      lastValues[ index ] = currValues[ i ];
    }
  
    uint32_t * swapKeys = lastKeys; 
    lastKeys = currKeys;
    currKeys = swapKeys;

    Type * swapValues = lastValues;
    lastValues = currValues;
    currValues = swapValues;
  }

 done:

  if ( 0 != ( pass & 1 ) ) // we ended up on an odd numbered pass, need to end up with result in dest
  {
    memcpy( keys, tempKeys, num * sizeof( uint32_t ) );
    memcpy( values, tempValues, num * sizeof( Type ) );
  }
}

// sorts to 6 bytes
static void RadixSort64( uint64_t * __restrict keys, uint64_t * __restrict tempKeys, uint32_t num )
{
  uint32_t histogram[256];
  uint64_t offsetTable[256];

  memset( offsetTable, 0, 256 * sizeof( uint64_t ) );

  uint64_t * __restrict currKeys = keys;
  uint64_t * __restrict lastKeys = tempKeys;

  int pass = 0;

  for ( ; pass < 6; ++pass )
  {
    memset( histogram, 0, 256 * sizeof( uint32_t ) );

    uint64_t key = currKeys[ 0 ];
    uint64_t prevKey = key;
    bool sortComplete = true;

    uint8_t shiftBits = pass << 3;
    
    for ( uint32_t i = 0; i < num; ++i )
    {
      key = currKeys[ i ];
      uint8_t rad = ( key >> shiftBits ) & 0xFF;
      ++histogram[ rad ]; 

      // check while constructing histogram if this LSD is already sorted
      sortComplete &= prevKey <= key;
      prevKey = key;
    }

    if ( true == sortComplete )
    {
      goto done;    
    }
  
    offsetTable[0] = 0;
    uint64_t * offset = &offsetTable[0];
    uint32_t * hist = &histogram[0];
  
    for ( uint32_t i = 1; i < 256; ++i, ++offset, ++hist )
    {
      offsetTable[ i ] = *offset + *hist;  
    }
   
    for ( uint32_t i = 0; i < num; ++i )
    {
      key = currKeys[ i ];
      uint8_t rad = ( key >> shiftBits ) & 0xFF;
      uint64_t index = offsetTable[ rad ]++;
  
      lastKeys[ index ] = key;
    }
  
    uint64_t * swapKeys = lastKeys; 
    lastKeys = currKeys;
    currKeys = swapKeys;
  }

done:

  if ( 0 != ( pass & 1 ) ) // we ended up on an odd numbered pass, need to end up with result in dest
  {
    memcpy( keys, tempKeys, num * sizeof( uint64_t ) );
  }
}

// sorts to 6 bytes
template <typename Type>
static void RadixSort64( uint64_t * __restrict keys, uint64_t * __restrict tempKeys, Type * __restrict values, Type * __restrict tempValues, uint32_t num )
{ 
  uint32_t histogram[256];
  uint64_t offsetTable[256];

  memset( offsetTable, 0, 256 * sizeof( uint64_t ) );

  uint64_t * __restrict currKeys = keys;
  uint64_t * __restrict lastKeys = tempKeys;

  Type * __restrict currValues = values;
  Type * __restrict lastValues = tempValues;

  int pass = 0;

  for ( ; pass < 5; ++pass )
  {
    memset( histogram, 0, 256 * sizeof( uint32_t ) );

    uint64_t key = currKeys[ 0 ];
    uint64_t prevKey = key;
    bool sortComplete = true;

    uint8_t shiftBits = pass << 3;
    
    for ( size_t i = 0; i < num; ++i )
    {
      key = currKeys[ i ];
      uint8_t rad = ( key >> shiftBits ) & 0xFF;
      ++histogram[ rad ]; 

      // check while constructing histogram if this LSD is already sorted
      sortComplete &= prevKey <= key;
      prevKey = key;
    }

    if ( true == sortComplete )
    {
      goto done;    
    }
  
    offsetTable[0] = 0;
    uint64_t * offset = &offsetTable[0];
    uint32_t * hist = &histogram[0];
  
    for ( uint32_t i = 1; i < 256; ++i, ++offset, ++hist )
    {
      offsetTable[ i ] = *offset + *hist;  
    }
   
    for ( uint32_t i = 0; i < num; ++i )
    {
      key = currKeys[ i ];
      uint8_t rad = ( key >> shiftBits ) & 0xFF;
      uint64_t index = offsetTable[ rad ]++;
  
      lastKeys[ index ] = key;
      lastValues[ index ] = currValues[ i ];
    }
  
    uint64_t * swapKeys = lastKeys; 
    lastKeys = currKeys;
    currKeys = swapKeys;

    Type * swapValues = lastValues;
    lastValues = currValues;
    currValues = swapValues;
  }

done:

  if ( 0 != ( pass & 1 ) ) // we ended up on an odd numbered pass (pass is incremented 1 beyond), need to end up with result in dest
  {
    memcpy( keys, tempKeys, num * sizeof( uint64_t ) );
    memcpy( values, tempValues, num * sizeof( Type ) );
  }
}
}
