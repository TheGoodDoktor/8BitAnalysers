# CPC ItemList Bank Ordering

## Background

`FCodeAnalysisState::ItemList` is a flat, ordered list of every annotated item
(labels, code, data) across the full 64K address space. It is rebuilt by
`UpdateItemList()` in two phases:

1. **Rebuild phase** — iterates all registered banks in registration order.
   For each dirty or empty bank, calls `UpdateItemListForBank()`, which walks the
   bank's pages and stores each item's address as
   `bank.PrimaryMappedPage * kPageSize + bankOffset`.

2. **Assembly phase** — walks the address space from page 0 upward using
   `MappedReadBanks[]`, appending each currently-mapped bank's cached `ItemList`
   to `state.ItemList` in slot order (0x0000 → 0x4000 → 0x8000 → 0xC000).

The assembly phase produces items in address order **provided** every bank's
cached `ItemList` was rebuilt against its *current* `PrimaryMappedPage`. If a
bank's `ItemList` is stale (built for a previous slot), its items carry addresses
from the old mapping while the assembly phase places the bank at its new slot —
producing an out-of-order `state.ItemList`.

## How the problem arises

`MapBank()` only sets `bIsDirty = true` the very first time a bank is mapped
(`bEverBeenMapped == false`). Subsequent remappings do not automatically dirty
the bank. `SetBankPrimaryPage()` updates `PrimaryMappedPage` but also does not
set `bIsDirty`.

### Concrete example

**Step 1 — initial state, RAM config 0 (standard mapping)**

| Slot | Address       | Bank  | PrimaryMappedPage |
|------|---------------|-------|-------------------|
| 0    | 0x0000–0x3FFF | RAM 3 | 0 (page 0)        |
| 1    | 0x4000–0x7FFF | RAM 1 | 16 (page 16)      |
| 2    | 0x8000–0xBFFF | RAM 2 | 32 (page 32)      |
| 3    | 0xC000–0xFFFF | RAM 3 | 48 (page 48)      |

`UpdateItemList` runs. Each bank's `ItemList` is built correctly:

- RAM 0 `ItemList`: items with addresses 0x0000–0x3FFF  
- RAM 3 `ItemList`: items with addresses 0xC000–0xFFFF  

`state.ItemList` assembled in correct address order. ✓

**Step 2 — bank switch: RAM config 2 moves RAM 3 from slot 3 to slot 0**

`SetBankPrimaryPage(RAM 3, 0)` updates `RAM 3.PrimaryMappedPage = 0`.  
Assume `bIsDirty` is **not** set for RAM 3 (the unsafe path without `SetAllBanksDirty`).

`UpdateItemList` runs:

- Assembly phase: `GetBankFromAddress(0x0000)` returns RAM 3 → appends RAM 3's
  stale `ItemList` (addresses **0xC000–0xFFFF**) into the *start* of `state.ItemList`.
- The bank now at slot 3 appends its items (0xC000–0xFFFF) at the end.

**Result:** `state.ItemList` begins with 0xC000 items, then has 0x4000 and 0x8000
items, then 0xC000 items again — items from block 3 appear before items from
block 0.

## The fix: SetAllBanksDirty

`FCPCEmu::UpdateBankMappings()` ends with:

```cpp
// Force all banks to update their item list.
CodeAnalysis.SetAllBanksDirty();
```

This marks every bank dirty after every banking change, ensuring all banks
rebuild their `ItemList` against their current `PrimaryMappedPage` before the
next assembly phase. It prevents stale addresses from entering `state.ItemList`.

## The safety net: stable_sort in FCPCAsmExporterBase

The CPC ASM exporter does not iterate `state.ItemList` directly (as the base
class `FASMExporter::ExportAddressRange` does). Instead it sorts a copy of the
in-range item indices by `AddressRef.Address` before iterating:

```cpp
std::stable_sort(indices.begin(), indices.end(), [&state](size_t a, size_t b) {
    return state.ItemList[a].AddressRef.Address < state.ItemList[b].AddressRef.Address;
});
```

This corrects any residual ordering issues even if `SetAllBanksDirty` is missed
— for example at startup before the first full rebuild, or if a future code path
introduces a remapping that bypasses `SetAllBanksDirty`.

`stable_sort` is used rather than plain `sort` because labels and their
associated code share the same address. `UpdateItemListForBank` always appends a
label before the code item at the same address; `stable_sort` preserves that
relative order for equal-address items, keeping labels above their instructions
in the exported output.

### Example: stable vs plain sort

Consider a label `GameLoop` and a `LD A,0x01` instruction, both at address
0x4000. A data item `db 0x00` sits at 0x4001.

`state.ItemList` (indices 0–2, after the bank rebuild):

| Index | Type  | AddressRef.Address | Content         |
|-------|-------|--------------------|-----------------|
| 0     | Label | 0x4000             | `GameLoop`      |
| 1     | Code  | 0x4000             | `LD A,0x01`     |
| 2     | Data  | 0x4001             | `db 0x00`       |

The sort comparator key for each index is just `AddressRef.Address`:

| Index | Sort key |
|-------|----------|
| 0     | 0x4000   |
| 1     | 0x4000   |
| 2     | 0x4001   |

Indices 0 and 1 have **equal keys**. This is where stable vs plain sort
differs:

**`std::sort` (unstable)** — equal-key elements may be reordered arbitrarily.
The sort is free to produce `[1, 0, 2]`, putting the code before the label:

```asm
; $4000    LD A,0x01       ; code item output first — WRONG
GameLoop:                   ; label output second
          db 0x00
```

**`std::stable_sort`** — equal-key elements keep their original relative order.
Index 0 (label) was before index 1 (code) in the input, so the result is always
`[0, 1, 2]`:

```asm
GameLoop:                   ; $4000  — label output first ✓
; $4000    LD A,0x01
          db 0x00
```

The label must precede the instruction it names so that downstream tooling (or
an AI agent reading the export) correctly associates `GameLoop` with the `LD`
instruction at 0x4000 rather than treating it as floating above unrelated code.
