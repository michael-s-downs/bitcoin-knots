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
        "\nReturns current values of selected configuration parameters.\n",
        {},
        RPCResult{
            RPCResult::Type::OBJ, "", "",
            {
                {RPCResult::Type::BOOL, "datacarrier", "Whether to allow OP_RETURN transactions (data carrier)"},
                {RPCResult::Type::NUM, "datacarriersize", "Maximum allowed bytes in OP_RETURN data"},
                {RPCResult::Type::BOOL, "blocksonly", "Whether to operate in blocks-only mode (no transaction relay)"},
                {RPCResult::Type::NUM, "mempoolminfee", "Minimum fee rate (in BTC/kvB) for mempool acceptance"},
                {RPCResult::Type::NUM, "blockmintxfee", "Minimum fee rate for mining (in BTC/kvB)"},
            }
        },
        RPCExamples{
            HelpExampleCli("getconfiginfo", "")
        },
        [](const RPCHelpMan& self, const JSONRPCRequest& request) -> UniValue
        {
            const ArgsManager& args = *::EnsureAnyNodeContext(request.context).args;

            UniValue result(UniValue::VOBJ);
            result.pushKV("datacarrier", args.GetBoolArg("-datacarrier", true));
            result.pushKV("datacarriersize", std::stoi(args.GetArg("-datacarriersize", "83")));
            result.pushKV("blocksonly", args.GetBoolArg("-blocksonly", false));
            result.pushKV("mempoolminfee", FormatMoney(AmountFromValue(UniValue(args.GetArg("-mempoolminfee", "0.00001")))));
            result.pushKV("blockmintxfee", FormatMoney(AmountFromValue(UniValue(args.GetArg("-blockmintxfee", "0.00001")))));


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