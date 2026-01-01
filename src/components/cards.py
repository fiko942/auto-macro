"""
Card Components untuk Gaming Tool
"""
from PyQt6.QtWidgets import (QFrame, QVBoxLayout, QHBoxLayout, QLabel, 
                              QGraphicsDropShadowEffect, QSizePolicy)
from PyQt6.QtCore import Qt
from PyQt6.QtGui import QColor

import sys
sys.path.append('..')
from src.theme import Colors


class GamingCard(QFrame):
    """Base card component"""
    
    def __init__(self, parent=None, highlighted: bool = False):
        super().__init__(parent)
        self.highlighted = highlighted
        
        self._setup_style()
        self._setup_shadow()
        self._setup_layout()
    
    def _setup_style(self):
        if self.highlighted:
            self.setStyleSheet(f"""
                QFrame {{
                    background: {Colors.PRIMARY_DARKEST};
                    border: 2px solid {Colors.PRIMARY};
                    border-radius: 12px;
                }}
            """)
        else:
            self.setStyleSheet(f"""
                QFrame {{
                    background: {Colors.SECONDARY_DARK};
                    border: 1px solid {Colors.BORDER_DEFAULT};
                    border-radius: 12px;
                }}
            """)
    
    def _setup_shadow(self):
        self.shadow = QGraphicsDropShadowEffect(self)
        self.shadow.setBlurRadius(20)
        self.shadow.setOffset(0, 4)
        self.shadow.setColor(QColor(0, 0, 0, 60))
        self.setGraphicsEffect(self.shadow)
    
    def _setup_layout(self):
        self.layout = QVBoxLayout(self)
        self.layout.setContentsMargins(20, 20, 20, 20)
        self.layout.setSpacing(12)
    
    def add_widget(self, widget):
        self.layout.addWidget(widget)
    
    def add_layout(self, layout):
        self.layout.addLayout(layout)


class StatCard(QFrame):
    """Stat card untuk angka/statistik"""
    
    def __init__(self, title: str, value: str, subtitle: str = "", 
                 accent_color: str = None, parent=None):
        super().__init__(parent)
        self.accent_color = accent_color or Colors.PRIMARY_LIGHT
        
        self._setup_style()
        self._setup_content(title, value, subtitle)
        self._setup_shadow()
    
    def _setup_style(self):
        self.setStyleSheet(f"""
            QFrame {{
                background: {Colors.SECONDARY_DARK};
                border: 1px solid {Colors.BORDER_DEFAULT};
                border-radius: 16px;
                border-left: 4px solid {self.accent_color};
            }}
        """)
        self.setMinimumHeight(110)
        self.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Fixed)
    
    def _setup_content(self, title: str, value: str, subtitle: str):
        layout = QVBoxLayout(self)
        layout.setContentsMargins(24, 20, 24, 20)
        layout.setSpacing(6)
        
        # Title
        title_label = QLabel(title)
        title_label.setStyleSheet(f"""
            QLabel {{
                color: {Colors.TEXT_SECONDARY};
                font-size: 12px;
                font-weight: 500;
                background: transparent;
                border: none;
            }}
        """)
        
        # Value
        self.value_label = QLabel(value)
        self.value_label.setStyleSheet(f"""
            QLabel {{
                color: {Colors.TEXT_PRIMARY};
                font-size: 32px;
                font-weight: 700;
                background: transparent;
                border: none;
            }}
        """)
        
        layout.addWidget(title_label)
        layout.addWidget(self.value_label)
        
        if subtitle:
            subtitle_label = QLabel(subtitle)
            subtitle_label.setStyleSheet(f"""
                QLabel {{
                    color: {self.accent_color};
                    font-size: 11px;
                    font-weight: 600;
                    background: transparent;
                    border: none;
                }}
            """)
            layout.addWidget(subtitle_label)
    
    def _setup_shadow(self):
        shadow = QGraphicsDropShadowEffect(self)
        shadow.setBlurRadius(20)
        shadow.setOffset(0, 4)
        shadow.setColor(QColor(0, 0, 0, 50))
        self.setGraphicsEffect(shadow)


class FeatureCard(QFrame):
    """Feature card dengan icon"""
    
    def __init__(self, title: str, description: str, icon_text: str = "★",
                 parent=None):
        super().__init__(parent)
        
        self._setup_style()
        self._setup_content(title, description, icon_text)
        self._setup_shadow()
    
    def _setup_style(self):
        self.setStyleSheet(f"""
            QFrame {{
                background: qlineargradient(x1:0, y1:0, x2:0, y2:1, 
                    stop:0 {Colors.SECONDARY_DARK}, stop:1 {Colors.SECONDARY_DARKER});
                border: 1px solid {Colors.BORDER_DEFAULT};
                border-radius: 16px;
            }}
            QFrame:hover {{
                border-color: {Colors.PRIMARY};
                background: qlineargradient(x1:0, y1:0, x2:0, y2:1, 
                    stop:0 {Colors.PRIMARY_DARKEST}, stop:1 {Colors.SECONDARY_DARK});
            }}
        """)
        self.setMinimumHeight(180)
        self.setMinimumWidth(200)
        self.setCursor(Qt.CursorShape.PointingHandCursor)
        self.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Fixed)
    
    def _setup_content(self, title: str, description: str, icon_text: str):
        layout = QVBoxLayout(self)
        layout.setContentsMargins(24, 24, 24, 24)
        layout.setSpacing(14)
        
        # Icon
        icon_label = QLabel(icon_text)
        icon_label.setFixedSize(52, 52)
        icon_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        icon_label.setStyleSheet(f"""
            QLabel {{
                background: qlineargradient(x1:0, y1:0, x2:1, y2:1, 
                    stop:0 {Colors.PRIMARY}, stop:1 {Colors.ACCENT});
                color: {Colors.TEXT_PRIMARY};
                font-size: 24px;
                border-radius: 14px;
                border: none;
            }}
        """)
        
        # Title
        title_label = QLabel(title)
        title_label.setStyleSheet(f"""
            QLabel {{
                color: {Colors.TEXT_PRIMARY};
                font-size: 15px;
                font-weight: 700;
                background: transparent;
                border: none;
            }}
        """)
        
        # Description
        desc_label = QLabel(description)
        desc_label.setWordWrap(True)
        desc_label.setStyleSheet(f"""
            QLabel {{
                color: {Colors.TEXT_MUTED};
                font-size: 12px;
                line-height: 1.4;
                background: transparent;
                border: none;
            }}
        """)
        
        layout.addWidget(icon_label)
        layout.addWidget(title_label)
        layout.addWidget(desc_label)
        layout.addStretch()
    
    def _setup_shadow(self):
        shadow = QGraphicsDropShadowEffect(self)
        shadow.setBlurRadius(25)
        shadow.setOffset(0, 6)
        shadow.setColor(QColor(0, 0, 0, 60))
        self.setGraphicsEffect(shadow)
