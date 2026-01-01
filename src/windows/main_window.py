"""
Main Window - Gaming Tool
"""
from PyQt6.QtWidgets import (
    QMainWindow, QWidget, QVBoxLayout, QHBoxLayout, QLabel
)
from PyQt6.QtCore import Qt

from src.theme import Colors
from src.components.panels import SidebarPanel, NavItem
from src.components.labels import StatusIndicator
from src.pages.hotkey_page import HotkeyPage
from src.pages.settings_page import SettingsPage
from src.controllers.hotkey_controller import HotkeyController


class MainWindow(QMainWindow):
    """Main application window"""
    
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Tobelsoft Macro - Gaming Tool")
        self.setMinimumSize(1200, 800)
        self.resize(1400, 900)
        
        # Initialize Controller (Singleton-like for window scope)
        self.hotkey_controller = HotkeyController(self)
        self.hotkey_controller.statusChanged.connect(self.set_status)
        
        self.current_page = None
        self._setup_ui()
    
    def _setup_ui(self):
        """Setup main UI"""
        central = QWidget()
        self.setCentralWidget(central)
        
        main_layout = QHBoxLayout(central)
        main_layout.setContentsMargins(0, 0, 0, 0)
        main_layout.setSpacing(0)
        
        # Sidebar
        sidebar = self._create_sidebar()
        main_layout.addWidget(sidebar)
        
        # Content area
        self.content_area = QWidget()
        self.content_area.setStyleSheet(f"background: {Colors.SECONDARY_DARKEST};")
        self.content_layout = QVBoxLayout(self.content_area)
        self.content_layout.setContentsMargins(0, 0, 0, 0)
        self.content_layout.setSpacing(0)
        
        # Connect Navigation
        self.nav_hotkey.clicked.connect(lambda: self._switch_to_page("hotkey"))
        self.nav_settings.clicked.connect(lambda: self._switch_to_page("settings"))
        
        # Load default page (Hotkey)
        self._load_page(HotkeyPage(self.hotkey_controller))
        
        main_layout.addWidget(self.content_area, 1)
    
    def _create_sidebar(self) -> SidebarPanel:
        """Create sidebar navigation"""
        sidebar = SidebarPanel(width=260)
        
        # Header - Logo
        header_widget = QWidget()
        header_widget.setStyleSheet("background: transparent;")
        header_layout = QVBoxLayout(header_widget)
        header_layout.setContentsMargins(0, 0, 0, 0)
        header_layout.setSpacing(4)
        
        logo_label = QLabel("⚡ TOBELSOFT")
        logo_label.setStyleSheet(f"""
            QLabel {{
                color: {Colors.ACCENT_LIGHT};
                font-size: 20px;
                font-weight: 800;
                letter-spacing: 3px;
                background: transparent;
                border: none;
            }}
        """)
        
        subtitle = QLabel("Gaming Macro Tool")
        subtitle.setStyleSheet(f"""
            QLabel {{
                color: {Colors.TEXT_MUTED};
                font-size: 11px;
                background: transparent;
                border: none;
            }}
        """)
        
        header_layout.addWidget(logo_label)
        header_layout.addWidget(subtitle)
        sidebar.set_header(header_widget)
        
        # Navigation
        self.nav_hotkey = NavItem("Gaming Hotkey", "⌨️", active=True)
        self.nav_settings = NavItem("Settings", "⚙️", active=False)
        
        sidebar.add_nav_item(self.nav_hotkey)
        sidebar.add_nav_item(self.nav_settings)
        
        # Footer - Status
        footer_widget = QWidget()
        footer_widget.setStyleSheet("background: transparent;")
        footer_layout = QHBoxLayout(footer_widget)
        footer_layout.setContentsMargins(0, 0, 0, 0)
        
        self.status_indicator = StatusIndicator("offline", 10)
        self.status_text = QLabel("Ready")
        self.status_text.setStyleSheet(f"""
            QLabel {{
                color: {Colors.TEXT_MUTED};
                font-size: 11px;
                background: transparent;
                border: none;
            }}
        """)
        
        footer_layout.addWidget(self.status_indicator)
        footer_layout.addWidget(self.status_text)
        footer_layout.addStretch()
        sidebar.set_footer(footer_widget)
        
        return sidebar
    
    def _load_page(self, page: QWidget):
        """Load a page into content area"""
        # Clear current
        if self.current_page:
            self.content_layout.removeWidget(self.current_page)
            self.current_page.deleteLater()
        
        self.current_page = page
        self.content_layout.addWidget(page)
    
    def set_status(self, active: bool):
        """Update sidebar status indicator"""
        if active:
            self.status_indicator.set_status("online")
            self.status_text.setText("Active")
            self.status_text.setStyleSheet(f"""
                QLabel {{
                    color: {Colors.SUCCESS};
                    font-size: 11px;
                    background: transparent;
                    border: none;
                }}
            """)
        else:
            self.status_indicator.set_status("offline")
            self.status_text.setText("Ready")
            self.status_text.setStyleSheet(f"""
                QLabel {{
                    color: {Colors.TEXT_MUTED};
                    font-size: 11px;
                    background: transparent;
                    border: none;
                }}
            """)


    def _switch_to_page(self, page_name: str):
        """Switch active page"""
        if page_name == "hotkey":
            self.nav_hotkey.set_active(True)
            self.nav_settings.set_active(False)
            self._load_page(HotkeyPage(self.hotkey_controller))
        elif page_name == "settings":
            self.nav_hotkey.set_active(False)
            self.nav_settings.set_active(True)
            self._load_page(SettingsPage(self.hotkey_controller))
