# Development Notes for Issue #115 – Config RPC Exposure

This folder tracks development work related to [Bitcoin Knots Issue #115](https://github.com/bitcoinknots/bitcoin/issues/115), which proposes the creation of new RPC endpoints to expose configuration values from GUI-only or runtime policy settings for use in headless environments, dashboards, website integrations and network sharing of settings via RPC.

---

## 🔧 Problem Summary

Currently, several node configuration and policy settings are only viewable in the Bitcoin Knots GUI and are not accessible via RPC. This limits observability and tooling integration. Further, there is a need for runtime updates and network sharing of settings via RPC.

---

## 🧭 Big Picture – One Flexible Reader/Writer Pair

This effort has consolidated around a single flexible pair of RPCs that support both local configuration updates and remote configuration fetching.

### Unified Pair: Runtime Get/Set (Session-Scoped + Optional Persist)

- **`getconfigvalues`** (Previously `getconfiginfo`)
  - Purpose: Fetch current runtime config values.
  - Input: Optional list of config keys (defaults to all). Optional `targetnode` argument (default: self).
  - Output: Flat key-value JSON object.
  - Notes:
    - Output grouping for UX purposes is **client-side only** to avoid long-term coupling.
    - Category-based display grouping (e.g., mempool, privacy) should not be embedded in core responses.
  - Potential Enhancements:
    1. Add `targetnode` to pull config from another node.
    2. Add subset fetch via `keys` array (e.g., `["blockmintxfee"]`, `[*]`= all). 
    3. Add `persist=true` option to write fetched config to `bitcoin_rw.conf`.
    4. (TBD) Allow fetch-and-apply combo, e.g., for copying trusted configs.

- **`setconfigvalues`** (In Progress)
  - Purpose: Set one or more config values at runtime.
  - Input: JSON object matching config schema.
  - Output: Array of per-key result objects:
    ```json
    [
      {
        "key": "string (keyname)",
        "applied": true|false,
        "persisted": true|false,
        "errors": [
            NONE,
            KEY_NOT_FOUND,
            FILE_ERROR,
            NODE_NOT_FOUND,...
        ]
      }
    ]
    ```
    This structure is designed to allow batched feedback for batched input, even when some keys succeed and others fail.
  
  - Potential Enhancements:
    - Accept `persist=true` to immediately write all accepted settings to `bitcoin_rw.conf`.

---

## 📍 RPC Roadmap

| RPC Name          | Purpose                                         | Status                                   |
|-------------------|--------------------------------------------------|------------------------------------------|
| `getconfigvalues` | Exposes current node config values              | ✅ Implemented locally                    |
| `setconfigvalues` | Enables runtime updates to specific settings    | ⏳ In progress                            |

All implementations return structured JSON objects for integration with external tools.

---

## 📦 Current Status as of Commit-date 06/04/2025

- 🔨 `getconfigvalues` implemented
- ✅ Compiles cleanly, visible under RPC `help`
- ✅ Returns expected config items as flat JSON object
- ✅ Accepts optional UX-style grouping (legacy)
- 🔁 Prepping next RPC: `setconfigvalues`

---

## 🧱 Proposed Config Schema

We propose a flat key-value map format mirroring `bitcoin.conf` layout, represented in JSON Schema form:

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

This schema is suitable for UI inputs, file-based persistence (`bitcoin_rw.conf`), scripting, and automation flows.

---

## 🔁 Runtime Change Semantics

Settings fall into different categories of runtime mutability under the following approach:

- **No eviction or disconnection** of current txns or peers.
- **Only new data** (connections, transactions, blocks) is affected.
- **Non-runtime modifiable settings** are clearly identified.

### Runtime Mutability Table

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
| blockprioritysize    | ✅ Yes  | Mining block template override                                 | Mining             |
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

## 🧰 Operator Use Cases

- 🔁 Dynamic testing without restart
- 🛠️ Remote dashboards and observability
- 📦 Bulk config application via API
- 🔐 Future secure RPC export/import workflows

---

## 🔭 Future Network Interop (Optional Direction)

Potential future goal: support trusted config pulls from canonical sources (e.g., pools, federations, NGOs).

- Example: `getconfigvalues targetnode=template.ocean.coop persist=true`

---

## 📁 Repo Structure (change-files only for #115)

```text
KNOTS-DEV/
├── doc/
│   └── issue_115_configrpc/
│       └── README.md      ← You are here
├── src/
│   └── rpc/
│       └── config.cpp     ← New RPC implementation
│       └── register.h     ← RPC registration hook
└── src/Makefile.am        ← Build integration
```
