"""
Hotkey Page - UI Layer
Hanya berisi layout dan binding ke controller, tidak ada business logic
"""
from PyQt6.QtWidgets import (
    QWidget, QVBoxLayout, QHBoxLayout, QLabel, QScrollArea,
    QFileDialog, QMessageBox, QInputDialog
)
from PyQt6.QtCore import Qt

from src.theme import Colors
from src.components.buttons import GamingButton, ToggleButton
from src.components.labels import TitleLabel
from src.controllers.hotkey_controller import HotkeyController
from src.widgets.hotkey_widgets import HotkeyItemWidget, AddEditHotkeyDialog


class HotkeyPage(QWidget):
    """
    Gaming Hotkey Page
    UI layer yang menggunakan HotkeyController untuk semua logic
    """
    
    def __init__(self, controller, parent=None):
        super().__init__(parent)
        
        # Controller
        self.controller = controller
        
        # Setup
        self._setup_ui()
        self._connect_signals()
    
    def _setup_ui(self):
        """Setup UI layout"""
        self.setStyleSheet(f"background: {Colors.SECONDARY_DARKEST};")
        
        layout = QVBoxLayout(self)
        layout.setContentsMargins(32, 32, 32, 32)
        layout.setSpacing(24)
        
        # Header Section
        self._create_header(layout)
        
        # Toolbar Section
        self._create_toolbar(layout)
        
        # Content Section (Hotkey List)
        self._create_content(layout)
    
    def _create_header(self, parent_layout: QVBoxLayout):
        """Create header with title, status, and toggle"""
        header = QHBoxLayout()
        
        # Title
        title = TitleLabel("Gaming Hotkey", level=1)
        header.addWidget(title)
        header.addStretch()
        
        # Status
        self.status_label = QLabel("● Inactive")
        self._update_status_style(False)
        header.addWidget(self.status_label)
        
        # Master Toggle
        self.master_toggle = ToggleButton("START", "STOP")
        self.master_toggle.clicked.connect(self._on_toggle_clicked)
        header.addWidget(self.master_toggle)
        
        parent_layout.addLayout(header)
    
    def _create_toolbar(self, parent_layout: QVBoxLayout):
        """Create toolbar with action buttons"""
        toolbar = QHBoxLayout()
        
        # Add Hotkey
        add_btn = GamingButton("+ Add Hotkey", "primary")
        add_btn.clicked.connect(self._on_add_clicked)
        toolbar.addWidget(add_btn)
        
        toolbar.addSpacing(20)
        
        
        toolbar.addSpacing(20)
        
        # Import Button (untuk load file single hotkey)
        import_btn = GamingButton("Import Hotkey", "secondary", "small")
        import_btn.clicked.connect(self._on_import)
        toolbar.addWidget(import_btn)
        
        toolbar.addStretch()
        
        toolbar.addStretch()
        parent_layout.addLayout(toolbar)
    
    def _create_content(self, parent_layout: QVBoxLayout):
        """Create scrollable content area"""
        scroll = QScrollArea()
        scroll.setWidgetResizable(True)
        scroll.setHorizontalScrollBarPolicy(Qt.ScrollBarPolicy.ScrollBarAlwaysOff)
        scroll.setStyleSheet("QScrollArea { background: transparent; border: none; }")
        
        self.list_container = QWidget()
        self.list_container.setStyleSheet("background: transparent;")
        self.list_layout = QVBoxLayout(self.list_container)
        self.list_layout.setContentsMargins(0, 0, 0, 0)
        self.list_layout.setSpacing(12)
        self.list_layout.setAlignment(Qt.AlignmentFlag.AlignTop)
        
        scroll.setWidget(self.list_container)
        parent_layout.addWidget(scroll, 1)
        
        # Initial refresh
        self._refresh_list()
    
    def _connect_signals(self):
        """Connect controller signals ke UI handlers"""
        self.controller.bindingsChanged.connect(self._refresh_list)
        self.controller.statusChanged.connect(self._on_status_changed)
        self.controller.error.connect(self._show_error)
        self.controller.success.connect(self._show_success)
    
    # ===================
    # UI UPDATE METHODS
    # ===================
    
    def _refresh_list(self):
        """Refresh hotkey list dari controller"""
        # Clear current items
        while self.list_layout.count():
            item = self.list_layout.takeAt(0)
            if item.widget():
                item.widget().deleteLater()
        
        bindings = self.controller.bindings
        
        if not bindings:
            # Empty state
            empty_label = QLabel("No hotkeys configured yet.\nClick '+ Add Hotkey' to create your first hotkey.")
            empty_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
            empty_label.setStyleSheet(f"""
                QLabel {{
                    color: {Colors.TEXT_MUTED};
                    font-size: 14px;
                    padding: 60px;
                    background: transparent;
                }}
            """)
            self.list_layout.addWidget(empty_label)
        else:
            # Add items
            for binding in bindings:
                item = HotkeyItemWidget(binding)
                item.editClicked.connect(self._on_edit_clicked)
                item.deleteClicked.connect(self._on_delete_clicked)
                item.exportClicked.connect(self._on_export_item)
                item.toggleClicked.connect(self._on_item_toggle)
                self.list_layout.addWidget(item)
            
            self.list_layout.addStretch()
    
    def _update_status_style(self, active: bool):
        """Update status label style"""
        if active:
            self.status_label.setText("● Active")
            self.status_label.setStyleSheet(f"""
                QLabel {{
                    color: {Colors.SUCCESS};
                    font-size: 13px;
                    font-weight: 500;
                    background: transparent;
                }}
            """)
        else:
            self.status_label.setText("● Inactive")
            self.status_label.setStyleSheet(f"""
                QLabel {{
                    color: {Colors.TEXT_MUTED};
                    font-size: 13px;
                    font-weight: 500;
                    background: transparent;
                }}
            """)
    
    def _show_error(self, message: str):
        """Show error message"""
        QMessageBox.warning(self, "Error", message)
    
    def _show_success(self, message: str):
        """Show success message"""
        QMessageBox.information(self, "Success", message)
    
    # ===================
    # EVENT HANDLERS
    # ===================
    
    def _on_status_changed(self, active: bool):
        """Handle status change from controller"""
        self._update_status_style(active)
    
    def _on_toggle_clicked(self):
        """Handle master toggle click"""
        self.controller.toggle_active()
    
    def _on_add_clicked(self):
        """Handle add button click"""
        dialog = AddEditHotkeyDialog(parent=self)
        if dialog.exec() and dialog.result_data:
            data = dialog.result_data
            self.controller.add_binding(
                name=data["name"],
                trigger_keys=data["trigger_keys"],
                actions=data["actions"],
                repeat=data["repeat"],
                repeat_delay=data["repeat_delay"],
                block_input=data["block_input"]
            )
    
    def _on_edit_clicked(self, binding_id: str):
        """Handle edit button click"""
        binding = self.controller.get_binding(binding_id)
        if binding:
            dialog = AddEditHotkeyDialog(binding, parent=self)
            if dialog.exec() and dialog.result_data:
                data = dialog.result_data
                self.controller.update_binding(
                    binding_id=binding_id,
                    name=data["name"],
                    trigger_keys=data["trigger_keys"],
                    actions=data["actions"],
                    repeat=data["repeat"],
                    repeat_delay=data["repeat_delay"],
                    block_input=data["block_input"]
                )
    
    def _on_delete_clicked(self, binding_id: str):
        """Handle delete button click"""
        reply = QMessageBox.question(
            self, "Delete Hotkey",
            "Are you sure you want to delete this hotkey?",
            QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No
        )
        if reply == QMessageBox.StandardButton.Yes:
            self.controller.remove_binding(binding_id)
    
    def _on_item_toggle(self, binding_id: str, enabled: bool):
        """Handle item enable/disable toggle"""
        self.controller.toggle_binding(binding_id, enabled)
    
    def _on_import(self):
        """Handle import single hotkey click"""
        filepath, _ = QFileDialog.getOpenFileName(
            self, "Import Hotkey", "", "JSON Files (*.json)"
        )
        if filepath:
            self.controller.import_hotkey(filepath)

    def _on_export_item(self, binding_id: str):
        """Handle export item click"""
        # Suggest filename based on binding name
        binding = self.controller.get_binding(binding_id)
        if not binding:
            return
            
        default_name = f"{binding.name.replace(' ', '_').lower()}.json"
        
        filepath, _ = QFileDialog.getSaveFileName(
            self, "Export Hotkey", default_name, "JSON Files (*.json)"
        )
        if filepath:
            self.controller.export_hotkey(binding_id, filepath)
