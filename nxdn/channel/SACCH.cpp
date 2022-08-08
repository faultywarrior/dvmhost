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
*   Copyright (C) 2018 by Jonathan Naylor G4KLX
*   Copyright (C) 2022 by Bryan Biedenkapp N2PLL
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#include "nxdn/channel/SACCH.h"
#include "nxdn/Convolution.h"
#include "nxdn/NXDNDefines.h"
#include "edac/CRC.h"
#include "Log.h"
#include "Utils.h"

using namespace edac;
using namespace nxdn;
using namespace nxdn::channel;

#include <cstdio>
#include <cassert>
#include <cstring>

// ---------------------------------------------------------------------------
//  Constants
// ---------------------------------------------------------------------------

const uint32_t INTERLEAVE_TABLE[] = {
    0U, 5U, 10U, 15U, 20U, 25U, 30U, 35U, 40U, 45U, 50U, 55U,
    1U, 6U, 11U, 16U, 21U, 26U, 31U, 36U, 41U, 46U, 51U, 56U,
    2U, 7U, 12U, 17U, 22U, 27U, 32U, 37U, 42U, 47U, 52U, 57U,
    3U, 8U, 13U, 18U, 23U, 28U, 33U, 38U, 43U, 48U, 53U, 58U,
    4U, 9U, 14U, 19U, 24U, 29U, 34U, 39U, 44U, 49U, 54U, 59U
};

const uint32_t PUNCTURE_LIST[] = { 5U, 11U, 17U, 23U, 29U, 35U, 41U, 47U, 53U, 59U, 65U, 71U };

// ---------------------------------------------------------------------------
//  Public Class Members
// ---------------------------------------------------------------------------

/// <summary>
/// Initializes a copy instance of the SACCH class.
/// </summary>
/// <param name="data"></param>
SACCH::SACCH(const SACCH& data) :
    m_verbose(false),
    m_ran(0U),
    m_structure(0U),
    m_data(NULL)
{
    m_data = new uint8_t[5U];
    ::memcpy(m_data, data.m_data, 5U);

    m_ran = m_data[0U] & 0x3FU;
    m_structure = (m_data[0U] >> 6) & 0x03U;
}

/// <summary>
/// Initializes a new instance of the SACCH class.
/// </summary>
SACCH::SACCH() :
    m_verbose(false),
    m_ran(0U),
    m_structure(0U),
    m_data(NULL)
{
    m_data = new uint8_t[5U];
}

/// <summary>
/// Finalizes a instance of SACCH class.
/// </summary>
SACCH::~SACCH()
{
    delete[] m_data;
}

/// <summary>
/// Equals operator.
/// </summary>
/// <param name="data"></param>
/// <returns></returns>
SACCH& SACCH::operator=(const SACCH& data)
{
    if (&data != this) {
        ::memcpy(m_data, data.m_data, 5U);

        m_verbose = data.m_verbose;

        m_ran = m_data[0U] & 0x3FU;
        m_structure = (m_data[0U] >> 6) & 0x03U;
    }

    return *this;
}

/// <summary>
/// Decode a slow associated control channel.
/// </summary>
/// <param name="data"></param>
/// <returns>True, if SACCH was decoded, otherwise false.</returns>
bool SACCH::decode(const uint8_t* data)
{
    assert(data != NULL);

    uint8_t buffer[NXDN_SACCH_LENGTH_BYTES + 1U];
    for (uint32_t i = 0U; i < NXDN_SACCH_LENGTH_BITS; i++) {
        uint32_t n = INTERLEAVE_TABLE[i] + NXDN_FSW_LENGTH_BITS + NXDN_LICH_LENGTH_BITS;
        bool b = READ_BIT(data, n);
        WRITE_BIT(buffer, i, b);
    }

#if DEBUG_NXDN_SACCH
    Utils::dump(2U, "SACCH::decode(), SACCH Raw", buffer, NXDN_SACCH_LENGTH_BYTES + 1U);
#endif

    // deinterleave
    uint8_t interleave[90U];
    uint32_t n = 0U;
    uint32_t index = 0U;
    for (uint32_t i = 0U; i < NXDN_SACCH_LENGTH_BITS; i++) {
        if (n == PUNCTURE_LIST[index]) {
            interleave[n++] = 1U;
            index++;
        }

        bool b = READ_BIT(buffer, i);
        interleave[n++] = b ? 2U : 0U;
    }

    for (uint32_t i = 0U; i < 8U; i++) {
        interleave[n++] = 0U;
    }

    // decode convolution
    Convolution conv;
    conv.start();

    n = 0U;
    for (uint32_t i = 0U; i < 40U; i++) {
        uint8_t s0 = interleave[n++];
        uint8_t s1 = interleave[n++];

        conv.decode(s0, s1);
    }

    conv.chainback(m_data, 36U);

    if (m_verbose) {
        Utils::dump(2U, "Decoded SACCH", m_data, 5U);
    }

    // check CRC-6
    bool ret = CRC::checkCRC6(m_data, 26U);
    if (!ret) {
        LogError(LOG_NXDN, "SACCH::decode(), failed CRC-6 check");
        return false;
    }

    m_ran = m_data[0U] & 0x3FU;
    m_structure = (m_data[0U] >> 6) & 0x03U;

    return true;
}

/// <summary>
/// Encode a slow associated control channel.
/// </summary>
/// <param name="data"></param>
void SACCH::encode(uint8_t* data) const
{
    assert(data != NULL);

	m_data[0U] &= 0xC0U;
	m_data[0U] |= m_ran;

	m_data[0U] &= 0x3FU;
	m_data[0U] |= (m_structure << 6) & 0xC0U;

    if (m_verbose) {
        Utils::dump(2U, "Encoded SACCH", m_data, 5U);
    }

    uint8_t buffer[5U];
    ::memset(buffer, 0x00U, 5U);

    for (uint32_t i = 0U; i < 26U; i++) {
        bool b = READ_BIT(m_data, i);
        WRITE_BIT(buffer, i, b);
    }

    CRC::addCRC6(buffer, 26U);

    // encode convolution
    uint8_t convolution[9U];
    Convolution conv;
    conv.encode(buffer, convolution, 36U);

#if DEBUG_NXDN_SACCH
    Utils::dump(2U, "SACCH::encode(), SACCH Convolution", convolution, 9U);
#endif

    // interleave and puncture
    uint8_t raw[8U];
    uint32_t n = 0U;
    uint32_t index = 0U;
    for (uint32_t i = 0U; i < 72U; i++) {
        if (i != PUNCTURE_LIST[index]) {
            bool b = READ_BIT(convolution, i);
            WRITE_BIT(raw, n, b);
            n++;
        } else {
            index++;
        }
    }

    for (uint32_t i = 0U; i < NXDN_SACCH_LENGTH_BITS; i++) {
        uint32_t n = INTERLEAVE_TABLE[i] + NXDN_FSW_LENGTH_BITS + NXDN_LICH_LENGTH_BITS;
        bool b = READ_BIT(raw, i);
        WRITE_BIT(data, n, b);
    }

#if DEBUG_NXDN_SACCH
    Utils::dump(2U, "SACCH::encode(), SACCH Interleave", raw, 8U);
#endif
}

/// <summary>
/// Gets the raw SACCH data.
/// </summary>
/// <param name="data"></param>
void SACCH::getData(uint8_t* data) const
{
    assert(data != NULL);

    uint32_t offset = 8U;
    for (uint32_t i = 0U; i < 18U; i++, offset++) {
        bool b = READ_BIT(m_data, offset);
        WRITE_BIT(data, i, b);
    }
}

/// <summary>
/// Sets the raw SACCH data.
/// </summary>
/// <param name="data"></param>
void SACCH::setData(const uint8_t* data)
{
    assert(data != NULL);

    uint32_t offset = 8U;
    for (uint32_t i = 0U; i < 18U; i++, offset++) {
        bool b = READ_BIT(data, i);
        WRITE_BIT(m_data, offset, b);
    }

    m_ran = m_data[0U] & 0x3FU;
    m_structure = (m_data[0U] >> 6) & 0x03U;
}