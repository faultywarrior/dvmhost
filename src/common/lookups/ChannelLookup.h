// SPDX-License-Identifier: GPL-2.0-only
/**
* Digital Voice Modem - Common Library
* GPLv2 Open Source. Use is subject to license terms.
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* @package DVM / Common Library
* @license GPLv2 License (https://opensource.org/licenses/GPL-2.0)
*
*  Copyright (C) 2024-2025 Bryan Biedenkapp, N2PLL
*
*/
/**
 * @defgroup lookups_ch RF Channel Lookups
 * @brief Implementation for RF channel lookup tables.
 * @ingroup lookups
 * 
 * @file ChannelLookup.h
 * @ingroup lookups_ch
 * @file ChannelLookup.cpp
 * @ingroup lookups_ch
 */
#if !defined(__CHANNEL_LOOKUP_H__)
#define __CHANNEL_LOOKUP_H__

#include "common/Defines.h"
#include "common/concurrent/vector.h"
#include "common/concurrent/unordered_map.h"
#include "common/Timer.h"

#include <cstdio>

namespace lookups
{
    // ---------------------------------------------------------------------------
    //  Class Declaration
    // ---------------------------------------------------------------------------

    /**
     * @brief Represents voice channel data.
     * @ingroup lookups_ch
     */
    class HOST_SW_API VoiceChData {
    public:
        /**
         * @brief Initializes a new instance of the VoiceChData class.
         */
        VoiceChData() :
            m_chId(0U),
            m_chNo(0U),
            m_rxChId(0xFFU),
            m_rxChNo(0xFFFFU),
            m_address(),
            m_port(),
            m_password(),
            m_ssl()
        {
            /* stub */
        }
        /**
         * @brief Initializes a new instance of the VoiceChData class.
         * @param chId Voice Channel Identity.
         * @param chNo Voice Channel Number.
         * @param address RPC/REST API Address.
         * @param port RPC/REST API Port.
         * @param password RPC/REST API Password.
         * @param ssl Flag indicating REST is using SSL.
         */
        VoiceChData(uint8_t chId, uint32_t chNo, std::string address, uint16_t port, std::string password, bool ssl = false) :
            m_chId(chId),
            m_chNo(chNo),
            m_rxChId(0xFFU),
            m_rxChNo(0xFFFFU),
            m_address(address),
            m_port(port),
            m_password(password),
            m_ssl(ssl)
        {
            /* stub */
        }
        /**
         * @brief Initializes a new instance of the VoiceChData class.
         * @param chId Voice Channel Identity.
         * @param chNo Voice Channel Number.
         * @param chId Voice Rx Channel Identity.
         * @param chNo Voice Rx Channel Number.
         * @param address RPC/REST API Address.
         * @param port RPC/REST API Port.
         * @param password RPC/REST API Password.
         * @param ssl Flag indicating REST is using SSL.
         */
        VoiceChData(uint8_t chId, uint32_t chNo, uint8_t rxChId, uint32_t rxChNo, std::string address, uint16_t port, std::string password, bool ssl = false) :
            m_chId(chId),
            m_chNo(chNo),
            m_rxChId(rxChId),
            m_rxChNo(rxChNo),
            m_address(address),
            m_port(port),
            m_password(password),
            m_ssl(ssl)
        {
            /* stub */
        }

        /**
         * @brief Equals operator.
         * @param data Instance of VoiceChData to copy.
         */
        VoiceChData& operator=(const VoiceChData& data)
        {
            if (this != &data) {
                m_chId = data.m_chId;
                m_chNo = data.m_chNo;
                m_rxChId = data.m_rxChId;
                m_rxChNo = data.m_rxChNo;
                m_address = data.m_address;
                m_port = data.m_port;
                m_password = data.m_password;
                m_ssl = data.m_ssl;
            }

            return *this;
        }

        /**
         * @brief Helper to determine if the channel identity is valid.
         * @returns True, if channel identity is valid, otherwise false.
         */
        bool isValidChId() const { return m_chId != 0U; }
        /**
         * @brief Helper to determine if the channel is valid.
         * @returns True, if channel is valid, otherwise false.
         */
        bool isValidCh() const { return m_chNo != 0U; }
        /**
         * @brief Helper to determine if the Rx channel identity is valid.
         * @returns True, if channel identity is valid, otherwise false.
         */
        bool isValidRxChId() const { return m_rxChId != 0U && m_rxChId != 0xFFU; }
        /**
         * @brief Helper to determine if the Rx channel is valid.
         * @returns True, if channel is valid, otherwise false.
         */
        bool isValidRxCh() const { return m_rxChNo != 0U && m_rxChNo != 0xFFFFU; }

        /**
         * @brief Helper to determine if the channel is explicitly defined with independant Rx and Tx channels.
         * @returns True, if channel is explicit, otherwise false.
         */
        bool isExplicitCh() const { return (m_rxChId != 0xFFU && m_rxChId > 0U) && (m_rxChNo != 0xFFFFU && m_rxChNo > 0U); }

    public:
        /**
         * @brief Voice Channel Identity.
         */
        DECLARE_RO_PROPERTY_PLAIN(uint8_t, chId);
        /**
         * @brief Voice Channel Number.
         */
        DECLARE_RO_PROPERTY_PLAIN(uint32_t, chNo);
        /**
         * @brief Voice Rx Channel Identity.
         */
        DECLARE_RO_PROPERTY_PLAIN(uint8_t, rxChId);
        /**
         * @brief Voice Rx Channel Number.
         */
        DECLARE_RO_PROPERTY_PLAIN(uint32_t, rxChNo);
        /**
         * @brief RPC/REST API Address.
         */
        DECLARE_PROPERTY_PLAIN(std::string, address);
        /**
         * @brief RPC/REST API Port.
         */
        DECLARE_PROPERTY_PLAIN(uint16_t, port);
        /**
         * @brief RPC/REST API Password.
         */
        DECLARE_RO_PROPERTY_PLAIN(std::string, password);
        /**
         * @brief Flag indicating REST is using SSL.
         */
        DECLARE_PROPERTY_PLAIN(bool, ssl);
    };

    // ---------------------------------------------------------------------------
    //  Class Declaration
    // ---------------------------------------------------------------------------

    /**
     * @brief Implements a lookup table class that contains RF channel information.
     * @ingroup lookups_ch
     */
    class HOST_SW_API ChannelLookup {
    public:
        /**
         * @brief Initializes a new instance of the ChannelLookup class.
         */
        ChannelLookup();
        /**
         * @brief Finalizes a instance of the ChannelLookup class.
         */
        virtual ~ChannelLookup();

        /** @name RF Channel Data */
        /**
         * @brief Gets the count of RF channel data.
         * @returns uint8_t Total count of RF channel data.
         */
        uint8_t rfChDataSize() const { return m_rfChDataTable.size(); }
        /**
         * @brief Gets the RF channel data table.
         * @returns std::unordered_map<uint32_t, VoiceChData> RF channel data table.
         */
        std::unordered_map<uint32_t, VoiceChData> rfChDataTable() const { return m_rfChDataTable.get(); }
        /**
         * @brief Helper to set RF channel data.
         * @param chData RF Channel data table.
         */
        void setRFChData(const std::unordered_map<uint32_t, VoiceChData>& chData) { m_rfChDataTable = chData; }
        /**
         * @brief Helper to set RF channel data.
         * @param chNo Channel Number.
         * @param chData Channel Data.
         */
        void setRFChData(uint32_t chNo, VoiceChData chData) { m_rfChDataTable[chNo] = chData; }
        /**
         * @brief Helper to get RF channel data.
         * @param chNo Channel Number.
         * @returns VoiceChData Channel Data.
         */
        VoiceChData getRFChData(uint32_t chNo) const;
        /**
         * @brief Helper to get first available channel number.
         * @returns uint32_t First available channel number.
         */
        uint32_t getFirstRFChannel() const { return m_rfChTable.at(0); }

        /**
         * @brief Gets the count of RF channels.
         * @returns uint8_t Total count of RF channels.
         */
        uint8_t rfChSize() const { return m_rfChTable.size(); }
        /**
         * @brief Gets the RF channels table.
         * @returns std::vector<uint32_t> RF channel table.
         */
        std::vector<uint32_t> rfChTable() const { return m_rfChTable.get(); }
        /**
         * @brief Helper to add a RF channel.
         * @param chNo Channel Number.
         * @param force Flag indicating the channel should be forcibly added.
         * @returns bool True, if channel added, otherwise false.
         */
        bool addRFCh(uint32_t chNo, bool force = false);
        /**
         * @brief Helper to remove a RF channel.
         * @param chNo Channel Number.
         * @returns bool True, if channel remove, otherwise false.
         */
        bool removeRFCh(uint32_t chNo);
        /**
         * @brief Helper to determine if there are any RF channels available..
         * @returns bool True, if any RF channels are available for use, otherwise false.
         */
        bool isRFChAvailable() const { return !m_rfChTable.empty(); }
        /** @} */

    private:
        concurrent::vector<uint32_t> m_rfChTable;
        concurrent::unordered_map<uint32_t, VoiceChData> m_rfChDataTable;
    };
} // namespace lookups

#endif // __CHANNEL_LOOKUP_H__
