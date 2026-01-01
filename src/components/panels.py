"""
Panel Components untuk Gaming Tool
"""
from PyQt6.QtWidgets import (QFrame, QVBoxLayout, QHBoxLayout, QLabel,
                              QGraphicsDropShadowEffect, QScrollArea, QWidget)
from PyQt6.QtCore import Qt, pyqtSignal
from PyQt6.QtGui import QColor

import sys
sys.path.append('..')
from src.theme import Colors


class GamingPanel(QFrame):
    """Base panel dengan gaming style"""
    
    def __init__(self, parent=None, title: str = None):
        super().__init__(parent)
        self.title_text = title
        
        self._setup_style()
        self._setup_layout()
        self._setup_shadow()
    
    def _setup_style(self):
        self.setStyleSheet(f"""
            GamingPanel {{
                background: {Colors.SECONDARY_DARKER};
                border: 1px solid {Colors.BORDER_DEFAULT};
                border-radius: 16px;
            }}
        """)
    
    def _setup_layout(self):
        self.main_layout = QVBoxLayout(self)
        self.main_layout.setContentsMargins(24, 24, 24, 24)
        self.main_layout.setSpacing(16)
        
        if self.title_text:
            title_container = QWidget()
            title_container.setStyleSheet("background: transparent;")
            title_layout = QVBoxLayout(title_container)
            title_layout.setContentsMargins(0, 0, 0, 12)
            title_layout.setSpacing(0)
            
            title_label = QLabel(self.title_text)
            title_label.setStyleSheet(f"""
                QLabel {{
                    color: {Colors.TEXT_PRIMARY};
                    font-size: 16px;
                    font-weight: 700;
                    background: transparent;
                    border: none;
                }}
            """)
            title_layout.addWidget(title_label)
            
            # Separator line
            separator = QFrame()
            separator.setFixedHeight(1)
            separator.setStyleSheet(f"background: {Colors.BORDER_DEFAULT};")
            title_layout.addWidget(separator)
            
            self.main_layout.addWidget(title_container)
        
        self.content_layout = QVBoxLayout()
        self.content_layout.setSpacing(12)
        self.main_layout.addLayout(self.content_layout)
    
    def _setup_shadow(self):
        shadow = QGraphicsDropShadowEffect(self)
        shadow.setBlurRadius(20)
        shadow.setOffset(0, 4)
        shadow.setColor(QColor(0, 0, 0, 50))
        self.setGraphicsEffect(shadow)
    
    def add_widget(self, widget):
        self.content_layout.addWidget(widget)
    
    def add_layout(self, layout):
        self.content_layout.addLayout(layout)
    
    def add_stretch(self):
        self.content_layout.addStretch()


class GlassPanel(QFrame):
    """Glass morphism panel"""
    
    def __init__(self, parent=None):
        super().__init__(parent)
        self._setup_style()
        self._setup_layout()
    
    def _setup_style(self):
        self.setStyleSheet(f"""
            GlassPanel {{
                background: {Colors.GLASS_BACKGROUND};
                border: 1px solid {Colors.rgba(Colors.PRIMARY_LIGHT, 0.3)};
                border-radius: 20px;
            }}
        """)
    
    def _setup_layout(self):
        self.layout = QVBoxLayout(self)
        self.layout.setContentsMargins(24, 24, 24, 24)
        self.layout.setSpacing(16)
    
    def add_widget(self, widget):
        self.layout.addWidget(widget)
    
    def add_layout(self, layout):
        self.layout.addLayout(layout)


class SidebarPanel(QFrame):
    """Sidebar navigation panel"""
    
    def __init__(self, parent=None, width: int = 260):
        super().__init__(parent)
        self.setFixedWidth(width)
        self._setup_style()
        self._setup_layout()
    
    def _setup_style(self):
        self.setStyleSheet(f"""
            SidebarPanel {{
                background: {Colors.SECONDARY_DARKEST};
                border-right: 1px solid {Colors.BORDER_DEFAULT};
            }}
        """)
    
    def _setup_layout(self):
        main_layout = QVBoxLayout(self)
        main_layout.setContentsMargins(0, 0, 0, 0)
        main_layout.setSpacing(0)
        
        # Header
        self.header_widget = QFrame()
        self.header_widget.setStyleSheet(f"""
            QFrame {{
                background: transparent;
                border: none;
                border-bottom: 1px solid {Colors.BORDER_DEFAULT};
            }}
        """)
        self.header_layout = QVBoxLayout(self.header_widget)
        self.header_layout.setContentsMargins(24, 24, 24, 24)
        main_layout.addWidget(self.header_widget)
        
        # Scrollable content
        scroll_area = QScrollArea()
        scroll_area.setWidgetResizable(True)
        scroll_area.setHorizontalScrollBarPolicy(Qt.ScrollBarPolicy.ScrollBarAlwaysOff)
        scroll_area.setVerticalScrollBarPolicy(Qt.ScrollBarPolicy.ScrollBarAsNeeded)
        scroll_area.setStyleSheet(f"""
            QScrollArea {{
                background: transparent;
                border: none;
            }}
            QScrollArea > QWidget > QWidget {{
                background: transparent;
            }}
        """)
        
        scroll_content = QWidget()
        scroll_content.setStyleSheet("background: transparent;")
        self.content_layout = QVBoxLayout(scroll_content)
        self.content_layout.setContentsMargins(16, 20, 16, 20)
        self.content_layout.setSpacing(6)
        self.content_layout.setAlignment(Qt.AlignmentFlag.AlignTop)
        
        scroll_area.setWidget(scroll_content)
        main_layout.addWidget(scroll_area, 1)
        
        # Footer
        self.footer_widget = QFrame()
        self.footer_widget.setStyleSheet(f"""
            QFrame {{
                background: transparent;
                border: none;
                border-top: 1px solid {Colors.BORDER_DEFAULT};
            }}
        """)
        self.footer_layout = QVBoxLayout(self.footer_widget)
        self.footer_layout.setContentsMargins(24, 16, 24, 16)
        main_layout.addWidget(self.footer_widget)
    
    def set_header(self, widget):
        while self.header_layout.count():
            item = self.header_layout.takeAt(0)
            if item.widget():
                item.widget().deleteLater()
        self.header_layout.addWidget(widget)
    
    def add_nav_item(self, widget):
        self.content_layout.addWidget(widget)
    
    def set_footer(self, widget):
        while self.footer_layout.count():
            item = self.footer_layout.takeAt(0)
            if item.widget():
                item.widget().deleteLater()
        self.footer_layout.addWidget(widget)


class NavItem(QFrame):
    """Navigation item untuk sidebar"""
    
    clicked = pyqtSignal()
    
    def __init__(self, text: str, icon_text: str = "●", active: bool = False, parent=None):
        super().__init__(parent)
        self.text = text
        self.icon_text = icon_text
        self._active = active
        
        self._setup_ui()
        self._update_style()
        self.setCursor(Qt.CursorShape.PointingHandCursor)
        
    def mouseReleaseEvent(self, event):
        """Handle click event"""
        if event.button() == Qt.MouseButton.LeftButton:
            self.clicked.emit()
            
    def set_active(self, active: bool):
        """Set active state"""
        self.active = active
    
    def _setup_ui(self):
        layout = QHBoxLayout(self)
        layout.setContentsMargins(16, 14, 16, 14)
        layout.setSpacing(14)
        
        self.icon_label = QLabel(self.icon_text)
        self.icon_label.setFixedWidth(24)
        self.icon_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        
        self.text_label = QLabel(self.text)
        
        layout.addWidget(self.icon_label)
        layout.addWidget(self.text_label)
        layout.addStretch()
    
    def _update_style(self):
        if self._active:
            self.setStyleSheet(f"""
                NavItem {{
                    background: {Colors.PRIMARY_DARK};
                    border-radius: 12px;
                    border: none;
                    border-left: 3px solid {Colors.ACCENT_LIGHT};
                }}
            """)
            self.icon_label.setStyleSheet(f"""
                QLabel {{
                    color: {Colors.ACCENT_LIGHT};
                    font-size: 16px;
                    background: transparent;
                    border: none;
                }}
            """)
            self.text_label.setStyleSheet(f"""
                QLabel {{
                    color: {Colors.TEXT_PRIMARY};
                    font-size: 14px;
                    font-weight: 600;
                    background: transparent;
                    border: none;
                }}
            """)
        else:
            self.setStyleSheet(f"""
                NavItem {{
                    background: transparent;
                    border-radius: 12px;
                    border: none;
                }}
                NavItem:hover {{
                    background: {Colors.rgba(Colors.PRIMARY, 0.15)};
                }}
            """)
            self.icon_label.setStyleSheet(f"""
                QLabel {{
                    color: {Colors.TEXT_MUTED};
                    font-size: 16px;
                    background: transparent;
                    border: none;
                }}
            """)
            self.text_label.setStyleSheet(f"""
                QLabel {{
                    color: {Colors.TEXT_SECONDARY};
                    font-size: 14px;
                    font-weight: 500;
                    background: transparent;
                    border: none;
                }}
            """)
    
    @property
    def active(self) -> bool:
        return self._active
    
    @active.setter
    def active(self, value: bool):
        self._active = value
        self._update_style()
