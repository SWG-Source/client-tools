/******************************************************************************
 *  AtiDXTC.cpp -- Contains all compression and decompression routines for DXT1,3,5
 ******************************************************************************
 $Header:: /Sushi/RunTime/AtiDXTC.cpp 6     1/18/01 12:11a Avlachos           $
 ******************************************************************************
 *  Created by Alex Vlachos (AVlachos@ati.com) on 3-26-2001
 *  (C) 2000 ATI Research, Inc.  All rights reserved.
 ******************************************************************************/

static char sgCopyrightString[] = "\r\n\r\n(C) 2000 ATI Research, Inc.\r\n\r\n";

// INCLUDES ===================================================================
#include "includes.h"

#ifdef ATI_MAC_OS
 #include <Endian.h>
#endif

// STATIC GLOBALS =============================================================
static uint8 *sgTexels = NULL;
static uint32 sgNumBytesAllocated = 0;

// FUNCTIONS ==================================================================
uint8* AtiDXTCDecompressMipChain (bool8 aMipMapped, int32 aWidth, int32 aHeight, uint8 *aCompressedTexels, uint8 *aType)
{
   int32 i;
   int32 j;
   int32 k;
   int32 m;
   uint32 tmpi;
   int32 mipLevel;
   int32 compressedOffset;
   int32 decompressedOffset;
   int16 width = aWidth;
   int16 height = aHeight;
   uint8 *final;

   /********************************/
   /* Allocate memory if necessary */
   /********************************/
   tmpi = width*height*4*2;
   if (tmpi > sgNumBytesAllocated)
   {
      AtiDXTCFreeMemory();
      sgTexels = (uint8 *) malloc (sizeof(uint8)*tmpi);
      if (sgTexels == NULL)
      {
         SuErrorLog("Error allocating memory for sgTexels in AtiDXTCDecompressMipChain!\n");
         return NULL;
      }

      sgNumBytesAllocated = tmpi;
   }

   /***************************************/
   /* Decompress each mip level at a time */
   /***************************************/
   compressedOffset = 0;
   decompressedOffset = 0;
   final = sgTexels;
   for (mipLevel=0; 1; mipLevel++) //For each mip level
   {
      uint16 *currentBlock = (uint16*)&(aCompressedTexels[compressedOffset]);
      uint32 blockSize = 8; //8 bytes

      /***********************************************************/
      /* Calculate number of final 4x4 blocks for this mip level */
      /***********************************************************/
      int32 w = width;
      int32 h = height;
      int32 numBlocks;
      if (w < 4)
         w = 4;
      w /= 4;
      if (h < 4)
         h = 4;
      h /= 4;
      numBlocks = w*h;

      /*******************************/
      /* Decompress entire mip level */
      /*******************************/
      for (i=0; i<h; i++) //For each row of blocks
      {
         for (j=0; j<w; j++) //For each block in row j
         {
            uint8 block[4][4][4];
            memset(block, 0, sizeof(block));

            /*****************************************/
            /* Decompress DXT3 alpha into same block */
            /*****************************************/
            if ((*aType) == ATI_TEXTURE_DXT3)
            {
               AtiDXTCDecompressDXT3Block(currentBlock, block);
               currentBlock += 4;
            }

            /*****************************************/
            /* Decompress DXT5 alpha into same block */
            /*****************************************/
            if ((*aType) == ATI_TEXTURE_DXT5)
            {
               AtiDXTCDecompressDXT5Block(currentBlock, block);
               currentBlock += 4;
            }

            /*************************************/
            /* Decompress RGB(A) block, aka DXT1 */
            /*************************************/
            AtiDXTCDecompressDXT1Block(currentBlock, block, ((*aType == ATI_TEXTURE_DXT1) ? TRUE : FALSE));
            currentBlock += 4;

            /**************************************************/
            /* Copy decompressed block into final texel array */
            /**************************************************/
            for (k=0; k<4; k++) //For each row of the block
            {
               for (m=0; m<4; m++) //For each pixel in the block
               {
                  tmpi = (decompressedOffset) + (i*width*4*4) + (k*width*4) + (j*4*4) + (m*4);
                  sgTexels[tmpi + 0] = block[m][k][2];
                  sgTexels[tmpi + 1] = block[m][k][1];
                  sgTexels[tmpi + 2] = block[m][k][0];
                  sgTexels[tmpi + 3] = block[m][k][3];
               }
            }
         }
      }

      /******************/
      /* Terminate loop */
      /******************/
      if (aMipMapped == FALSE) //If not mip mapping
      {
         break;
      }

      if ((width == 1) && (height == 1)) //If just added 1x1 mip level
      {
         break;
      }

      /*******************************/
      /* Increment index into arrays */
      /*******************************/
      if ((*aType) == ATI_TEXTURE_DXT1)
         compressedOffset += 8*numBlocks; //DXT1
      else
         compressedOffset += 8*numBlocks*2; //DXT3 or DXT5
      
      decompressedOffset += width*height*4; //Always compress into a 32-bit texture

      /***************************/
      /* Update image dimensions */
      /***************************/
      width >>= 1;
      if (width < 1)
         width = 1;

      height >>= 1;
      if (height < 1)
         height = 1;
   }

   /********************************/
   /* Change the format and return */
   /********************************/
   *aType = ATI_TEXTURE_RGBA;
   return sgTexels;
}

//=============================================================================
void AtiDXTCFreeMemory (void)
{
   if (sgTexels != NULL)
   {
      free(sgTexels);
      sgTexels = NULL;
   }
   sgNumBytesAllocated = 0;
}

//=============================================================================
void AtiDXTCDecompressDXT1Block (uint16 *block, uint8 dest[4][4][4], bool8 dxt1)
{
   int32 x;
   int32 y;
   uint16 c0;
   uint16 c1;
   uint8 colors[2][3];

#ifdef ATI_MAC_OS
   block[0] = Endian16_Swap(block[0]);
   block[1] = Endian16_Swap(block[1]);
   block[2] = Endian16_Swap(block[2]);
   block[3] = Endian16_Swap(block[3]);
#endif

   /********************/
   /* Get the 2 colors */
   /********************/
   c0 = block[0];
   c1 = block[1];

   //Convert to 24-bits
   colors[0][0] = ((c0>>11)&0x1f)<<3;
   colors[0][1] = ((c0>>5)&0x3f)<<2;
   colors[0][2] = (c0&0x1f)<<3;

   colors[1][0] = ((c1>>11)&0x1f)<<3;
   colors[1][1] = ((c1>>5)&0x3f)<<2;
   colors[1][2] = (c1&0x1f)<<3;

   /************************/
   /* Decompress the block */
   /************************/
   for (y=0; y<4; y++) //For each row
   {
      for (x=0; x<4; x++) //For each pixel in this row
      {
         uint8 bits;

         /************************/
         /* Get the 2 bit values */
         /************************/
         bits = (block[2+(y>>1)] >> (8*(y&0x1)+(x<<1))) & 0x3;

         /******************/
         /* Compute colors */
         /******************/
         if (c0 >= c1) //4 color block
         {
            switch (bits)
            {
               case 0x0:
               case 0x1:
                  dest[x][y][0] = colors[bits][0];
                  dest[x][y][1] = colors[bits][1];
                  dest[x][y][2] = colors[bits][2];
                  break;

               case 0x2:
                  dest[x][y][0] = ((colors[0][0]<<1) + colors[1][0]) / 3;
                  dest[x][y][1] = ((colors[0][1]<<1) + colors[1][1]) / 3;
                  dest[x][y][2] = ((colors[0][2]<<1) + colors[1][2]) / 3;
                  break;

               case 0x3:
                  dest[x][y][0] = (colors[0][0] + (colors[1][0]<<1)) / 3;
                  dest[x][y][1] = (colors[0][1] + (colors[1][1]<<1)) / 3;
                  dest[x][y][2] = (colors[0][2] + (colors[1][2]<<1)) / 3;
                  break;
            }
         }
         else //3 color block with 1-bit alpha
         {
            if (bits == 0x3)
            {
               dest[x][y][0] = 0;
               dest[x][y][1] = 0;
               dest[x][y][2] = 0;
               dest[x][y][3] = 0;
            }
            else
            {
               switch (bits)
               {
                  case 0x0:
                  case 0x1:
                     dest[x][y][0] = colors[bits][0];
                     dest[x][y][1] = colors[bits][1];
                     dest[x][y][2] = colors[bits][2];
                     dest[x][y][3] = 255;
                     break;

                  case 0x2:
                     dest[x][y][0] = (colors[0][0] + colors[1][0]) >> 1;
                     dest[x][y][1] = (colors[0][1] + colors[1][1]) >> 1;
                     dest[x][y][2] = (colors[0][2] + colors[1][2]) >> 1;
                     dest[x][y][3] = 255;
                     break;
               }
            }
         }
      }
   }
}

//=============================================================================
void AtiDXTCDecompressDXT3Block (uint16 *block, uint8 dest[4][4][4])
{
   uint16 x;
   uint16 y;

#ifdef ATI_MAC_OS
   block[0] = Endian16_Swap(block[0]);
   block[1] = Endian16_Swap(block[1]);
   block[2] = Endian16_Swap(block[2]);
   block[3] = Endian16_Swap(block[3]);
#endif

   /************************/
   /* Decompress the block */
   /************************/
   for (y=0; y<4; y++) //For each row
   {
      for (x=0; x<4; x++) //For each pixel in this row
      {
         /***********************/
         /* Get the 4 bit value */
         /***********************/
         dest[x][y][3] = (uint8)(((block[y] >> (x*4)) & 0x000f) << 4);
      }
   }
}

//=============================================================================
void AtiDXTCDecompressDXT5Block (uint16 *block, uint8 dest[4][4][4])
{
   int32 x;
   int32 y;
   uint8 alpha[8];

#ifdef ATI_MAC_OS
   block[0] = Endian16_Swap(block[0]);
   block[1] = Endian16_Swap(block[1]);
   block[2] = Endian16_Swap(block[2]);
   block[3] = Endian16_Swap(block[3]);
#endif

   /**************/
   /* Get colors */
   /**************/
   alpha[0] = block[0] & 0x00ff;
   alpha[1] = (block[0] >> 8) & 0x00ff;


   //8-alpha or 6-alpha block?
   if (alpha[0] >= alpha[1])
   {    
       //8-alpha block: Derive the other six alphas
       //Bit code 000 = alpha_0, 001 = alpha_1, others are interpolated
       alpha[2] = (6 * alpha[0] + 1 * alpha[1] + 3) / 7; //Bit code 010
       alpha[3] = (5 * alpha[0] + 2 * alpha[1] + 3) / 7; //Bit code 011
       alpha[4] = (4 * alpha[0] + 3 * alpha[1] + 3) / 7; //Bit code 100
       alpha[5] = (3 * alpha[0] + 4 * alpha[1] + 3) / 7; //Bit code 101
       alpha[6] = (2 * alpha[0] + 5 * alpha[1] + 3) / 7; //Bit code 110
       alpha[7] = (1 * alpha[0] + 6 * alpha[1] + 3) / 7; //Bit code 111
   }    
   else
   {
       //6-alpha block
       //Bit code 000 = alpha_0, 001 = alpha_1, others are interpolated
       alpha[2] = (4 * alpha[0] + 1 * alpha[1] + 2) / 5; //Bit code 010
       alpha[3] = (3 * alpha[0] + 2 * alpha[1] + 2) / 5; //Bit code 011
       alpha[4] = (2 * alpha[0] + 3 * alpha[1] + 2) / 5; //Bit code 100
       alpha[5] = (1 * alpha[0] + 4 * alpha[1] + 2) / 5; //Bit code 101
       alpha[6] = 0;                                     //Bit code 110
       alpha[7] = 255;                                   //Bit code 111
   }

   /************************/
   /* Decompress the block */
   /************************/
   for (y=0; y<4; y++) //For each row
   {
      for (x=0; x<4; x++) //For each pixel in this row
      {
         uint8 bits;
         uint8 pixel;

         pixel = y*4 + x;
         switch (pixel)
         {
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
               bits = (block[1] >> (pixel*3)) & 0x07;
               break;

            case 5:
               bits = ((block[1] >> 15) & 0x01) | ((block[2] << 1) & 0x06);
               break;

            case 6:
            case 7:
            case 8:
            case 9:
               bits = (block[2] >> ((pixel-6)*3 + 2)) & 0x07;
               break;

            case 10:
               bits = ((block[2] >> 14) & 0x03) | ((block[3] & 0x01) << 2);
               break;

            case 11:
            case 12:
            case 13:
            case 14:
            case 15:
               bits = (block[3] >> ((pixel-11)*3 + 1)) & 0x07;
               break;
         }

         dest[x][y][3] = alpha[bits];
      }
   }
}
