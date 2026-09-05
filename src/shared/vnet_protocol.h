#pragma once
#include <string>

// ============================================================
// VNET PROTOCOL - SHARED BETWEEN CLIENT & SERVER
// ============================================================

// Packet command strings (matching your server's cmd parsing)
namespace VNetCmd {
    const char* const PING = "PING";
    const char* const GET = "GET";
    const char* const CHAT = "CHAT";
    const char* const WHISPER = "WHISPER";
    const char* const DOS = "DOS";
    const char* const SPIKE = "SPIKE";
    const char* const SNOOP = "SNOOP";
    const char* const REDIRECT = "REDIRECT";
    const char* const OVERLOAD = "OVERLOAD";
    const char* const WIN = "WIN";
    const char* const TAKEOVER = "TAKEOVER";
    const char* const PROBE = "PROBE";
    const char* const SCAN = "SCAN";
    const char* const SATSCAN = "SATSCAN";
    const char* const SNIFFER_ADD = "SNIFFER_ADD";
    const char* const SNIFFER_STATUS = "SNIFFER_STATUS";
    const char* const PATCH = "PATCH";
    const char* const DECOY = "DECOY";
    const char* const PROXY = "PROXY";
    const char* const CAT = "CAT";
    const char* const LS = "LS";
    const char* const CRACK = "CRACK";
    const char* const MINE_EVENT = "MINE_EVENT";
    const char* const PAY_TRANSFER = "PAY_TRANSFER";
    const char* const PAY_CLAIM = "PAY_CLAIM";
    const char* const GET_ESCROWS = "GET_ESCROWS";
    const char* const ION_STRIKE = "ION_STRIKE";
    const char* const NETSCAN = "NETSCAN";
    const char* const TRACE_BUST = "TRACE_BUST";
    const char* const SCAN_REQ = "SCAN:REQ";
    const char* const SATSCAN_REQ = "SATSCAN:REQ";
    const char* const VDEC_DECRYPT = "VDEC_DECRYPT";
    const char* const VDEC_ENCRYPT = "VDEC_ENCRYPT";
    const char* const VDEC_HASH = "VDEC_HASH";
    const char* const VDEC_KEY_STATUS = "VDEC_KEY_STATUS";
    const char* const VDEC_MINIGAME = "VDEC_MINIGAME";
}

// Server response prefixes
namespace VNetResp {
    const char* const KEY_SYNC = "KEY_SYNC:";
    const char* const NEW_BLOCKS = "NEW_BLOCKS:";
    const char* const FEED_EVENT = "FEED_EVENT:";
    const char* const DECOY_TRIPPED = "DECOY_TRIPPED:";
    const char* const DOS_DROP = "DOS_DROP:";
    const char* const DOS_DROP_DIR = "DOS_DROP_DIR:";
    const char* const PATCH_SUCCESS = "PATCH_SUCCESS:";
    const char* const EXPLOIT_WINNER = "EXPLOIT:WINNER:";
    const char* const EXPLOIT_BOT_STALK = "EXPLOIT:BOT_STALK";
    const char* const EXPLOIT_FEDERAL_RAID = "EXPLOIT:FEDERAL_RAID";
    const char* const EXPLOIT_TRACE_SPIKE = "EXPLOIT:TRACE_SPIKE";
    const char* const EXPLOIT_REDIRECT = "EXPLOIT:REDIRECT:";
    const char* const EXPLOIT_DOS = "EXPLOIT:DOS";
    const char* const EXPLOIT_SITE_OVERLOADED = "EXPLOIT:SITE_OVERLOADED:";
    const char* const EXPLOIT_BRAINDEAD = "EXPLOIT:BRAINDEAD:";
    const char* const VFS_DATA = "VFS_DATA:";
    const char* const VFS_LIST = "VFS_LIST:";
    const char* const ESCROW_LIST = "ESCROW_LIST:";
    const char* const PAY_CLAIM_SUCCESS = "PAY_CLAIM_SUCCESS:";
    const char* const PAY_CLAIM_FAILED = "PAY_CLAIM_FAILED:";
    const char* const SCAN_RESULT = "SCAN_RESULT:";
    const char* const SATSCAN_RES = "SATSCAN_RES:";
    const char* const SNIFFER_ADD_ACK = "SNIFFER_ADD_ACK:";
    const char* const WHISPER_IN = "WHISPER_IN:";
    const char* const VDEC_DECRYPT_RES = "VDEC_DECRYPT_RES:";
    const char* const VDEC_ENCRYPT_RES = "VDEC_ENCRYPT_RES:";
    const char* const VDEC_HASH_RES = "VDEC_HASH_RES:";
    const char* const VDEC_KEY_STATUS_RES = "VDEC_KEY_STATUS_RES:";
    const char* const VDEC_MINIGAME_RES = "VDEC_MINIGAME_RES:";
}

// ============================================================
// HELPER FUNCTIONS
// ============================================================

// Build a command packet: "CMD:payload"
inline std::string BuildPacket(const std::string& cmd, const std::string& payload = "") {
    if (payload.empty()) {
        return cmd;
    }
    return cmd + ":" + payload;
}

// Parse a packet: returns {cmd, payload}
inline std::pair<std::string, std::string> ParsePacket(const std::string& packet) {
    size_t sep = packet.find(':');
    if (sep == std::string::npos) {
        return {packet, ""};
    }
    return {packet.substr(0, sep), packet.substr(sep + 1)};
}