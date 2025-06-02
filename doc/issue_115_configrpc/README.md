# Development Notes for Issue #115 – Config RPC Exposure

This folder tracks development work related to [Bitcoin Knots Issue #115](https://github.com/bitcoinknots/bitcoin/issues/115), which proposes the creation of new RPC endpoints to expose configuration values from GUI-only or runtime policy settings for use in headless environments, dashboards and website integrations.

---

## 🔧 Problem Summary

Currently, several node configuration and policy settings are only viewable in the Bitcoin Knots GUI and are not accessible via RPC. This limits observability and tooling integration.  Furthermore, there is a need for runtime updates and the ability to safely share configuration settings over RPC.

---

## 🎯 Community-Driven Goals (Summarized from Discord and GitHub Issue #115)

This initiative aims to create **four RPCs** that satisfy three key design goals:

1. **Expose GUI-only settings via RPC** (for headless usability)
2. **Support runtime updates to those settings via RPC**
3. **Enable safe export and import of configuration bundles (for backup or network-wide consistency)**

---

## 📍 RPC Roadmap

| RPC Name              | Purpose                                        | Status                                       |
|-----------------------|------------------------------------------------|----------------------------------------------|
| `getconfiginfo`       | Exposes current node config values             | ✅ Implemented locally (branch: `feature/getconfiginfo`) |
| `setconfigvalue`      | Enables runtime updates to specific settings   | ⏳ Planned / In Progress (`feature/setconfiginfo`) |
| `exportconfigbundle`  | Outputs all user-modifiable config values      | ⏳ Planned / In Progress (`feature/exportconfigbundle`) |
| `importconfigbundle`  | Applies config bundle with safe validation     | ⏳ Planned / In Progress (`feature/importconfigbundle`) |


All implementations will be designed to return structured, predictable JSON objects for integration with external tools.

---

## 📦 Current Status as of Commit-date 06/02/2025

- 🔨 `getconfiginfo` implemented
- ✅ Compiles cleanly, visible under RPC `help`
- ✅ Returns datacarrier, fees, and blocks-only status with expected formatting conventions
- 🔁 Prepping next RPC: `setconfigvalue`

---

## 📁 Repo Structure (change-files only for #115)

```text
KNOTS-DEV/
├── doc/
│   └── issue_115_configrpc/
│       └── README.md           ← You are here
├── src/
│   └── rpc/
│       ├── config.cpp          ← New RPC implementation
│       └── register.h          ← RPC registration
├── src/Makefile.am             ← Added config.cpp to build
```
---

## 🚧 Next Steps

- Continue implementation of `setconfigvalue` RPC
- Validate parameter security and input coercion
- Create test harness for headless RPC config access
- Coordinate with maintainers on export/import design