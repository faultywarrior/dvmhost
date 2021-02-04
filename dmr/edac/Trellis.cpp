/**
* Digital Voice Modem - Host Software
* GPLv2 Open Source. Use is subject to license terms.
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* @package DVM / Host Software
*
*/
//
// Based on code from the MMDVMHost project. (https://github.com/g4klx/MMDVMHost)
// Licensed under the GPLv2 License (https://opensource.org/licenses/GPL-2.0)
//
/*
*   Copyright (C) 2016 by Jonathan Naylor, G4KLX
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; version 2 of the License.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*/
#include "Defines.h"
#include "dmr/edac/Trellis.h"

using namespace dmr::edac;

#include <cstdio>
#include <cassert>

// ---------------------------------------------------------------------------
//  Constants
// ---------------------------------------------------------------------------

const uint32_t INTERLEAVE_TABLE[] = {
    0U, 1U, 8U,   9U, 16U, 17U, 24U, 25U, 32U, 33U, 40U, 41U, 48U, 49U, 56U, 57U, 64U, 65U, 72U, 73U, 80U, 81U, 88U, 89U, 96U, 97U,
    2U, 3U, 10U, 11U, 18U, 19U, 26U, 27U, 34U, 35U, 42U, 43U, 50U, 51U, 58U, 59U, 66U, 67U, 74U, 75U, 82U, 83U, 90U, 91U,
    4U, 5U, 12U, 13U, 20U, 21U, 28U, 29U, 36U, 37U, 44U, 45U, 52U, 53U, 60U, 61U, 68U, 69U, 76U, 77U, 84U, 85U, 92U, 93U,
    6U, 7U, 14U, 15U, 22U, 23U, 30U, 31U, 38U, 39U, 46U, 47U, 54U, 55U, 62U, 63U, 70U, 71U, 78U, 79U, 86U, 87U, 94U, 95U };

const uint8_t ENCODE_TABLE[] = {
    0U,  8U, 4U, 12U, 2U, 10U, 6U, 14U,
    4U, 12U, 2U, 10U, 6U, 14U, 0U,  8U,
    1U,  9U, 5U, 13U, 3U, 11U, 7U, 15U,
    5U, 13U, 3U, 11U, 7U, 15U, 1U,  9U,
    3U, 11U, 7U, 15U, 1U,  9U, 5U, 13U,
    7U, 15U, 1U,  9U, 5U, 13U, 3U, 11U,
    2U, 10U, 6U, 14U, 0U,  8U, 4U, 12U,
    6U, 14U, 0U,  8U, 4U, 12U, 2U, 10U };

// ---------------------------------------------------------------------------
//  Public Class Members
// ---------------------------------------------------------------------------
/// <summary>
/// Initializes a new instance of the Trellis class.
/// </summary>
Trellis::Trellis()
{
    /* stub */
}

/// <summary>
/// Finalizes a instance of the Trellis class.
/// </summary>
Trellis::~Trellis()
{
    /* stub */
}

/// <summary>
/// Decodes 3/4 rate Trellis.
/// </summary>
/// <param name="data"></param>
/// <param name="payload"></param>
/// <returns></returns>
bool Trellis::decode(const uint8_t* data, uint8_t* payload)
{
    assert(data != NULL);
    assert(payload != NULL);

    int8_t dibits[98U];
    deinterleave(data, dibits);

    uint8_t points[49U];
    dibitsToPoints(dibits, points);

    // Check the original code
    uint8_t tribits[49U];
    uint32_t failPos = checkCode(points, tribits);
    if (failPos == 999U) {
        tribitsToBits(tribits, payload);
        return true;
    }

    uint8_t savePoints[49U];
    for (uint32_t i = 0U; i < 49U; i++)
        savePoints[i] = points[i];

    bool ret = fixCode(points, failPos, payload);
    if (ret)
        return true;

    if (failPos == 0U)
        return false;

    // Backtrack one place for a last go
    return fixCode(savePoints, failPos - 1U, payload);
}

/// <summary>
/// Encodes 3/4 rate Trellis.
/// </summary>
/// <param name="payload"></param>
/// <param name="data"></param>
void Trellis::encode(const uint8_t* payload, uint8_t* data)
{
    assert(payload != NULL);
    assert(data != NULL);

    uint8_t tribits[49U];
    bitsToTribits(payload, tribits);

    uint8_t points[49U];
    uint8_t state = 0U;

    for (uint32_t i = 0U; i < 49U; i++) {
        uint8_t tribit = tribits[i];

        points[i] = ENCODE_TABLE[state * 8U + tribit];

        state = tribit;
    }

    int8_t dibits[98U];
    pointsToDibits(points, dibits);

    interleave(dibits, data);
}

// ---------------------------------------------------------------------------
//  Private Class Members
// ---------------------------------------------------------------------------
/// <summary>
///
/// </summary>
/// <param name="data"></param>
/// <param name="dibits"></param>
void Trellis::deinterleave(const uint8_t* data, int8_t* dibits) const
{
    for (uint32_t i = 0U; i < 98U; i++) {
        uint32_t n = i * 2U + 0U;
        if (n >= 98U) n += 68U;
        bool b1 = READ_BIT(data, n) != 0x00U;

        n = i * 2U + 1U;
        if (n >= 98U) n += 68U;
        bool b2 = READ_BIT(data, n) != 0x00U;

        int8_t dibit;
        if (!b1 && b2)
            dibit = +3;
        else if (!b1 && !b2)
            dibit = +1;
        else if (b1 && !b2)
            dibit = -1;
        else
            dibit = -3;

        n = INTERLEAVE_TABLE[i];
        dibits[n] = dibit;
    }
}

/// <summary>
///
/// </summary>
/// <param name="dibits"></param>
/// <param name="data"></param>
void Trellis::interleave(const int8_t* dibits, uint8_t* data) const
{
    for (uint32_t i = 0U; i < 98U; i++) {
        uint32_t n = INTERLEAVE_TABLE[i];

        bool b1, b2;
        switch (dibits[n]) {
            case +3:
                b1 = false;
                b2 = true;
                break;
            case +1:
                b1 = false;
                b2 = false;
                break;
            case -1:
                b1 = true;
                b2 = false;
                break;
            default:
                b1 = true;
                b2 = true;
                break;
        }

        n = i * 2U + 0U;
        if (n >= 98U) n += 68U;
        WRITE_BIT(data, n, b1);

        n = i * 2U + 1U;
        if (n >= 98U) n += 68U;
        WRITE_BIT(data, n, b2);
    }
}

/// <summary>
///
/// </summary>
/// <param name="dibits"></param>
/// <param name="points"></param>
void Trellis::dibitsToPoints(const int8_t* dibits, uint8_t* points) const
{
    for (uint32_t i = 0U; i < 49U; i++) {
        if (dibits[i * 2U + 0U] == +1 && dibits[i * 2U + 1U] == -1)
            points[i] = 0U;
        else if (dibits[i * 2U + 0U] == -1 && dibits[i * 2U + 1U] == -1)
            points[i] = 1U;
        else if (dibits[i * 2U + 0U] == +3 && dibits[i * 2U + 1U] == -3)
            points[i] = 2U;
        else if (dibits[i * 2U + 0U] == -3 && dibits[i * 2U + 1U] == -3)
            points[i] = 3U;
        else if (dibits[i * 2U + 0U] == -3 && dibits[i * 2U + 1U] == -1)
            points[i] = 4U;
        else if (dibits[i * 2U + 0U] == +3 && dibits[i * 2U + 1U] == -1)
            points[i] = 5U;
        else if (dibits[i * 2U + 0U] == -1 && dibits[i * 2U + 1U] == -3)
            points[i] = 6U;
        else if (dibits[i * 2U + 0U] == +1 && dibits[i * 2U + 1U] == -3)
            points[i] = 7U;
        else if (dibits[i * 2U + 0U] == -3 && dibits[i * 2U + 1U] == +3)
            points[i] = 8U;
        else if (dibits[i * 2U + 0U] == +3 && dibits[i * 2U + 1U] == +3)
            points[i] = 9U;
        else if (dibits[i * 2U + 0U] == -1 && dibits[i * 2U + 1U] == +1)
            points[i] = 10U;
        else if (dibits[i * 2U + 0U] == +1 && dibits[i * 2U + 1U] == +1)
            points[i] = 11U;
        else if (dibits[i * 2U + 0U] == +1 && dibits[i * 2U + 1U] == +3)
            points[i] = 12U;
        else if (dibits[i * 2U + 0U] == -1 && dibits[i * 2U + 1U] == +3)
            points[i] = 13U;
        else if (dibits[i * 2U + 0U] == +3 && dibits[i * 2U + 1U] == +1)
            points[i] = 14U;
        else if (dibits[i * 2U + 0U] == -3 && dibits[i * 2U + 1U] == +1)
            points[i] = 15U;
    }
}

/// <summary>
///
/// </summary>
/// <param name="points"></param>
/// <param name="dibits"></param>
void Trellis::pointsToDibits(const uint8_t* points, int8_t* dibits) const
{
    for (uint32_t i = 0U; i < 49U; i++) {
        switch (points[i]) {
            case 0U:
                dibits[i * 2U + 0U] = +1;
                dibits[i * 2U + 1U] = -1;
                break;
            case 1U:
                dibits[i * 2U + 0U] = -1;
                dibits[i * 2U + 1U] = -1;
                break;
            case 2U:
                dibits[i * 2U + 0U] = +3;
                dibits[i * 2U + 1U] = -3;
                break;
            case 3U:
                dibits[i * 2U + 0U] = -3;
                dibits[i * 2U + 1U] = -3;
                break;
            case 4U:
                dibits[i * 2U + 0U] = -3;
                dibits[i * 2U + 1U] = -1;
                break;
            case 5U:
                dibits[i * 2U + 0U] = +3;
                dibits[i * 2U + 1U] = -1;
                break;
            case 6U:
                dibits[i * 2U + 0U] = -1;
                dibits[i * 2U + 1U] = -3;
                break;
            case 7U:
                dibits[i * 2U + 0U] = +1;
                dibits[i * 2U + 1U] = -3;
                break;
            case 8U:
                dibits[i * 2U + 0U] = -3;
                dibits[i * 2U + 1U] = +3;
                break;
            case 9U:
                dibits[i * 2U + 0U] = +3;
                dibits[i * 2U + 1U] = +3;
                break;
            case 10U:
                dibits[i * 2U + 0U] = -1;
                dibits[i * 2U + 1U] = +1;
                break;
            case 11U:
                dibits[i * 2U + 0U] = +1;
                dibits[i * 2U + 1U] = +1;
                break;
            case 12U:
                dibits[i * 2U + 0U] = +1;
                dibits[i * 2U + 1U] = +3;
                break;
            case 13U:
                dibits[i * 2U + 0U] = -1;
                dibits[i * 2U + 1U] = +3;
                break;
            case 14U:
                dibits[i * 2U + 0U] = +3;
                dibits[i * 2U + 1U] = +1;
                break;
            default:
                dibits[i * 2U + 0U] = -3;
                dibits[i * 2U + 1U] = +1;
                break;
        }
    }
}

/// <summary>
///
/// </summary>
/// <param name="payload"></param>
/// <param name="tribits"></param>
void Trellis::bitsToTribits(const uint8_t* payload, uint8_t* tribits) const
{
    for (uint32_t i = 0U; i < 48U; i++) {
        uint32_t n = 143U - i * 3U;

        bool b1 = READ_BIT(payload, n) != 0x00U;
        n--;
        bool b2 = READ_BIT(payload, n) != 0x00U;
        n--;
        bool b3 = READ_BIT(payload, n) != 0x00U;

        uint8_t tribit = 0U;
        tribit |= b1 ? 4U : 0U;
        tribit |= b2 ? 2U : 0U;
        tribit |= b3 ? 1U : 0U;

        tribits[i] = tribit;
    }

    tribits[48U] = 0U;
}

/// <summary>
///
/// </summary>
/// <param name="tribits"></param>
/// <param name="payload"></param>
void Trellis::tribitsToBits(const uint8_t* tribits, uint8_t* payload) const
{
    for (uint32_t i = 0U; i < 48U; i++) {
        uint8_t tribit = tribits[i];

        bool b1 = (tribit & 0x04U) == 0x04U;
        bool b2 = (tribit & 0x02U) == 0x02U;
        bool b3 = (tribit & 0x01U) == 0x01U;

        uint32_t n = 143U - i * 3U;

        WRITE_BIT(payload, n, b1);
        n--;
        WRITE_BIT(payload, n, b2);
        n--;
        WRITE_BIT(payload, n, b3);
    }
}

/// <summary>
///
/// </summary>
/// <param name="points"></param>
/// <param name="failPos"></param>
/// <param name="payload"></param>
/// <returns></returns>
bool Trellis::fixCode(uint8_t* points, uint32_t failPos, uint8_t* payload) const
{
    for (unsigned j = 0U; j < 20U; j++) {
        uint32_t bestPos = 0U;
        uint32_t bestVal = 0U;

        for (uint32_t i = 0U; i < 16U; i++) {
            points[failPos] = i;

            uint8_t tribits[49U];
            uint32_t pos = checkCode(points, tribits);
            if (pos == 999U) {
                tribitsToBits(tribits, payload);
                return true;
            }

            if (pos > bestPos) {
                bestPos = pos;
                bestVal = i;
            }
        }

        points[failPos] = bestVal;
        failPos = bestPos;
    }

    return false;
}

/// <summary>
///
/// </summary>
/// <param name="points"></param>
/// <param name="tribits"></param>
/// <returns></returns>
uint32_t Trellis::checkCode(const uint8_t* points, uint8_t* tribits) const
{
    uint8_t state = 0U;

    for (uint32_t i = 0U; i < 49U; i++) {
        tribits[i] = 9U;

        for (uint32_t j = 0U; j < 8U; j++) {
            if (points[i] == ENCODE_TABLE[state * 8U + j]) {
                tribits[i] = j;
                break;
            }
        }

        if (tribits[i] == 9U)
            return i;

        state = tribits[i];
    }

    if (tribits[48U] != 0U)
        return 48U;

    return 999U;
}