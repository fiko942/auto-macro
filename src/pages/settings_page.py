"""
Settings Page
Manages global configurations including Keymapping.
"""
from PyQt6.QtWidgets import (
    QWidget, QVBoxLayout, QHBoxLayout, QLabel, QListWidget, QMessageBox
)
from PyQt6.QtCore import Qt

from src.theme import Colors
from src.components.labels import TitleLabel
from src.components.buttons import GamingButton
from src.widgets.hotkey_widgets import InputCaptureDialog


class SettingsPage(QWidget):
    """
    Halaman Settings yang menerima controller dari luar
    """
    def __init__(self, controller, parent=None):
        super().__init__(parent)
        self.controller = controller
        self._setup_ui()
        self._load_data()
        
    def _setup_ui(self):
        self.setStyleSheet(f"background: {Colors.SECONDARY_DARKEST};")
        layout = QVBoxLayout(self)
        layout.setContentsMargins(32, 32, 32, 32)
        layout.setSpacing(24)
        
        # Header
        header = TitleLabel("Settings", level=1)
        layout.addWidget(header)
        
        # KEYMAP SECTION wrapped in GamingPanel
        from src.components.panels import GamingPanel
        self.keymap_panel = GamingPanel(title="Global Trigger Keys")
        
        desc = QLabel("Set hotkeys to toggle the main macro system (Start/Stop).\nMultiple keys can be assigned. Pressing ANY of them will toggle the state.")
        desc.setStyleSheet(f"color: {Colors.TEXT_MUTED}; font-size: 13px; line-height: 1.4; margin-bottom: 10px;")
        desc.setWordWrap(True)
        self.keymap_panel.add_widget(desc)
        
        # List of Master Keys (Grid/Icon Mode for Chip look)
        self.keys_list = QListWidget()
        self.keys_list.setViewMode(QListWidget.ViewMode.IconMode)
        self.keys_list.setResizeMode(QListWidget.ResizeMode.Adjust)
        self.keys_list.setMovement(QListWidget.Movement.Static)
        self.keys_list.setSpacing(8)
        self.keys_list.setMinimumHeight(140)
        
        # Keycap Style
        self.keys_list.setStyleSheet(f"""
            QListWidget {{ 
                background: transparent; 
                border: none; 
                outline: none;
            }}
            QListWidget::item {{ 
                background: {Colors.SECONDARY_DARK}; 
                color: {Colors.ACCENT};
                border: 1px solid {Colors.ACCENT_DARK};
                border-radius: 6px; 
                width: 100px; 
                height: 40px;
                margin: 4px;
                font-weight: bold;
                font-size: 14px;
            }}
            QListWidget::item:selected {{ 
                background: {Colors.PRIMARY_DARK}; 
                border: 1px solid {Colors.PRIMARY_LIGHT};
                color: white;
            }}
            QListWidget::item:hover {{ 
                border: 1px solid {Colors.ACCENT};
                background: {Colors.SECONDARY_DARKER};
            }}
        """)
        self.keymap_panel.add_widget(self.keys_list)
        
        # Buttons
        btns = QHBoxLayout()
        btns.setSpacing(12)
        
        add_btn = GamingButton("+ Add Key", "primary", "small")
        add_btn.clicked.connect(self._add_key)
        
        del_btn = GamingButton("Remove", "secondary", "small")
        del_btn.clicked.connect(self._remove_key)
        
        btns.addWidget(add_btn)
        btns.addWidget(del_btn)
        btns.addStretch()
        
        self.keymap_panel.add_layout(btns)
        layout.addWidget(self.keymap_panel)
        
        layout.addStretch()
        
    def _load_data(self):
        """Load current toggle keys from controller"""
        self.keys_list.clear()
        for key in self.controller.master_triggers:
            from PyQt6.QtWidgets import QListWidgetItem
            item = QListWidgetItem(key.upper()) # Display as Uppercase
            item.setTextAlignment(Qt.AlignmentFlag.AlignCenter)
            self.keys_list.addItem(item)
            
    def _add_key(self):
        """Open capture dialog to add key"""
        dialog = InputCaptureDialog(title="Press key to assign...", parent=self)
        if dialog.exec():
            key = dialog.captured_key
            if key:
                # Use a fresh list to update
                current_keys = list(self.controller.master_triggers)
                if key not in current_keys:
                    current_keys.append(key)
                    self.controller.set_master_triggers(current_keys)
                    self._load_data()
                else:
                    QMessageBox.warning(self, "Duplicate", f"Key '{key}' is already in the list.")

    def _remove_key(self):
        """Remove selected key"""
        row = self.keys_list.currentRow()
        if row >= 0:
            # Convert back to lower because we display UPPECASE in UI but store lowercase
            key = self.keys_list.item(row).text().lower()
            current_keys = list(self.controller.master_triggers)
            if key in current_keys:
                current_keys.remove(key)
                self.controller.set_master_triggers(current_keys)
                self._load_data()
        else:
            QMessageBox.information(self, "Info", "Please select a key to remove.")
