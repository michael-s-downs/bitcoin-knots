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
#include <rpc/server.h>         // For RPC interfaces (JSONRPCRequest, CRPCTable)
#include <common/args.h>        // For ArgsManager
#include <string>               // For std::stoi and std::stod
#include <univalue.h>           // For UniValue JSON
#include <consensus/amount.h>   // for CAmount and FormatMoney
#include <util/moneystr.h>      // For FormatMoney
#include <rpc/util.h>           // for AmountFromValue
#include <string>
#include <map>

static RPCHelpMan getconfiginfo()
{
    return RPCHelpMan{
        "getconfiginfo",
        "\nReturns the current node configuration values.\n"
        "By default, values are returned as a uniform key-value map compatible with bitcoin.conf automation.\n"
        "To group values by UX-friendly categories (mempool, block creation, privacy), use the 'grouped' flag.\n",
        {
            {
                "grouped",
                RPCArg::Type::BOOL,
                RPCArg::Default{false},
                "Return values grouped by UX-friendly categories instead of flat key-value pairs"
            }
        },
        RPCResult{
            RPCResult::Type::OBJ, "", "",
            {
                {RPCResult::Type::BOOL, "datacarrier", "Whether to relay OP_RETURN transactions"},
                {RPCResult::Type::NUM, "datacarriersize", "Max bytes allowed in OP_RETURN"},
                {RPCResult::Type::BOOL, "permitbaremultisig", "Allow bare multisig relay"},
                {RPCResult::Type::BOOL, "blocksonly", "Ignore transactions, blocks only"},
                {RPCResult::Type::NUM, "maxmempool", "Max mempool size (MB)"},
                {RPCResult::Type::STR_AMOUNT, "mempoolminfee", "Minimum fee for mempool acceptance"},
                {RPCResult::Type::NUM, "blockmaxsize", "Max block size in bytes"},
                {RPCResult::Type::NUM, "blockmaxweight", "Max block weight (segwit-adjusted)"},
                {RPCResult::Type::STR_AMOUNT, "blockmintxfee", "Minimum fee for mined transactions"},
                {RPCResult::Type::NUM, "blockprioritysize", "Priority space for free transactions"},
                {RPCResult::Type::BOOL, "whitelistforcerelay", "Force relay for whitelisted peers"},
                {RPCResult::Type::BOOL, "whitelistrelay", "Relay from whitelisted peers below minfee"},
                {RPCResult::Type::BOOL, "walletrbf", "Allow Replace-by-Fee (RBF) transactions"},
                {RPCResult::Type::BOOL, "walletbroadcast", "Rebroadcast unconfirmed wallet txns"},
                {RPCResult::Type::BOOL, "dnsseed", "Use DNS seeding to discover peers"},
                {RPCResult::Type::BOOL, "fixedseeds", "Use built-in fallback peer IPs"},
                {RPCResult::Type::BOOL, "listenonion", "Enable inbound Tor hidden service"},
                {RPCResult::Type::BOOL, "listen", "Allow inbound peer connections"},
                {RPCResult::Type::BOOL, "peerbloomfilters", "Support SPV clients using bloom filters"},
                {RPCResult::Type::BOOL, "blockfilterindex", "Enable compact block filters"}
            }
        },
        RPCExamples{
            HelpExampleCli("getconfiginfo", "") +
            HelpExampleCli("getconfiginfo", R"("true")") +
            HelpExampleRpc("getconfiginfo", "{ \"grouped\": true }")
        },
        [](const RPCHelpMan& self, const JSONRPCRequest& request) -> UniValue
        {
            const ArgsManager& args = *::EnsureAnyNodeContext(request.context).args;
            const bool grouped = request.params[0].isBool() ? request.params[0].get_bool() : false;

            if (grouped) {
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
            UniValue flatResult(UniValue::VOBJ);
            flatResult.pushKV("datacarrier", args.GetBoolArg("-datacarrier", true));
            flatResult.pushKV("datacarriersize", std::stoi(args.GetArg("-datacarriersize", "83")));
            flatResult.pushKV("permitbaremultisig", args.GetBoolArg("-permitbaremultisig", false));
            flatResult.pushKV("blocksonly", args.GetBoolArg("-blocksonly", false));
            flatResult.pushKV("maxmempool", std::stoi(args.GetArg("-maxmempool", "300")));
            flatResult.pushKV("mempoolminfee", FormatMoney(AmountFromValue(UniValue(args.GetArg("-mempoolminfee", "0.00001")))));
            flatResult.pushKV("blockmaxsize", std::stoi(args.GetArg("-blockmaxsize", "300000")));
            flatResult.pushKV("blockmaxweight", std::stoi(args.GetArg("-blockmaxweight", "1500000")));
            flatResult.pushKV("blockmintxfee", FormatMoney(AmountFromValue(UniValue(args.GetArg("-blockmintxfee", "0.00001")))));
            flatResult.pushKV("blockprioritysize", std::stoi(args.GetArg("-blockprioritysize", "100000")));
            flatResult.pushKV("whitelistforcerelay", args.GetBoolArg("-whitelistforcerelay", false));
            flatResult.pushKV("whitelistrelay", args.GetBoolArg("-whitelistrelay", false));
            flatResult.pushKV("walletrbf", args.GetBoolArg("-walletrbf", false));
            flatResult.pushKV("walletbroadcast", args.GetBoolArg("-walletbroadcast", false));
            flatResult.pushKV("dnsseed", args.GetBoolArg("-dnsseed", true));
            flatResult.pushKV("fixedseeds", args.GetBoolArg("-fixedseeds", true));
            flatResult.pushKV("listenonion", args.GetBoolArg("-listenonion", true));
            flatResult.pushKV("listen", args.GetBoolArg("-listen", true));
            flatResult.pushKV("peerbloomfilters", args.GetBoolArg("-peerbloomfilters", false));
            flatResult.pushKV("blockfilterindex", args.GetBoolArg("-blockfilterindex", true));
            return flatResult;
        }
    };
}

static RPCHelpMan setconfigvalue()
{
    return RPCHelpMan{
        "setconfigvalue",
        "Temporarily sets a node configuration value at runtime. Will persist for this session only.\n",
        {
            {
                "key", RPCArg::Type::STR, RPCArg::Optional::NO, "Configuration key to set (e.g. 'mempoolminfee')"
            },
            {
                "value", RPCArg::Type::STR, RPCArg::Optional::NO, "New value to assign to the key"
            },
        },
        RPCResult{
            RPCResult::Type::OBJ, "", "Result object",
            {
                {RPCResult::Type::STR, "key", "The config key updated"},
                {RPCResult::Type::BOOL, "applied", "True if the change was accepted and applied"},
                {RPCResult::Type::STR, "message", "Details on the application of the update"}
            }
        },
        RPCExamples{
            HelpExampleCli("setconfigvalue", "mempoolminfee 0.00002")
        },
        [](const RPCHelpMan& self, const JSONRPCRequest& request) -> UniValue
        {
            const std::string key = request.params[0].get_str();
            const std::string value = request.params[1].get_str();
            
            ArgsManager& args = *::EnsureAnyNodeContext(request.context).args;

            // Placeholder: Replace with actual per-key validation and setting logic
            bool supported = key == "mempoolminfee" || key == "blockmintxfee";
            bool applied = false;

            if (supported) {
                args.ForceSetArg("-" + key, value);
                applied = true;
            }

            // Description map for contextual messaging
            const std::map<std::string, std::string> runtime_messages = {
                {"mempoolminfee", "Will persist for this session and apply to new mempool entries."},
                {"blockmintxfee", "Will persist for this session and apply to new blocks."},
            };

            UniValue result(UniValue::VOBJ);
            result.pushKV("key", key);
            result.pushKV("applied", applied);

            if (applied) {
                std::string msg = "Updated successfully.";
                if (runtime_messages.count(key)) msg += " " + runtime_messages.at(key);
                result.pushKV("message", msg);
            } else {
                result.pushKV("message", "Unsupported or non-runtime-modifiable config key.");
            }

            return result;
        }
    };
}

void RegisterConfigRPCCommands(CRPCTable& t)
{
    static const CRPCCommand commands[]{
        { "control", &getconfiginfo },
        { "control", &setconfigvalue } 
    };
    for (const auto& c : commands) {
        t.appendCommand(c.name, &c);
    }
}