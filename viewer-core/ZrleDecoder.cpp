// Copyright (C) 2011,2012 GlavSoft LLC.
// All rights reserved.
//
//-------------------------------------------------------------------------
// This file is part of the TightVNC software.  Please visit our Web site:
//
//                       http://www.tightvnc.com/
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//-------------------------------------------------------------------------
//

#include <vector>

#include "ZrleDecoder.h"

typedef vector<unsigned int> Palette;

ZrleDecoder::ZrleDecoder(LogWriter *logWriter)
: DecoderOfRectangle(logWriter)
{
  m_encoding = EncodingDefs::ZRLE;
}

ZrleDecoder::~ZrleDecoder()
{
}

void ZrleDecoder::decode(RfbInputGate *input,
                         FrameBuffer *frameBuffer,
                         const Rect *dstRect)
{
  size_t maxUnpackedSize = getMaxSizeOfRectangle(dstRect);
  inflate(input, maxUnpackedSize);

  size_t outputSize = m_inflater.getOutputSize();
  if (outputSize == 0) {
    m_logWriter->debug(_T("Empty unpacked data (zrle-decoder)"));
    if (dstRect->area() != 0) {
      m_logWriter->detail(_T("Corrupted data in zrle-decoder, rectangle is undefined."));
      m_logWriter->detail(_T("Possible, data is corrupted or error in server"));
    }
    return;
  }
  vector<unsigned char> out;
  out.resize(outputSize);
  out.assign(m_inflater.getOutput(), m_inflater.getOutput() + outputSize);
  size_t readed = 0;

  m_bytesPerPixel = frameBuffer->getBytesPerPixel();
  m_numberFirstByte = 0;

  // FIXME: test this code.
  PixelFormat pxFormat = frameBuffer->getPixelFormat();
  if (pxFormat.bitsPerPixel == 32 && pxFormat.colorDepth <= 24) {
    UINT32 colorMaxValue = pxFormat.blueMax  << pxFormat.blueShift  |
                           pxFormat.greenMax << pxFormat.greenShift |
                           pxFormat.redMax   << pxFormat.redShift;
    bool bytesIsUnUse[4] = {0, 0, 0, 0};
    for (int i = 0; i < 4; i++) {
      bytesIsUnUse[i] = (colorMaxValue & 0xFF) == 0;
      colorMaxValue >>= 8;
    }
    if (bytesIsUnUse[3]) {
      m_bytesPerPixel = 3;
      m_numberFirstByte = 0;
    } else if (bytesIsUnUse[0]) {
      m_bytesPerPixel = 3;
      m_numberFirstByte = 1;
    }
  }
  for (int y = dstRect->top; y < dstRect->bottom; y += TILE_SIZE) 
    for (int x = dstRect->left; x < dstRect->right; x += TILE_SIZE) {
      Rect tileRect(x, y, 
                    min(x + TILE_SIZE, dstRect->right),
                    min(y + TILE_SIZE, dstRect->bottom));

      if (!frameBuffer->getDimension().getRect().intersection(&tileRect).isEqualTo(&tileRect))
        throw Exception(_T("Error in protocol: incorrect size of tile (zrle-decoder)"));
      size_t tileLength = tileRect.area();
      size_t tileBytesLength = tileLength * m_bytesPerPixel;
      vector<char> pixels;
      pixels.resize(tileBytesLength);

      int type = readType(out, &readed);

      // raw pixel data
      if (type == 0)
        readRawTile(out, &readed, pixels, &tileRect);

      // a solid tile consisting of a single colour
      if (type == 1)
        readSolidTile(out, &readed, pixels, &tileRect);

      // packed palette
      if (type >= 2 && type <= 16)
        readPackedPaletteTile(out, &readed, pixels, &tileRect, type);

      // plain rle
      if (type == 128)
        readPlainRleTile(out, &readed, pixels, &tileRect);

      // palette rle
      if (type >= 130 && type <= 255) 
        readPaletteRleTile(out, &readed, pixels, &tileRect, type);

      // unused types
      if (type >= 17 && type <= 127 || type == 129) {
        StringStorage error;
        error.format(_T("Error: subencoding %d of Zrle encoding is unused"), type);
        throw Exception(error.getString());
      }

      drawTile(frameBuffer, &tileRect, &pixels);
    } // tile(x, y)
}

void ZrleDecoder::inflate(RfbInputGate *input, size_t unpackedSize)
{
  UINT32 length = input->readUInt32();
  std::vector<char> zlibData;
  zlibData.resize(length);
  if (length == 0) {
    zlibData.resize(1);
  }
  input->readFully(&zlibData.front(), length);

  m_inflater.setInput(&zlibData.front(), length);

  m_inflater.setUnpackedSize(unpackedSize);
  m_inflater.inflate();
}

size_t ZrleDecoder::getMaxSizeOfRectangle(const Rect *dstRect)
{
  size_t widthCount = (dstRect->getWidth() + TILE_SIZE - 1) / TILE_SIZE;
  size_t heightCount = (dstRect->getHeight() + TILE_SIZE - 1) / TILE_SIZE;
  size_t tileCount = widthCount * heightCount;
  return TILE_LENGTH_SIZE + MAXIMAL_TILE_SIZE * tileCount;
}

int ZrleDecoder::readType(const vector<unsigned char> &out,
                          size_t *const readed)
{
  if (*readed >= out.size())
    throw Exception(_T("error in read type (zrle-decoder): out of input-buffer"));
  int type = out[*readed];
  (*readed)++;
  return type;
}

size_t ZrleDecoder::readRunLength(const vector<unsigned char> &out,
                                  size_t *const readed)
{
  size_t runLength = 0;
  size_t delta;
  do {
    if (*readed >= out.size())
      throw Exception(_T("error in read-run-lenght (zrle-decoder): out of input-buffer"));
    delta = out[*readed];
    (*readed)++;
    runLength += delta;
  } while (delta == 255); // if value == 255 then continue reading run-length
  return runLength + 1; // the length is one more than the sum
 }

Palette ZrleDecoder::readPalette(const vector<unsigned char> &out,
                                 size_t *const readed,
                                 const int paletteSize)
{
  Palette palette(paletteSize);
  for (int i = 0; i < paletteSize; i++) {
    if (*readed + m_bytesPerPixel > out.size())
      throw Exception(_T("error in read palette (zrle-decoder): out of input-buffer"));
    memcpy(&palette[i] + m_numberFirstByte, &out[*readed], m_bytesPerPixel);
    *readed += m_bytesPerPixel;
  }
  return palette;
}

void ZrleDecoder::readRawTile(const vector<unsigned char> &out,
                                size_t *const readed,
                                vector<char> &pixels,
                                const Rect *tileRect)
{
  size_t tileBytesLength = tileRect->area() * m_bytesPerPixel;
  if (*readed + tileBytesLength > out.size())
    throw Exception(_T("error in read raw-tile (zrle-decoder): out of input-buffer"));
  memcpy(&pixels.front(), &out[*readed], tileBytesLength);
  *readed += tileBytesLength;
}

void ZrleDecoder::readSolidTile(const vector<unsigned char> &out,
                                size_t *const readed,
                                vector<char> &pixels,
                                const Rect *tileRect)
{
  size_t tileLength = tileRect->area();
  char solid[4] = {0, 0, 0, 0};
  if (*readed + m_bytesPerPixel > out.size())
    throw Exception(_T("error in solid-tile (zrle-decoder): out of input-buffer"));
  memcpy(solid + m_numberFirstByte, &out[*readed], m_bytesPerPixel);
  *readed += m_bytesPerPixel;
  for (size_t i = 0; i < tileLength; i++)
    memcpy(&pixels[i * m_bytesPerPixel], solid, m_bytesPerPixel);
}

void ZrleDecoder::readPackedPaletteTile(const vector<unsigned char> &out,
                                        size_t *const readed,
                                        vector<char> &pixels,
                                        const Rect *tileRect,
                                        const int type)
{
  int width = tileRect->getWidth();
  int height = tileRect->getHeight();

  // type and palette size is equal
  int paletteSize = type;
  Palette palette = readPalette(out, readed, paletteSize);
  
  int m = 0;
  unsigned char mask = 0;
  unsigned char deltaOffset = 0;
  if (paletteSize == 2) {
    m = (width + 7) / 8;
    mask = 0x01;
    deltaOffset = 1;
  }

  if (paletteSize == 3 || paletteSize == 4) {
    m = (width + 3) / 4;
    mask = 0x03;
    deltaOffset = 2;
  }

  if (paletteSize >= 5 && paletteSize <= 16) {
    m = (width + 1) / 2;
    mask = 0x0F;
    deltaOffset = 4;
  }

  for (int y = 0; y < height; y++) {
    if (m == 0 || *readed + m > out.size())
      throw Exception(_T("error in packed-palette-tile (zrle-decoder): out of input-buffer"));
    // bit lenght of UINT8
    unsigned char offset = 8;
    int index = 0;

    for (int x = 0; x < width; x++) {
      offset -= deltaOffset;
      int color = (out[*readed + index] >> offset) & mask;
      if (offset == 0) {
        offset = 8;
        index++;
      }

      size_t count = y * width + x;
      memcpy(&pixels[count * m_bytesPerPixel], &palette[color], m_bytesPerPixel);
    }
    *readed += m;
  }
}

void ZrleDecoder::readPlainRleTile(const vector<unsigned char> &out,
                                   size_t *const readed,
                                   vector<char> &pixels,
                                   const Rect *tileRect)
{
  size_t tileLength = tileRect->area();
  for (size_t indexPixel = 0; indexPixel < tileLength;) {
    char color[4] = {0, 0, 0, 0};
    if (*readed + m_bytesPerPixel > out.size())
      throw Exception(_T("error in plain-rre-tile in zrle-decoder: out of input-buffer"));
    memcpy(color + m_numberFirstByte, &out[*readed], m_bytesPerPixel);
    *readed += m_bytesPerPixel;

    size_t runLength = readRunLength(out, readed);

    for(size_t i = 0; i < runLength; i++) {
      memcpy(&pixels[(indexPixel + i) * m_bytesPerPixel], color, m_bytesPerPixel);
    }
    indexPixel += runLength;
  } 
}

void ZrleDecoder::readPaletteRleTile(const vector<unsigned char> &out,
                                     size_t *const readed,
                                     vector<char> &pixels,
                                     const Rect *tileRect,
                                     const int type)
{
  size_t tileLength = tileRect->area();

  int paletteSize = type - 128;
  Palette palette = readPalette(out, readed, paletteSize);

  for (size_t indexPixel = 0; indexPixel < tileLength;) {
    unsigned char color;
    if (*readed >= out.size())
      throw Exception(_T("error in palette-rre-tile in zrle-decoder: out of input-buffer"));
    color = out[*readed];
    (*readed)++;

    size_t runLength = 1;
    if (color >= 128) {
      color -= 128;
      runLength = readRunLength(out, readed);
    }

    for(size_t i = 0; i < runLength; i++)
      memcpy(&pixels[(indexPixel + i) * m_bytesPerPixel], &palette[color], m_bytesPerPixel);

    indexPixel += runLength;
  }
}        

void ZrleDecoder::drawTile(FrameBuffer *fb,
                           const Rect *tileRect,
                           const vector<char> *pixels)
{
  int width = tileRect->getWidth();
  int height = tileRect->getHeight();
  size_t fbBytesPerPixel = m_bytesPerPixel;
  if (fbBytesPerPixel == 3)
    fbBytesPerPixel++;

  int tileLength = tileRect->area();

  int x = tileRect->left;
  int y = tileRect->top;
  for (int i = 0; i < tileLength; i++) {
    void *pixelPtr = fb->getBufferPtr(x + i % width, y + i / width);

    memset(pixelPtr, 0, fbBytesPerPixel);
    memcpy(pixelPtr,
           &pixels->operator[](i * m_bytesPerPixel),
           m_bytesPerPixel);
  }
}
