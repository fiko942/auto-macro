"""
Custom Button Components untuk Gaming Tool
"""
from PyQt6.QtWidgets import QPushButton, QGraphicsDropShadowEffect
from PyQt6.QtCore import Qt, QPropertyAnimation, QEasingCurve, pyqtProperty, QSize
from PyQt6.QtGui import QColor, QIcon, QFont

import sys
sys.path.append('..')
from src.theme import Colors, Styles


class GamingButton(QPushButton):
    """
    Gaming-styled button dengan animasi dan glow effect
    
    Variants: primary, secondary, accent, danger, ghost
    Sizes: small, normal, large
    """
    
    def __init__(self, text: str = "", variant: str = "primary", 
                 size: str = "normal", parent=None):
        super().__init__(text, parent)
        self.variant = variant
        self.size = size
        self._glow_intensity = 0
        
        self._setup_style()
        self._setup_shadow()
        self._setup_animation()
    
    def _setup_style(self):
        """Apply style berdasarkan variant"""
        styles = {
            "primary": Styles.button_primary(),
            "secondary": Styles.button_secondary(),
            "accent": Styles.button_accent(),
            "danger": Styles.button_danger(),
            "ghost": Styles.button_ghost()
        }
        
        # Size adjustments
        size_styles = {
            "small": "padding: 8px 16px; font-size: 10px; min-height: 16px;",
            "normal": "",  # Use default
            "large": "padding: 16px 32px; font-size: 14px; min-height: 24px;"
        }
        
        base_style = styles.get(self.variant, styles["primary"])
        size_style = size_styles.get(self.size, "")
        
        # Combine styles
        self.setStyleSheet(base_style)
        if size_style:
            current = self.styleSheet()
            # Inject size style into QPushButton block
            self.setStyleSheet(current.replace("QPushButton {", f"QPushButton {{ {size_style}"))
        
        self.setCursor(Qt.CursorShape.PointingHandCursor)
    
    def _setup_shadow(self):
        """Setup drop shadow/glow effect"""
        self.shadow = QGraphicsDropShadowEffect(self)
        
        shadow_colors = {
            "primary": Colors.PRIMARY_LIGHT,
            "secondary": Colors.PRIMARY,
            "accent": Colors.ACCENT_LIGHT,
            "danger": Colors.ERROR_LIGHT,
            "ghost": Colors.TEXT_PRIMARY
        }
        
        color = QColor(shadow_colors.get(self.variant, Colors.PRIMARY_LIGHT))
        color.setAlpha(0)  # Start with no shadow
        
        self.shadow.setBlurRadius(20)
        self.shadow.setOffset(0, 0)
        self.shadow.setColor(color)
        self.setGraphicsEffect(self.shadow)
    
    def _setup_animation(self):
        """Setup hover animation"""
        self._anim = QPropertyAnimation(self, b"glowIntensity")
        self._anim.setDuration(200)
        self._anim.setEasingCurve(QEasingCurve.Type.OutCubic)
    
    def _get_glow_intensity(self) -> int:
        return self._glow_intensity
    
    def _set_glow_intensity(self, value: int):
        self._glow_intensity = value
        color = self.shadow.color()
        color.setAlpha(value)
        self.shadow.setColor(color)
    
    glowIntensity = pyqtProperty(int, _get_glow_intensity, _set_glow_intensity)
    
    def enterEvent(self, event):
        """Mouse enter - animate glow in"""
        self._anim.stop()
        self._anim.setStartValue(self._glow_intensity)
        self._anim.setEndValue(150)
        self._anim.start()
        super().enterEvent(event)
    
    def leaveEvent(self, event):
        """Mouse leave - animate glow out"""
        self._anim.stop()
        self._anim.setStartValue(self._glow_intensity)
        self._anim.setEndValue(0)
        self._anim.start()
        super().leaveEvent(event)


class IconButton(QPushButton):
    """
    Circular icon button
    """
    
    def __init__(self, icon: QIcon = None, size: int = 40, parent=None):
        super().__init__(parent)
        self.button_size = size
        
        if icon:
            self.setIcon(icon)
            self.setIconSize(QSize(size - 16, size - 16))
        
        self.setStyleSheet(Styles.button_icon(size))
        self.setFixedSize(size, size)
        self.setCursor(Qt.CursorShape.PointingHandCursor)
        
        # Add shadow
        shadow = QGraphicsDropShadowEffect(self)
        shadow.setBlurRadius(15)
        shadow.setOffset(0, 2)
        shadow.setColor(QColor(0, 0, 0, 80))
        self.setGraphicsEffect(shadow)


class ToggleButton(QPushButton):
    """
    Toggle button yang bisa ON/OFF
    """
    
    def __init__(self, text_on: str = "ON", text_off: str = "OFF", parent=None):
        super().__init__(parent)
        self.text_on = text_on
        self.text_off = text_off
        self._is_on = False
        
        self.setCheckable(True)
        self.clicked.connect(self._on_toggle)
        self._update_style()
        self.setCursor(Qt.CursorShape.PointingHandCursor)
    
    def _update_style(self):
        """Update style berdasarkan state"""
        if self._is_on:
            self.setText(self.text_on)
            self.setStyleSheet(f"""
                QPushButton {{
                    background: {Colors.GRADIENT_ACCENT};
                    color: {Colors.SECONDARY_DARKEST};
                    border: none;
                    border-radius: 8px;
                    padding: 12px 24px;
                    font-weight: 700;
                    font-size: 12px;
                }}
                QPushButton:hover {{
                    background: {Colors.ACCENT_LIGHT};
                }}
            """)
        else:
            self.setText(self.text_off)
            self.setStyleSheet(f"""
                QPushButton {{
                    background: {Colors.SECONDARY_DARK};
                    color: {Colors.TEXT_SECONDARY};
                    border: 2px solid {Colors.BORDER_DEFAULT};
                    border-radius: 8px;
                    padding: 10px 22px;
                    font-weight: 600;
                    font-size: 12px;
                }}
                QPushButton:hover {{
                    background: {Colors.SECONDARY};
                    border-color: {Colors.SECONDARY_LIGHT};
                    color: {Colors.TEXT_PRIMARY};
                }}
            """)
    
    def _on_toggle(self):
        self._is_on = not self._is_on
        self._update_style()
    
    @property
    def is_on(self) -> bool:
        return self._is_on
    
    @is_on.setter
    def is_on(self, value: bool):
        self._is_on = value
        self._update_style()
