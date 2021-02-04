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
*   Copyright (C) 2015,2016,2017,2018 by Jonathan Naylor G4KLX
*   Copyright (C) 2020 by Bryan Biedenkapp N2PLL
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
#if !defined(__BASE_NETWORK_H__)
#define __BASE_NETWORK_H__

#include "Defines.h"
#include "dmr/DMRDefines.h"
#include "p25/P25Defines.h"
#include "dmr/data/Data.h"
#include "p25/data/LowSpeedData.h"
#include "p25/lc/LC.h"
#include "p25/lc/TSBK.h"
#include "p25/lc/TDULC.h"
#include "p25/Audio.h"
#include "network/UDPSocket.h"
#include "RingBuffer.h"
#include "Timer.h"

#include <string>
#include <cstdint>
#include <random>

// ---------------------------------------------------------------------------
//  Constants
// ---------------------------------------------------------------------------
#define DVM_RAND_MIN 0x00000001
#define DVM_RAND_MAX 0xfffffffe

#define TAG_DMR_DATA            "DMRD"
#define TAG_P25_DATA            "P25D"

#define TAG_MASTER_WL_RID       "MSTWRID"
#define TAG_MASTER_BL_RID       "MSTBRID"
#define TAG_MASTER_ACTIVE_TGS   "MSTTID"
#define TAG_MASTER_DEACTIVE_TGS "MSTDTID"

#define TAG_MASTER_NAK          "MSTNAK"
#define TAG_MASTER_CLOSING      "MSTCL"
#define TAG_MASTER_PONG         "MSTPONG"

#define TAG_REPEATER_LOGIN      "RPTL"
#define TAG_REPEATER_AUTH       "RPTK"
#define TAG_REPEATER_CONFIG     "RPTC"

#define TAG_REPEATER_ACK        "RPTACK"
#define TAG_REPEATER_CLOSING    "RPTCL"
#define TAG_REPEATER_PING       "RPTPING"

#define TAG_REPEATER_LOG        "TRNSLOG"//"RPTALOG"

namespace network
{
    // ---------------------------------------------------------------------------
    //  Constants
    // ---------------------------------------------------------------------------

    // P25 V.24 LDU1 Encapsulation Frames
    const uint8_t LDU1_REC62[] = {
        0x62U, 0x02U, 0x02U, 0x0CU, 0x0BU, 0x12U, 0x64U, 0x00U, 0x00U, 0x80U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
        0x00U, 0x00U, 0x00U, 0x00U, 0x00U };

    const uint8_t LDU1_REC63[] = {
        0x63U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x02U };

    const uint8_t LDU1_REC64[] = {
        0x64U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x02U };

    const uint8_t LDU1_REC65[] = {
        0x65U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x02U };

    const uint8_t LDU1_REC66[] = {
        0x66U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x02U };

    const uint8_t LDU1_REC67[] = {
        0x67U, 0xF0U, 0x9DU, 0x6AU, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x02U };

    const uint8_t LDU1_REC68[] = {
        0x68U, 0x19U, 0xD4U, 0x26U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x02U };

    const uint8_t LDU1_REC69[] = {
        0x69U, 0xE0U, 0xEBU, 0x7BU, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x02U };

    const uint8_t LDU1_REC6A[] = {
        0x6AU, 0x00U, 0x00U, 0x02U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U };

    // P25 V.24 LDU2 Encapsulation Frames
    const uint8_t LDU2_REC6B[] = {
        0x6BU, 0x02U, 0x02U, 0x0CU, 0x0BU, 0x12U, 0x64U, 0x00U, 0x00U, 0x80U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
        0x00U, 0x00U, 0x00U, 0x00U, 0x00U };

    const uint8_t LDU2_REC6C[] = {
        0x6CU, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x02U };

    const uint8_t LDU2_REC6D[] = {
        0x6DU, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x02U };

    const uint8_t LDU2_REC6E[] = {
        0x6EU, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x02U };

    const uint8_t LDU2_REC6F[] = {
        0x6FU, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x02U };

    const uint8_t LDU2_REC70[] = {
        0x70U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x02U };

    const uint8_t LDU2_REC71[] = {
        0x71U, 0xACU, 0xB8U, 0xA4U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x02U };

    const uint8_t LDU2_REC72[] = {
        0x72U, 0x9BU, 0xDCU, 0x75U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x02U };

    const uint8_t LDU2_REC73[] = {
        0x73U, 0x00U, 0x00U, 0x02U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U };

    const uint32_t DATA_PACKET_LENGTH = 8192U;
    const uint32_t DMR_PACKET_SIZE = 55U;
    const uint32_t PACKET_PAD = 8U;

    // ---------------------------------------------------------------------------
    //  Network Peer Connection Status
    // ---------------------------------------------------------------------------

    enum NET_CONN_STATUS {
        // Common States
        NET_STAT_WAITING_CONNECT,
        NET_STAT_WAITING_LOGIN,
        NET_STAT_WAITING_AUTHORISATION,
        NET_STAT_WAITING_CONFIG,
        NET_STAT_RUNNING,

        // Master States
        NET_STAT_RPTL_RECEIVED,
        NET_STAT_CHALLENGE_SENT,

        NET_STAT_MST_RUNNING,

        NET_STAT_INVALID = 0x7FFFFFF
    };

    // ---------------------------------------------------------------------------
    //  Class Declaration
    //      Implements the base networking logic.
    // ---------------------------------------------------------------------------

    class HOST_SW_API BaseNetwork {
    public:
        /// <summary>Initializes a new instance of the BaseNetwork class.</summary>
        BaseNetwork(uint32_t localPort, uint32_t id, bool duplex, bool debug, bool slot1, bool slot2, bool transferActivityLog);
        /// <summary>Finalizes a instance of the BaseNetwork class.</summary>
        virtual ~BaseNetwork();

        /// <summary>Gets the current status of the network.</summary>
        NET_CONN_STATUS getStatus() { return m_status; }

        /// <summary>Reads DMR frame data from the DMR ring buffer.</summary>
        virtual bool readDMR(dmr::data::Data& data);
        /// <summary>Reads P25 frame data from the P25 ring buffer.</summary>
        virtual uint8_t* readP25(bool& ret, p25::lc::LC& control, p25::data::LowSpeedData& lsd, uint8_t& duid, uint32_t& len);

        /// <summary>Writes DMR frame data to the network.</summary>
        virtual bool writeDMR(const dmr::data::Data& data);
        /// <summary>Writes P25 LDU1 frame data to the network.</summary>
        virtual bool writeP25LDU1(const p25::lc::LC& control, const p25::data::LowSpeedData& lsd, const uint8_t* data);
        /// <summary>Writes P25 LDU2 frame data to the network.</summary>
        virtual bool writeP25LDU2(const p25::lc::LC& control, const p25::data::LowSpeedData& lsd, const uint8_t* data);
        /// <summary>Writes P25 TDU frame data to the network.</summary>
        virtual bool writeP25TDU(const p25::lc::LC& control, const p25::data::LowSpeedData& lsd);
        /// <summary>Writes P25 TSDU frame data to the network.</summary>
        virtual bool writeP25TSDU(const p25::lc::TSBK& control, const uint8_t* data);
        /// <summary>Writes P25 PDU frame data to the network.</summary>
        virtual bool writeP25PDU(const uint32_t llId, const uint8_t dataType, const uint8_t* data, const uint32_t len);

        /// <summary>Writes the local activity log to the network.</summary>
        virtual bool writeActLog(const char* message);

        /// <summary>Updates the timer by the passed number of milliseconds.</summary>
        virtual void clock(uint32_t ms) = 0;

        /// <summary>Opens connection to the network.</summary>
        virtual bool open() = 0;

        /// <summary>Closes connection to the network.</summary>
        virtual void close() = 0;

        /// <summary>Resets the DMR ring buffer for the given slot.</summary>
        virtual void resetDMR(uint32_t slotNo);
        /// <summary>Resets the P25 ring buffer.</summary>
        virtual void resetP25();

    protected:
        uint32_t m_id;

        bool m_slot1;
        bool m_slot2;

        bool m_transferActivityLog;

        bool m_duplex;
        bool m_debug;

        UDPSocket m_socket;
        NET_CONN_STATUS m_status;

        Timer m_retryTimer;
        Timer m_timeoutTimer;

        uint8_t* m_buffer;
        uint8_t* m_salt;

        uint32_t* m_streamId;
        uint32_t m_p25StreamId;

        RingBuffer<uint8_t> m_rxDMRData;
        RingBuffer<uint8_t> m_rxP25Data;

        p25::Audio m_audio;

        std::mt19937 m_random;

        /// <summary>Writes DMR frame data to the network.</summary>
        bool writeDMR(const uint32_t id, const uint32_t streamId, const dmr::data::Data& data);
        /// <summary>Writes P25 LDU1 frame data to the network.</summary>
        bool writeP25LDU1(const uint32_t id, const uint32_t streamId, const p25::lc::LC& control, const p25::data::LowSpeedData& lsd, const uint8_t* data);
        /// <summary>Writes P25 LDU2 frame data to the network.</summary>
        bool writeP25LDU2(const uint32_t id, const uint32_t streamId, const p25::lc::LC& control, const p25::data::LowSpeedData& lsd, const uint8_t* data);
        /// <summary>Writes P25 TDU frame data to the network.</summary>
        bool writeP25TDU(const uint32_t id, const uint32_t streamId, const p25::lc::LC& control, const p25::data::LowSpeedData& lsd);
        /// <summary>Writes P25 TSDU frame data to the network.</summary>
        bool writeP25TSDU(const uint32_t id, const uint32_t streamId, const p25::lc::TSBK& control, const uint8_t* data);
        /// <summary>Writes P25 PDU frame data to the network.</summary>
        bool writeP25PDU(const uint32_t id, const uint32_t streamId, const uint32_t llId, const uint8_t dataType, const uint8_t* data, const uint32_t len);

        /// <summary>Writes data to the network.</summary>
        virtual bool write(const uint8_t* data, uint32_t length) = 0;
        /// <summary>Writes data to the network.</summary>
        virtual bool write(const uint8_t* data, uint32_t length, const in_addr& address, uint32_t port);
    };
} // namespace network

#endif // __BASE_NETWORK_H__