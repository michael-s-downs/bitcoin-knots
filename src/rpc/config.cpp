// Copyright (c) 2024 The Bitcoin Knots Developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or https://opensource.org/licenses/MIT.

/**
 * RPC command: getconfiginfo
 *
 * Returns selected runtime configuration values, especially those with
 * policy or economic implications. Useful for troubleshooting, monitoring,
 * and external automation systems. This is a read-only endpoint, further config RPCs can follow.
 *
 * Note: Intended to assist node operators, dashboard builders, and devs 
 * working with nodes in dynamic environments.
 */

// Note: FormatMoney expects satoshi units (CAmount = int64_t).
// We convert BTC string args via AmountFromValue to preserve full precision.

#include <node/context.h>       // Full definition of NodeContext
#include <rpc/server_util.h>    // For EnsureAnyNodeContext
#include <rpc/server.h>         // For RPC interfaces
#include <common/args.h>        // For ArgsManager
#include <string>               // For std::stoi and std::stod
#include <univalue.h>           // For UniValue JSON
#include <consensus/amount.h>   // for CAmount and FormatMoney
#include <util/moneystr.h>      // For FormatMoney
#include <rpc/util.h>           // for AmountFromValue

static RPCHelpMan getconfiginfo()
{
    return RPCHelpMan{
        "getconfiginfo",
        "\nReturns the current node configuration values grouped by category.\n"
        "This includes relay, mempool, block creation, and privacy-related settings.\n"
        "Useful for headless nodes, remote dashboards, or policy monitoring tools.\n",
        {},  // No arguments
        RPCResult{
            RPCResult::Type::OBJ, "", "",
            {
                {RPCResult::Type::OBJ, "mempool", "Mempool and relay configuration", {
                    {RPCResult::Type::BOOL, "datacarrier", "Whether OP_RETURN transactions are relayed"},
                    {RPCResult::Type::NUM, "datacarriersize", "Maximum bytes allowed in OP_RETURN data"},
                    {RPCResult::Type::BOOL, "permitbaremultisig", "Whether bare multisig outputs are relayed"},
                    {RPCResult::Type::BOOL, "blocksonly", "Whether the node ignores incoming transactions"},
                    {RPCResult::Type::NUM, "maxmempool", "Maximum mempool size in MB"},
                    {RPCResult::Type::STR_AMOUNT, "mempoolminfee", "Minimum fee rate (BTC/kvB) for relay/mempool acceptance"}
                }},
                {RPCResult::Type::OBJ, "block_creation", "Block template generation policies", {
                    {RPCResult::Type::NUM, "blockmaxsize", "Maximum block size in bytes"},
                    {RPCResult::Type::NUM, "blockmaxweight", "Maximum block weight (segwit adjusted)"},
                    {RPCResult::Type::STR_AMOUNT, "blockmintxfee", "Minimum fee rate (BTC/kvB) for mined transactions"},
                    {RPCResult::Type::NUM, "blockprioritysize", "Reserved space (in bytes) for high-priority transactions"}
                }},
                {RPCResult::Type::OBJ, "privacy", "Privacy-related and peer interaction settings", {
                    {RPCResult::Type::BOOL, "whitelistforcerelay", "Force relay from whitelisted peers regardless of policy"},
                    {RPCResult::Type::BOOL, "whitelistrelay", "Allow relaying from whitelisted peers even if fee is below minimum"},
                    {RPCResult::Type::BOOL, "walletrbf", "Whether opt-in Replace-by-Fee is allowed"},
                    {RPCResult::Type::BOOL, "walletbroadcast", "Whether wallet rebroadcasts unconfirmed transactions"},
                    {RPCResult::Type::BOOL, "dnsseed", "Use DNS seeding to discover new peers"},
                    {RPCResult::Type::BOOL, "fixedseeds", "Use built-in fallback peer IPs if DNS seeding fails"},
                    {RPCResult::Type::BOOL, "listenonion", "Enable inbound Tor hidden service"},
                    {RPCResult::Type::BOOL, "listen", "Whether to listen for inbound peer connections"},
                    {RPCResult::Type::BOOL, "peerbloomfilters", "Support legacy bloom filter-based SPV clients"},
                    {RPCResult::Type::BOOL, "blockfilterindex", "Enable block filter index (used by compact block filters)"}
                }}
            }
        },
        RPCExamples{
            HelpExampleCli("getconfiginfo", "") +
            HelpExampleRpc("getconfiginfo", "")
        },
        [](const RPCHelpMan& self, const JSONRPCRequest& request) -> UniValue
        {
            const ArgsManager& args = *::EnsureAnyNodeContext(request.context).args;

            UniValue result(UniValue::VOBJ);

            UniValue mempool(UniValue::VOBJ);
            mempool.pushKV("datacarrier", args.GetBoolArg("-datacarrier", true));
            mempool.pushKV("datacarriersize", std::stoi(args.GetArg("-datacarriersize", "83")));
            mempool.pushKV("permitbaremultisig", args.GetBoolArg("-permitbaremultisig", false));
            mempool.pushKV("blocksonly", args.GetBoolArg("-blocksonly", false));
            mempool.pushKV("maxmempool", std::stoi(args.GetArg("-maxmempool", "300")));
            mempool.pushKV("mempoolminfee", FormatMoney(AmountFromValue(UniValue(args.GetArg("-mempoolminfee", "0.00001")))));

            UniValue block_creation(UniValue::VOBJ);
            block_creation.pushKV("blockmaxsize", std::stoi(args.GetArg("-blockmaxsize", "300000")));
            block_creation.pushKV("blockmaxweight", std::stoi(args.GetArg("-blockmaxweight", "1500000")));
            block_creation.pushKV("blockmintxfee", FormatMoney(AmountFromValue(UniValue(args.GetArg("-blockmintxfee", "0.00001")))));
            block_creation.pushKV("blockprioritysize", std::stoi(args.GetArg("-blockprioritysize", "100000")));

            UniValue privacy(UniValue::VOBJ);
            privacy.pushKV("whitelistforcerelay", args.GetBoolArg("-whitelistforcerelay", false));
            privacy.pushKV("whitelistrelay", args.GetBoolArg("-whitelistrelay", false));
            privacy.pushKV("walletrbf", args.GetBoolArg("-walletrbf", false));
            privacy.pushKV("walletbroadcast", args.GetBoolArg("-walletbroadcast", false));
            privacy.pushKV("dnsseed", args.GetBoolArg("-dnsseed", true));
            privacy.pushKV("fixedseeds", args.GetBoolArg("-fixedseeds", true));
            privacy.pushKV("listenonion", args.GetBoolArg("-listenonion", true));
            privacy.pushKV("listen", args.GetBoolArg("-listen", true));
            privacy.pushKV("peerbloomfilters", args.GetBoolArg("-peerbloomfilters", false));
            privacy.pushKV("blockfilterindex", args.GetBoolArg("-blockfilterindex", true));

            result.pushKV("mempool", mempool);
            result.pushKV("block_creation", block_creation);
            result.pushKV("privacy", privacy);

            return result;
        }
    };
}

void RegisterConfigRPCCommands(CRPCTable& t)
{
    static const CRPCCommand commands[]{
        { "control", &getconfiginfo }
    };
    for (const auto& c : commands) {
        t.appendCommand(c.name, &c);
    }
}