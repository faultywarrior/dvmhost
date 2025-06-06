// SPDX-License-Identifier: GPL-2.0-only
/*
 * Digital Voice Modem - Common Library
 * GPLv2 Open Source. Use is subject to license terms.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 *  Copyright (C) 2022 Bryan Biedenkapp, N2PLL
 *
 */
/**
 * @file MBT_IOSP_GRP_AFF.h
 * @ingroup p25_ambt
 * @file MBT_IOSP_GRP_AFF.cpp
 * @ingroup p25_ambt
 */
#if !defined(__P25_LC_TSBK__MBT_IOSP_GRP_AFF_H__)
#define  __P25_LC_TSBK__MBT_IOSP_GRP_AFF_H__

#include "common/Defines.h"
#include "common/p25/lc/AMBT.h"

namespace p25
{
    namespace lc
    {
        namespace tsbk
        {
            // ---------------------------------------------------------------------------
            //  Class Declaration
            // ---------------------------------------------------------------------------

            /**
             * @brief Implements GRP AFF REQ - Group Affiliation Request (ISP) and
             *  GRP AFF RSP - Group Affiliation Response (OSP)
             * @ingroup p25_ambt
             */
            class HOST_SW_API MBT_IOSP_GRP_AFF : public AMBT {
            public:
                /**
                 * @brief Initializes a new instance of the MBT_IOSP_GRP_AFF class.
                 */
                MBT_IOSP_GRP_AFF();

                /**
                 * @brief Decode a alternate trunking signalling block.
                 * @param[in] dataHeader P25 PDU data header
                 * @param[in] blocks P25 PDU data blocks
                 * @returns bool True, if AMBT decoded, otherwise false.
                 */
                bool decodeMBT(const data::DataHeader& dataHeader, const data::DataBlock* blocks) override;
                /**
                 * @brief Encode a alternate trunking signalling block.
                 * @param[out] dataHeader P25 PDU data header
                 * @param[out] pduUserData P25 PDU user data
                 */
                void encodeMBT(data::DataHeader& dataHeader, uint8_t* pduUserData) override;

                /**
                 * @brief Returns a string that represents the current AMBT.
                 * @returns std::string String representation of the AMBT.
                 */
                std::string toString(bool isp = false) override;

            public:
                /**
                 * @brief Announcement group.
                 */
                DECLARE_PROPERTY(uint32_t, announceGroup, AnnounceGroup);
            };
        } // namespace tsbk
    } // namespace lc
} // namespace p25

#endif // __P25_LC_TSBK__MBT_IOSP_GRP_AFF_H__
