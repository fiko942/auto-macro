# Tobelsoft Macro: Superpowers Knowledge Base & Technical Blueprint

**Version:** 2.0 (Native C11 Architecture)  
**Last Updated:** 2026-09-21  

---

## 📂 Superpowers Documentation Tree

```
docs/superpowers/
├── README.md                                          # Master index of superpowers documentation
├── specs/                                             # Technical specifications
│   ├── 2026-09-21-native-c-architecture-spec.md       # C11 system architecture, data models, memory layout
│   ├── 2026-09-21-directinput-macro-engine-spec.md    # Low-level hooks, hardware scancodes, timers
│   └── 2026-09-21-vector-icon-engine-spec.md          # GDI vector icon math, rendering APIs, tokens
├── architecture/                                      # Deep architectural blueprints
│   ├── system-overview.md                             # High-level architecture, thread model, lifecycle
│   ├── directinput-subsystem.md                       # DirectInput scancode injection, kernel hook loop
│   └── gdi-rendering-and-vector-engine.md             # GDI double buffering, color matrix, custom controls
├── benchmarks/                                        # Performance benchmarks & audits
│   └── performance-audit-2026-09-21.md                # Latency, RAM, CPU, startup profiling data
└── plans/                                             # Historical & current implementation plans
    ├── 2026-09-20-about-page.md                       # About page implementation
    ├── 2026-09-20-left-click-safety-and-input-blocking.md # Left click safety guard
    ├── 2026-09-20-ui-smooth-animations.md             # Smooth UI transitions
    ├── 2026-09-20-ui-visual-audit-and-native-polish.md # Visual polish audit
    └── 2026-09-21-native-c-rewrite-and-vector-icon-engine.md # Complete C rewrite & Vector icon engine
```

---

## ⚡ Key Architectural Highlights

1. **Sub-Microsecond Input Latency (<0.001 ms)**:
   - Hardware scancode translation (`KEYEVENTF_SCANCODE`) bypasses Windows virtual key translation layers.
   - O(1) bitwise input hook evaluation (`WH_KEYBOARD_LL` / `WH_MOUSE_LL`).

2. **100% Pure GDI Vector Icon Engine**:
   - Zero dependencies on Windows default Unicode emojis or font-fallback glyphs.
   - 35+ custom geometric rendering routines scaling losslessly to any DPI.

3. **Zero-Dependency Single Executable**:
   - Pure C11 compiled with Clang `-O3`.
   - Links exclusively against native Windows system DLLs.
   - ~215 KB binary size and ~4.2 MB RAM usage.
