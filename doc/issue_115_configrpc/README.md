# Development Notes for Issue #115 – Config RPC Exposure

This folder tracks development work related to [Bitcoin Knots Issue #115](https://github.com/bitcoinknots/bitcoin/issues/115), which proposes the creation of new RPC endpoints to expose configuration values from GUI-only or runtime policy settings for use in headless environments, dashboards and website integrations.

---

## 🔧 Problem Summary

Currently, several node configuration and policy settings are only viewable in the Bitcoin Knots GUI and are not accessible via RPC. This limits observability and tooling integration. Further, there is a need for runtime updates and network sharing of settings via RPC.

---

## 🎯 FOSS Community-Guidance Notes (Summarized from Discord & Issue #115 Discussion)

This initiative aims to create **four RPCs** that satisfy three key design goals:

1. **Expose GUI-only settings via RPC** (for headless usability)
2. **Support runtime updates to those settings via RPC**
3. **Enable safe export/import (i.e. sharing) of complete configuration bundles**

---

## 📍 RPC Roadmap

| RPC Name              | Purpose                                         | Status                                   |
|-----------------------|--------------------------------------------------|------------------------------------------|
| `getconfiginfo`       | Exposes current node config values              | ✅ Implemented locally, On-Fork as feature/getconfiginfo |
| `setconfigvalue`      | Enables runtime updates to specific settings    | ⏳ In progress as feature/setconfiginfo   |
| `exportconfigbundle`  | Outputs all user-modifiable config values       | ⏳ Planned                                |
| `importconfigbundle`  | Applies config bundle with safe validation      | ⏳ Planned                                |

All implementations return structured JSON objects for integration with external tools.

---

## 📦 Current Status as of Commit-date 06/04/2025

- 🔨 `getconfiginfo` implemented
- ✅ Compiles cleanly, visible under RPC `help`
- ✅ Returns datacarrier, fees, and blocks-only status with expected formatting conventions
- ✅ Offers optional UX-style grouping or flat key-value map (default)
- 🔁 Prepping next RPC: `setconfigvalue`

---

## 🧱 Proposed Config Schema (get/export/import)

We propose a flat key-value map format mirroring `bitcoin.conf` layout, represented in JSON Schema form, which should ease potential
future movement between file-based configuration (i.e. bitcoin_rw.conf) and dynamic run-time operations, executed locally or via 
network resources:

```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "title": "Bitcoin Config Key-Value Map",
  "type": "object",
  "additionalProperties": {
    "oneOf": [
      { "type": "boolean" },
      { "type": "integer" },
      { "type": "number" },
      { "type": "string" }
    ]
  },
  "examples": [
    {
      "blockmaxweight": 1500000,
      "mempoolminfee": "0.00001",
      "listen": true
    }
  ]
}
```

This schema is suitable for programmatic editing, syncing, UI input, and automation flows.

---

## 🔁 Runtime Change Semantics

Settings fall into different categories of runtime mutability when considered under the following strategy or approach:

- We propose **No eviction or disconnection** of existing peers or mempool contents when relevant settings change at runtime.
- Changes need **only apply to new inbound data** (mempool entries, connections, block creation) which keeps this work simple, useful, non-aggressive.
- Some settings are still not amenable to runtime mutation based on the nature of what they control.

We suggest the following table, listing each config key considered under this change, its runtime mutability status, and categorization notes.

| Setting              | Mutable | Notes                                                         | Affects            |
|----------------------|---------|----------------------------------------------------------------|--------------------|
| datacarrier          | ✅ Yes  | Relay policy only, applies to new txns                         | Mempool/Relay      |
| datacarriersize      | ✅ Yes  | Controls validation of OP_RETURN payload                      | Mempool/Relay      |
| permitbaremultisig   | ✅ Yes  | Affects relay policy only                                      | Mempool/Relay      |
| blocksonly           | ✅ Yes  | Applies to new peer tx relay behavior                          | Network/Peers      |
| maxmempool           | ✅ Yes  | Memory usage policy                                            | Mempool            |
| mempoolminfee        | ✅ Yes  | Used in mempool admission, applies to new txns only           | Mempool            |
| blockmaxsize         | ✅ Yes  | Used in next block construction only                           | Mining             |
| blockmaxweight       | ✅ Yes  | Same as above                                                  | Mining             |
| blockmintxfee        | ✅ Yes  | Used in mining template generation                             | Mining             |
| blockprioritysize    | ✅ Yes  | Mining block template override                                | Mining             |
| whitelistforcerelay  | ✅ Yes  | Affects newly arriving peers                                   | Network/Peers      |
| whitelistrelay       | ✅ Yes  | Affects relay of txns from whitelisted peers                   | Network/Peers      |
| walletrbf            | ❌ No   | Requires wallet reload                                         | Wallet             |
| walletbroadcast      | ❌ No   | Wallet-level flag, not dynamically changeable                 | Wallet             |
| dnsseed              | ❌ No   | Used at startup only                                           | Network/Startup    |
| fixedseeds           | ❌ No   | Used at startup fallback only                                 | Network/Startup    |
| listenonion          | ❌ No   | Requires restart                                               | Network            |
| listen               | ❌ No   | Startup-level option                                           | Network/Startup    |
| peerbloomfilters     | ❌ No   | Affects service flags, requires restart                       | Network/Service    |
| blockfilterindex     | ❌ No   | Requires indexing and restart                                 | Indexing/Storage   |

---

## 📤 Planned Return Format for `setconfigvalue`

Because setconfigvalue will take any conformant JSON Bitcoin Config Key-Value Map as input, it will output a matching map of *application result objects*
for each included item (some may succeed, others fail etc).

Here are proposed examples of individual kinds of results: 

```json
{
  "key": "blockmintxfee",
  "applied": true,
  "message": "Updated successfully. Will persist for this session and apply to new transactions or blocks. Use export to transfer to persistent file."
}
```

Other classes of behavior include:

- 🧠 Mempool-only: applies only to new mempool entries.
- 🌐 Peer-related: applies only to newly connected peers.
- ⚠️ Non-modifiable: clear error and guidance to use `bitcoin.conf` or `bitcoin_rw.conf`.

---

## 🧰 Operator Use Cases

- 🔁 Dynamic testing without restart
- 🛠️ Remote dashboards and observability
- 📦 Bulk config application via API
- 🔐 Future secure RPC export/import workflows

---

## 🔭 Future Network Interop (Optional Direction)

A future direction is syncing config schemas (in whole or part) across federated dashboards, trusted peers, or validator sets.

Example: Pool templates, canary configurations, policy alignment, served from community servers for download and application at runtime.

---

## 📁 Repo Structure (change-files only for #115)

```text
KNOTS-DEV/
├── doc/
│   └── issue_115_configrpc/
│       └── README.md      ← You are here
├── src/
│   └── rpc/
│       └── config.cpp     ← New RPC implementations
│       └── register.h     ← RPC registration hook
└── src/Makefile.am        ← Build integration
```