"""
Color Palette untuk Gaming Tool Theme
Dominan: Purple & Dark Blue dengan aksen Cyan/Teal
"""

class Colors:
    """Color system untuk gaming tool dengan nuansa gelap & cyberpunk"""
    
    # ===== PRIMARY COLORS - Purple =====
    PRIMARY_DARKEST = "#1a0a2e"      # Sangat gelap, untuk background utama
    PRIMARY_DARKER = "#2d1b4e"       # Gelap, untuk panel/card background
    PRIMARY_DARK = "#4a2872"         # Dark purple untuk hover states
    PRIMARY = "#6b3fa0"              # Main purple
    PRIMARY_LIGHT = "#8b5fc4"        # Light purple untuk highlights
    PRIMARY_LIGHTER = "#ab7fe8"      # Lighter untuk text highlights
    PRIMARY_LIGHTEST = "#c9a3f5"     # Sangat terang untuk glow effects
    
    # ===== SECONDARY COLORS - Dark Blue =====
    SECONDARY_DARKEST = "#0a0f1a"    # Sangat gelap, hampir hitam
    SECONDARY_DARKER = "#111827"     # Dark blue-gray untuk main background
    SECONDARY_DARK = "#1e293b"       # Dark slate untuk cards
    SECONDARY = "#334155"            # Main slate blue
    SECONDARY_LIGHT = "#475569"      # Untuk borders
    SECONDARY_LIGHTER = "#64748b"    # Untuk disabled states
    SECONDARY_LIGHTEST = "#94a3b8"   # Untuk secondary text
    
    # ===== ACCENT COLORS - Cyan/Teal =====
    ACCENT_DARKEST = "#083344"       # Deep cyan
    ACCENT_DARKER = "#0e4a5c"        # Dark cyan
    ACCENT_DARK = "#155e75"          # Cyan dark
    ACCENT = "#0891b2"               # Main cyan/teal
    ACCENT_LIGHT = "#22d3ee"         # Bright cyan
    ACCENT_LIGHTER = "#67e8f9"       # Very bright cyan
    ACCENT_LIGHTEST = "#a5f3fc"      # Glow cyan
    
    # ===== GRADIENT PRESETS =====
    GRADIENT_PRIMARY = "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #6b3fa0, stop:1 #0891b2)"
    GRADIENT_DARK = "qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #1a0a2e, stop:1 #0a0f1a)"
    GRADIENT_BUTTON = "qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #6b3fa0, stop:1 #8b5fc4)"
    GRADIENT_BUTTON_HOVER = "qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #8b5fc4, stop:1 #ab7fe8)"
    GRADIENT_ACCENT = "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #0891b2, stop:1 #22d3ee)"
    GRADIENT_GLOW = "qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #6b3fa0, stop:0.5 #0891b2, stop:1 #22d3ee)"
    
    # ===== TEXT COLORS =====
    TEXT_PRIMARY = "#f8fafc"         # Putih untuk text utama
    TEXT_SECONDARY = "#94a3b8"       # Gray untuk secondary text
    TEXT_MUTED = "#64748b"           # Muted text
    TEXT_DISABLED = "#475569"        # Disabled text
    TEXT_ACCENT = "#22d3ee"          # Cyan text untuk highlights
    TEXT_WARNING = "#fbbf24"         # Yellow untuk warning
    TEXT_ERROR = "#ef4444"           # Red untuk error
    TEXT_SUCCESS = "#22c55e"         # Green untuk success
    
    # ===== STATUS COLORS =====
    SUCCESS = "#22c55e"              # Green
    SUCCESS_DARK = "#15803d"         # Dark green
    SUCCESS_LIGHT = "#4ade80"        # Light green
    
    WARNING = "#f59e0b"              # Amber
    WARNING_DARK = "#b45309"         # Dark amber
    WARNING_LIGHT = "#fbbf24"        # Light amber
    
    ERROR = "#ef4444"                # Red
    ERROR_DARK = "#b91c1c"           # Dark red
    ERROR_LIGHT = "#f87171"          # Light red
    
    INFO = "#3b82f6"                 # Blue
    INFO_DARK = "#1d4ed8"            # Dark blue
    INFO_LIGHT = "#60a5fa"           # Light blue
    
    # ===== SPECIAL EFFECTS =====
    GLOW_PURPLE = "#8b5fc4"
    GLOW_CYAN = "#22d3ee"
    SHADOW_COLOR = "rgba(0, 0, 0, 0.5)"
    OVERLAY_DARK = "rgba(10, 15, 26, 0.8)"
    GLASS_BACKGROUND = "rgba(45, 27, 78, 0.6)"
    
    # ===== BORDER COLORS =====
    BORDER_DEFAULT = "#334155"
    BORDER_FOCUS = "#6b3fa0"
    BORDER_HOVER = "#8b5fc4"
    BORDER_ACCENT = "#0891b2"
    BORDER_GLOW = "#22d3ee"
    
    @classmethod
    def rgba(cls, hex_color: str, alpha: float) -> str:
        """Convert hex color to rgba string"""
        hex_color = hex_color.lstrip('#')
        r = int(hex_color[0:2], 16)
        g = int(hex_color[2:4], 16)
        b = int(hex_color[4:6], 16)
        return f"rgba({r}, {g}, {b}, {alpha})"
    
    @classmethod
    def gradient(cls, color1: str, color2: str, direction: str = "horizontal") -> str:
        """Generate gradient string untuk Qt stylesheet"""
        if direction == "horizontal":
            return f"qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 {color1}, stop:1 {color2})"
        elif direction == "vertical":
            return f"qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 {color1}, stop:1 {color2})"
        else:  # diagonal
            return f"qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 {color1}, stop:1 {color2})"
