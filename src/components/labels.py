"""
Label Components untuk Gaming Tool
"""
from PyQt6.QtWidgets import QLabel, QGraphicsDropShadowEffect, QFrame, QHBoxLayout
from PyQt6.QtCore import Qt, QPropertyAnimation, QEasingCurve, pyqtProperty
from PyQt6.QtGui import QColor

import sys
sys.path.append('..')
from src.theme import Colors


class GlowLabel(QLabel):
    """Label dengan glow effect"""
    
    def __init__(self, text: str = "", glow_color: str = None, parent=None):
        super().__init__(text, parent)
        self.glow_color_hex = glow_color or Colors.ACCENT_LIGHT
        self._glow_intensity = 150
        
        self._setup_style()
        self._setup_glow()
    
    def _setup_style(self):
        self.setStyleSheet(f"""
            QLabel {{
                color: {self.glow_color_hex};
                font-size: 24px;
                font-weight: 700;
                background: transparent;
                border: none;
            }}
        """)
    
    def _setup_glow(self):
        self.glow = QGraphicsDropShadowEffect(self)
        self.glow.setBlurRadius(20)
        self.glow.setOffset(0, 0)
        color = QColor(self.glow_color_hex)
        color.setAlpha(self._glow_intensity)
        self.glow.setColor(color)
        self.setGraphicsEffect(self.glow)


class TitleLabel(QLabel):
    """Title label untuk headings"""
    
    def __init__(self, text: str = "", level: int = 1, parent=None):
        super().__init__(text, parent)
        self.level = level
        self._setup_style()
    
    def _setup_style(self):
        sizes = {1: 28, 2: 24, 3: 20, 4: 16, 5: 14, 6: 12}
        weights = {1: 800, 2: 700, 3: 700, 4: 600, 5: 600, 6: 500}
        
        size = sizes.get(self.level, 20)
        weight = weights.get(self.level, 600)
        
        self.setStyleSheet(f"""
            QLabel {{
                color: {Colors.TEXT_PRIMARY};
                font-size: {size}px;
                font-weight: {weight};
                background: transparent;
                border: none;
            }}
        """)


class BadgeLabel(QFrame):
    """Badge/chip label"""
    
    VARIANTS = {
        "primary": (Colors.PRIMARY, Colors.TEXT_PRIMARY),
        "accent": (Colors.ACCENT, Colors.SECONDARY_DARKEST),
        "success": (Colors.SUCCESS, Colors.TEXT_PRIMARY),
        "warning": (Colors.WARNING, Colors.SECONDARY_DARKEST),
        "error": (Colors.ERROR, Colors.TEXT_PRIMARY),
        "info": (Colors.INFO, Colors.TEXT_PRIMARY)
    }
    
    def __init__(self, text: str = "", variant: str = "primary", parent=None):
        super().__init__(parent)
        self.variant = variant
        self._setup_ui(text)
        self._setup_style()
    
    def _setup_ui(self, text: str):
        layout = QHBoxLayout(self)
        layout.setContentsMargins(12, 6, 12, 6)
        layout.setSpacing(0)
        
        self.label = QLabel(text)
        self.label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        layout.addWidget(self.label)
    
    def _setup_style(self):
        bg_color, text_color = self.VARIANTS.get(self.variant, self.VARIANTS["primary"])
        
        self.setStyleSheet(f"""
            BadgeLabel {{
                background: {bg_color};
                border-radius: 14px;
                border: none;
            }}
        """)
        
        self.label.setStyleSheet(f"""
            QLabel {{
                color: {text_color};
                font-size: 11px;
                font-weight: 700;
                background: transparent;
                border: none;
            }}
        """)
    
    def setText(self, text: str):
        self.label.setText(text)


class StatusIndicator(QFrame):
    """Status indicator dot"""
    
    def __init__(self, status: str = "offline", size: int = 12, parent=None):
        super().__init__(parent)
        self.status = status
        self.indicator_size = size
        self._setup_style()
    
    def _setup_style(self):
        colors = {
            "online": Colors.SUCCESS,
            "offline": Colors.SECONDARY_LIGHT,
            "away": Colors.WARNING,
            "busy": Colors.ERROR
        }
        
        color = colors.get(self.status, colors["offline"])
        
        self.setFixedSize(self.indicator_size, self.indicator_size)
        self.setStyleSheet(f"""
            StatusIndicator {{
                background: {color};
                border-radius: {self.indicator_size // 2}px;
                border: 2px solid {Colors.SECONDARY_DARK};
            }}
        """)
    
    def set_status(self, status: str):
        self.status = status
        self._setup_style()
