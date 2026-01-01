"""
Hotkey Widgets - UI Components dengan Qt Native Input Capture
"""
from PyQt6.QtWidgets import (
    QWidget, QVBoxLayout, QHBoxLayout, QLabel, QPushButton,
    QFrame, QDialog, QLineEdit, QSpinBox, QListWidget, QListWidgetItem, QAbstractItemView,
    QCheckBox, QMessageBox, QApplication, QComboBox, QSlider
)
from PyQt6.QtCore import Qt, pyqtSignal, QEvent, QSize
from PyQt6.QtGui import QKeyEvent, QMouseEvent

from src.theme import Colors
from src.components.controls import GamingCheckbox
from src.components.switches import GamingSwitch
from src.core.hotkey_manager import HotkeyBinding, KeyAction, ActionType


# Complete Qt key mapping
QT_KEY_MAP = {
    # Letters
    Qt.Key.Key_A: 'a', Qt.Key.Key_B: 'b', Qt.Key.Key_C: 'c', Qt.Key.Key_D: 'd',
    Qt.Key.Key_E: 'e', Qt.Key.Key_F: 'f', Qt.Key.Key_G: 'g', Qt.Key.Key_H: 'h',
    Qt.Key.Key_I: 'i', Qt.Key.Key_J: 'j', Qt.Key.Key_K: 'k', Qt.Key.Key_L: 'l',
    Qt.Key.Key_M: 'm', Qt.Key.Key_N: 'n', Qt.Key.Key_O: 'o', Qt.Key.Key_P: 'p',
    Qt.Key.Key_Q: 'q', Qt.Key.Key_R: 'r', Qt.Key.Key_S: 's', Qt.Key.Key_T: 't',
    Qt.Key.Key_U: 'u', Qt.Key.Key_V: 'v', Qt.Key.Key_W: 'w', Qt.Key.Key_X: 'x',
    Qt.Key.Key_Y: 'y', Qt.Key.Key_Z: 'z',
    # Numbers
    Qt.Key.Key_0: '0', Qt.Key.Key_1: '1', Qt.Key.Key_2: '2', Qt.Key.Key_3: '3',
    Qt.Key.Key_4: '4', Qt.Key.Key_5: '5', Qt.Key.Key_6: '6', Qt.Key.Key_7: '7',
    Qt.Key.Key_8: '8', Qt.Key.Key_9: '9',
    # Function keys
    Qt.Key.Key_F1: 'f1', Qt.Key.Key_F2: 'f2', Qt.Key.Key_F3: 'f3', Qt.Key.Key_F4: 'f4',
    Qt.Key.Key_F5: 'f5', Qt.Key.Key_F6: 'f6', Qt.Key.Key_F7: 'f7', Qt.Key.Key_F8: 'f8',
    Qt.Key.Key_F9: 'f9', Qt.Key.Key_F10: 'f10', Qt.Key.Key_F11: 'f11', Qt.Key.Key_F12: 'f12',
    # Special keys
    Qt.Key.Key_Space: 'space', 
    Qt.Key.Key_Return: 'enter', 
    Qt.Key.Key_Enter: 'enter',
    Qt.Key.Key_Tab: 'tab', 
    Qt.Key.Key_Backspace: 'backspace', 
    Qt.Key.Key_Delete: 'delete',
    Qt.Key.Key_Insert: 'insert', 
    Qt.Key.Key_Home: 'home', 
    Qt.Key.Key_End: 'end',
    Qt.Key.Key_PageUp: 'pageup', 
    Qt.Key.Key_PageDown: 'pagedown',
    Qt.Key.Key_Left: 'left', 
    Qt.Key.Key_Right: 'right', 
    Qt.Key.Key_Up: 'up', 
    Qt.Key.Key_Down: 'down',
    Qt.Key.Key_CapsLock: 'capslock', 
    Qt.Key.Key_NumLock: 'numlock',
    Qt.Key.Key_Escape: 'esc',
    Qt.Key.Key_Print: 'printscreen',
    Qt.Key.Key_Pause: 'pause',
    Qt.Key.Key_ScrollLock: 'scrolllock',
    # Symbols
    Qt.Key.Key_Minus: 'minus', 
    Qt.Key.Key_Plus: 'plus', 
    Qt.Key.Key_Equal: 'equal',
    Qt.Key.Key_BracketLeft: 'bracketleft', 
    Qt.Key.Key_BracketRight: 'bracketright',
    Qt.Key.Key_Semicolon: 'semicolon', 
    Qt.Key.Key_Apostrophe: 'apostrophe',
    Qt.Key.Key_Comma: 'comma', 
    Qt.Key.Key_Period: 'period', 
    Qt.Key.Key_Slash: 'slash', 
    Qt.Key.Key_Backslash: 'backslash',
    Qt.Key.Key_QuoteLeft: 'grave',
}


class InputCaptureDialog(QDialog):
    """Dialog untuk capture keyboard dan mouse"""
    
    def __init__(self, title: str = "Press any key or mouse button...", 
                 allow_escape: bool = False, parent=None):
        super().__init__(parent)
        self.captured_key = ""
        self.captured_display = ""
        self.allow_escape = allow_escape  # Jika True, Escape juga di-capture
        
        self._setup_ui(title)
        
        # Focus settings
        self.setFocusPolicy(Qt.FocusPolicy.StrongFocus)
        self.setMouseTracking(True)
        
    def showEvent(self, event):
        super().showEvent(event)
        self.activateWindow()
        self.raise_()
        self.setFocus(Qt.FocusReason.ActiveWindowFocusReason)
    
    def _setup_ui(self, title: str):
        self.setWindowTitle("Capture Input")
        self.setFixedSize(420, 200)
        self.setModal(True)
        self.setStyleSheet(f"QDialog {{ background: {Colors.SECONDARY_DARKEST}; }}")
        
        layout = QVBoxLayout(self)
        layout.setAlignment(Qt.AlignmentFlag.AlignCenter)
        layout.setSpacing(16)
        layout.setContentsMargins(30, 30, 30, 30)
        
        icon = QLabel("🎮")
        icon.setStyleSheet("font-size: 32px; background: transparent;")
        icon.setAlignment(Qt.AlignmentFlag.AlignCenter)
        layout.addWidget(icon)
        
        self.label = QLabel(title)
        self.label.setStyleSheet(f"""
            QLabel {{ color: {Colors.ACCENT_LIGHT}; font-size: 15px; font-weight: 600; background: transparent; }}
        """)
        self.label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        layout.addWidget(self.label)
        
        hint = QLabel("Keyboard • Mouse • Combinations (Ctrl+Shift+Key)")
        hint.setStyleSheet(f"QLabel {{ color: {Colors.TEXT_MUTED}; font-size: 11px; background: transparent; }}")
        hint.setAlignment(Qt.AlignmentFlag.AlignCenter)
        layout.addWidget(hint)
        
        layout.addSpacing(10)
        
        cancel_btn = QPushButton("Cancel")
        cancel_btn.clicked.connect(self.reject)
        cancel_btn.setStyleSheet(f"""
            QPushButton {{ background: {Colors.SECONDARY_DARK}; color: {Colors.TEXT_SECONDARY}; 
                border: 1px solid {Colors.BORDER_DEFAULT}; border-radius: 6px; padding: 10px 24px; }}
            QPushButton:hover {{ border-color: {Colors.TEXT_SECONDARY}; }}
        """)
        layout.addWidget(cancel_btn, alignment=Qt.AlignmentFlag.AlignCenter)
    
    def keyPressEvent(self, event: QKeyEvent):
        key = event.key()
        modifiers = event.modifiers()
        
        # Cancel dengan Escape (kecuali allow_escape = True)
        if key == Qt.Key.Key_Escape and not self.allow_escape:
            self.reject()
            return
        
        # Ignore lone modifiers
        if key in (Qt.Key.Key_Control, Qt.Key.Key_Shift, Qt.Key.Key_Alt, Qt.Key.Key_Meta):
            return
        
        # Get key name dari map
        key_enum = Qt.Key(key)
        key_name = QT_KEY_MAP.get(key_enum)
        
        if not key_name:
            # Fallback: gunakan text
            text = event.text()
            if text and text.isprintable():
                key_name = text.lower()
            else:
                return
        
        # Build full key with modifiers
        parts = []
        if modifiers & Qt.KeyboardModifier.ControlModifier:
            parts.append('ctrl')
        if modifiers & Qt.KeyboardModifier.ShiftModifier:
            parts.append('shift')
        if modifiers & Qt.KeyboardModifier.AltModifier:
            parts.append('alt')
        parts.append(key_name)
        
        self.captured_key = '+'.join(parts)
        self.captured_display = '⌨️ ' + ' + '.join(p.upper() for p in parts)
        self.accept()
    
    def mousePressEvent(self, event: QMouseEvent):
        button = event.button()
        modifiers = event.modifiers()
        
        button_map = {
            Qt.MouseButton.LeftButton: ('mouse_left', 'Left Click'),
            Qt.MouseButton.RightButton: ('mouse_right', 'Right Click'),
            Qt.MouseButton.MiddleButton: ('mouse_middle', 'Middle Click'),
            Qt.MouseButton.BackButton: ('mouse_back', 'Back'),
            Qt.MouseButton.ForwardButton: ('mouse_forward', 'Forward'),
        }
        
        if button in button_map:
            key_name, display = button_map[button]
            
            parts = []
            if modifiers & Qt.KeyboardModifier.ControlModifier:
                parts.append('ctrl')
            if modifiers & Qt.KeyboardModifier.ShiftModifier:
                parts.append('shift')
            if modifiers & Qt.KeyboardModifier.AltModifier:
                parts.append('alt')
            
            if parts:
                self.captured_key = '+'.join(parts) + '+' + key_name
                self.captured_display = '🖱️ ' + ' + '.join(p.upper() for p in parts) + ' + ' + display
            else:
                self.captured_key = key_name
                self.captured_display = '🖱️ ' + display
            self.accept()


class AddActionDialog(QDialog):
    """Dialog untuk menambah action dengan pilihan type"""
    
    def __init__(self, action: KeyAction = None, parent=None):
        super().__init__(parent)
        self.result_action = None
        self.edit_action = action
        self._setup_ui()
    
    def _setup_ui(self):
        title_text = "Edit Action" if self.edit_action else "Add Action"
        self.setWindowTitle(title_text)
        self.setFixedSize(450, 380)
        self.setStyleSheet(f"QDialog {{ background: {Colors.SECONDARY_DARKEST}; }}")
        
        layout = QVBoxLayout(self)
        layout.setContentsMargins(24, 24, 24, 24)
        layout.setSpacing(16)
        
        # Title
        title = QLabel(title_text)
        title.setStyleSheet(f"color: {Colors.TEXT_PRIMARY}; font-size: 20px; font-weight: 700; background: transparent;")
        layout.addWidget(title)
        
        # Action type selector
        type_label = QLabel("Action Type")
        type_label.setStyleSheet(f"color: {Colors.TEXT_SECONDARY}; font-weight: 600; background: transparent;")
        layout.addWidget(type_label)
        
        self.type_combo = QComboBox()
        self.type_combo.addItems([
            "🎹 Key Press (press & release)",
            "⬇️ Key Down (press only)",
            "⬆️ Key Up (release only)",
            "⏸️ Delay (wait)"
        ])
        self.type_combo.currentIndexChanged.connect(self._on_type_changed)
        self.type_combo.setStyleSheet(f"""
            QComboBox {{
                background: {Colors.SECONDARY_DARK};
                color: {Colors.TEXT_PRIMARY};
                border: 2px solid {Colors.BORDER_DEFAULT};
                border-radius: 8px;
                padding: 12px;
                font-size: 13px;
            }}
            QComboBox::drop-down {{ border: none; width: 30px; }}
            QComboBox::down-arrow {{ image: none; border: none; }}
            QComboBox QAbstractItemView {{
                background: {Colors.SECONDARY_DARK};
                color: {Colors.TEXT_PRIMARY};
                selection-background-color: {Colors.PRIMARY_DARK};
            }}
        """)
        layout.addWidget(self.type_combo)
        
        # Key capture section
        self.key_section = QWidget()
        key_layout = QVBoxLayout(self.key_section)
        key_layout.setContentsMargins(0, 0, 0, 0)
        
        key_label = QLabel("Key to Execute")
        key_label.setStyleSheet(f"color: {Colors.TEXT_SECONDARY}; font-weight: 600; background: transparent;")
        key_layout.addWidget(key_label)
        
        self.key_btn = QPushButton("Click to capture key...")
        self.key_btn.clicked.connect(self._capture_key)
        self.key_btn.setCursor(Qt.CursorShape.PointingHandCursor)
        self.key_btn.setStyleSheet(f"""
            QPushButton {{
                background: {Colors.SECONDARY_DARK};
                color: {Colors.TEXT_SECONDARY};
                border: 2px dashed {Colors.BORDER_DEFAULT};
                border-radius: 8px;
                padding: 14px;
                font-size: 13px;
            }}
            QPushButton:hover {{ border-color: {Colors.PRIMARY}; color: {Colors.TEXT_PRIMARY}; }}
        """)
        key_layout.addWidget(self.key_btn)
        layout.addWidget(self.key_section)
        
        self._captured_key = ""
        
        # Delay section
        self.delay_section = QWidget()
        delay_layout = QVBoxLayout(self.delay_section)
        delay_layout.setContentsMargins(0, 0, 0, 0)
        delay_layout.setSpacing(12)
        
        delay_header = QHBoxLayout()
        delay_label = QLabel("Delay Duration")
        delay_label.setStyleSheet(f"color: {Colors.TEXT_SECONDARY}; font-weight: 600; background: transparent;")
        delay_header.addWidget(delay_label)
        
        self.delay_value_label = QLabel("100 ms")
        self.delay_value_label.setStyleSheet(f"color: {Colors.ACCENT_LIGHT}; font-weight: 700; background: transparent;")
        delay_header.addWidget(self.delay_value_label)
        delay_header.addStretch()
        delay_layout.addLayout(delay_header)
        
        # Slider
        self.delay_slider = QSlider(Qt.Orientation.Horizontal)
        self.delay_slider.setRange(1, 1000)  # 1-1000 dalam unit yang dipilih
        self.delay_slider.setValue(100)
        self.delay_slider.valueChanged.connect(self._on_delay_changed)
        self.delay_slider.setStyleSheet(f"""
            QSlider::groove:horizontal {{ background: {Colors.SECONDARY_DARK}; height: 8px; border-radius: 4px; }}
            QSlider::handle:horizontal {{ 
                background: {Colors.ACCENT_LIGHT}; width: 20px; height: 20px; margin: -6px 0; border-radius: 10px; 
            }}
            QSlider::sub-page:horizontal {{ 
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 {Colors.PRIMARY}, stop:1 {Colors.ACCENT}); 
                border-radius: 4px; 
            }}
        """)
        delay_layout.addWidget(self.delay_slider)
        
        # Unit selector
        unit_layout = QHBoxLayout()
        unit_label = QLabel("Unit:")
        unit_label.setStyleSheet(f"color: {Colors.TEXT_MUTED}; background: transparent;")
        unit_layout.addWidget(unit_label)
        
        self.unit_combo = QComboBox()
        self.unit_combo.addItems(["Milliseconds (ms)", "Seconds (s)", "Minutes (m)"])
        self.unit_combo.currentIndexChanged.connect(self._on_unit_changed)
        self.unit_combo.setStyleSheet(f"""
            QComboBox {{
                background: {Colors.SECONDARY_DARK};
                color: {Colors.TEXT_PRIMARY};
                border: 1px solid {Colors.BORDER_DEFAULT};
                border-radius: 6px;
                padding: 8px;
            }}
        """)
        unit_layout.addWidget(self.unit_combo)
        unit_layout.addStretch()
        delay_layout.addLayout(unit_layout)
        
        layout.addWidget(self.delay_section)
        self.delay_section.hide()
        
        layout.addStretch()
        
        # Buttons
        btns = QHBoxLayout()
        btns.addStretch()
        
        cancel = QPushButton("Cancel")
        cancel.clicked.connect(self.reject)
        cancel.setStyleSheet(f"""
            QPushButton {{ background: transparent; color: {Colors.TEXT_SECONDARY}; 
                border: 1px solid {Colors.BORDER_DEFAULT}; border-radius: 8px; padding: 12px 24px; }}
        """)
        btns.addWidget(cancel)
        
        add_text = "Save Changes" if self.edit_action else "Add Action"
        add = QPushButton(add_text)
        add.clicked.connect(self._add)
        add.setStyleSheet(f"""
            QPushButton {{ background: {Colors.PRIMARY}; color: white; border: none; 
                border-radius: 8px; padding: 12px 24px; font-weight: 600; }}
            QPushButton:hover {{ background: {Colors.PRIMARY_LIGHT}; }}
        """)
        btns.addWidget(add)
        layout.addLayout(btns)
        
        if self.edit_action:
            self._load_existing_data()
            
    def _load_existing_data(self):
        """Populate fields from edit_action"""
        a = self.edit_action
        action_map = {
            ActionType.KEY_PRESS: 0,
            ActionType.KEY_DOWN: 1,
            ActionType.KEY_UP: 2,
            ActionType.DELAY: 3
        }
        idx = action_map.get(a.action_type, 0)
        self.type_combo.setCurrentIndex(idx)
        
        if idx == 3: # Delay
             ms = a.duration
             if ms >= 60000 and ms % 60000 == 0:
                 self.unit_combo.setCurrentIndex(2) # Minutes
                 self.delay_slider.setValue(max(1, ms // 60000))
             elif ms >= 1000 and ms % 1000 == 0:
                 self.unit_combo.setCurrentIndex(1) # Seconds
                 self.delay_slider.setValue(max(1, ms // 1000))
             else:
                 self.unit_combo.setCurrentIndex(0) # ms
                 self.delay_slider.setValue(max(1, ms))
        else: # Key
             if a.keys:
                 self._captured_key = a.keys[0]
                 self.key_btn.setText(a.keys[0].upper())
                 self.key_btn.setStyleSheet(f"""
                    QPushButton {{
                        background: {Colors.PRIMARY_DARK};
                        color: {Colors.ACCENT_LIGHT};
                        border: 2px solid {Colors.PRIMARY};
                        border-radius: 8px;
                        padding: 14px;
                        font-size: 14px;
                        font-weight: 700;
                    }}
                """)
    
    def _on_type_changed(self, index):
        if index == 3:  # Delay
            self.key_section.hide()
            self.delay_section.show()
        else:
            self.key_section.show()
            self.delay_section.hide()
    
    def _capture_key(self):
        dialog = InputCaptureDialog("Press key to add...", allow_escape=True, parent=self)
        if dialog.exec() == QDialog.DialogCode.Accepted:
            self._captured_key = dialog.captured_key
            self.key_btn.setText(dialog.captured_display)
            self.key_btn.setStyleSheet(f"""
                QPushButton {{
                    background: {Colors.PRIMARY_DARK};
                    color: {Colors.ACCENT_LIGHT};
                    border: 2px solid {Colors.PRIMARY};
                    border-radius: 8px;
                    padding: 14px;
                    font-size: 14px;
                    font-weight: 700;
                }}
            """)
    
    def _on_delay_changed(self, value):
        unit_idx = self.unit_combo.currentIndex()
        if unit_idx == 0:
            self.delay_value_label.setText(f"{value} ms")
        elif unit_idx == 1:
            self.delay_value_label.setText(f"{value} s")
        else:
            self.delay_value_label.setText(f"{value} m")
    
    def _on_unit_changed(self, index):
        # Adjust slider range based on unit
        if index == 0:  # ms
            self.delay_slider.setRange(1, 10000)
            self.delay_slider.setValue(100)
        elif index == 1:  # seconds
            self.delay_slider.setRange(1, 300)
            self.delay_slider.setValue(1)
        else:  # minutes
            self.delay_slider.setRange(1, 60)
            self.delay_slider.setValue(1)
        self._on_delay_changed(self.delay_slider.value())
    
    def _get_delay_ms(self) -> int:
        value = self.delay_slider.value()
        unit_idx = self.unit_combo.currentIndex()
        if unit_idx == 0:
            return value
        elif unit_idx == 1:
            return value * 1000
        else:
            return value * 60000
    
    def _add(self):
        type_idx = self.type_combo.currentIndex()
        
        if type_idx == 3:  # Delay
            self.result_action = KeyAction(ActionType.DELAY, [], self._get_delay_ms())
        else:
            if not self._captured_key:
                QMessageBox.warning(self, "Error", "Please capture a key first.")
                return
            
            action_types = [ActionType.KEY_PRESS, ActionType.KEY_DOWN, ActionType.KEY_UP]
            self.result_action = KeyAction(action_types[type_idx], [self._captured_key])
        
        self.accept()


class KeyCaptureButton(QPushButton):
    """Button untuk capture trigger key"""
    
    keyCaptured = pyqtSignal(str)
    
    def __init__(self, text: str = "Click to set trigger...", parent=None):
        super().__init__(text, parent)
        self._captured_key = ""
        self._captured_display = ""
        
        self._apply_default_style()
        self.clicked.connect(self._open_dialog)
        self.setCursor(Qt.CursorShape.PointingHandCursor)
    
    def _apply_default_style(self):
        self.setStyleSheet(f"""
            QPushButton {{
                background: {Colors.SECONDARY_DARK};
                color: {Colors.TEXT_SECONDARY};
                border: 2px dashed {Colors.BORDER_DEFAULT};
                border-radius: 10px;
                padding: 16px 24px;
                font-size: 13px;
                min-width: 250px;
                min-height: 50px;
            }}
            QPushButton:hover {{ border-color: {Colors.PRIMARY}; color: {Colors.TEXT_PRIMARY}; }}
        """)
    
    def _apply_captured_style(self):
        self.setStyleSheet(f"""
            QPushButton {{
                background: {Colors.PRIMARY_DARK};
                color: {Colors.TEXT_PRIMARY};
                border: 2px solid {Colors.PRIMARY};
                border-radius: 10px;
                padding: 16px 24px;
                font-size: 14px;
                font-weight: 700;
                min-width: 250px;
                min-height: 50px;
            }}
            QPushButton:hover {{ background: {Colors.PRIMARY}; }}
        """)
    
    def _open_dialog(self):
        dialog = InputCaptureDialog("Press trigger key or click mouse...", parent=self.window())
        if dialog.exec() == QDialog.DialogCode.Accepted and dialog.captured_key:
            self._captured_key = dialog.captured_key
            self._captured_display = dialog.captured_display
            self.setText(dialog.captured_display)
            self._apply_captured_style()
            self.keyCaptured.emit(dialog.captured_key)
    
    def setKey(self, key: str, display: str = None):
        self._captured_key = key
        if key:
            self._captured_display = display or key.upper()
            self.setText(self._captured_display)
            self._apply_captured_style()
        else:
            self.setText("Click to set trigger...")
            self._apply_default_style()
    
    def getKey(self) -> str:
        return self._captured_key


class HotkeyItemWidget(QFrame):
    """Widget untuk single hotkey item"""
    
    editClicked = pyqtSignal(str)
    deleteClicked = pyqtSignal(str)
    toggleClicked = pyqtSignal(str, bool)
    exportClicked = pyqtSignal(str)
    
    def __init__(self, binding: HotkeyBinding, parent=None):
        super().__init__(parent)
        self.binding = binding
        self._setup_ui()
    
    def _setup_ui(self):
        self.setStyleSheet(f"""
            HotkeyItemWidget {{ background: {Colors.SECONDARY_DARK}; border: 1px solid {Colors.BORDER_DEFAULT}; border-radius: 12px; }}
            HotkeyItemWidget:hover {{ border-color: {Colors.PRIMARY_DARK}; }}
        """)
        self.setFixedHeight(80)
        
        layout = QHBoxLayout(self)
        layout.setContentsMargins(20, 16, 20, 16)
        layout.setSpacing(16)
        
        self.switch = GamingSwitch()
        self.switch.setChecked(self.binding.enabled)
        self.switch.toggled.connect(lambda c: self.toggleClicked.emit(self.binding.id, c))
        layout.addWidget(self.switch)
        
        info = QVBoxLayout()
        info.setSpacing(4)
        
        name = QLabel(self.binding.name)
        name.setStyleSheet(f"color: {Colors.TEXT_PRIMARY}; font-size: 14px; font-weight: 600; background: transparent;")
        info.addWidget(name)
        
        details = f"{len(self.binding.actions)} action(s)"
        if self.binding.repeat:
            details += " • Repeat"
        # Show trigger count
        trigger_count = len(self.binding.trigger_keys)
        if trigger_count > 1:
            details += f" • {trigger_count} triggers"
        det = QLabel(details)
        det.setStyleSheet(f"color: {Colors.TEXT_MUTED}; font-size: 12px; background: transparent;")
        info.addWidget(det)
        layout.addLayout(info, 1)
        
        # Show first trigger (or count if multiple)
        trigger_text = self.binding.trigger_keys[0].upper() if self.binding.trigger_keys else "NO TRIGGER"
        if len(self.binding.trigger_keys) > 1:
            trigger_text += f" +{len(self.binding.trigger_keys) - 1}"
        
        badge = QLabel(trigger_text)
        badge.setMinimumWidth(100)
        badge.setAlignment(Qt.AlignmentFlag.AlignCenter)
        badge.setStyleSheet(f"""
            QLabel {{ background: {Colors.PRIMARY_DARK}; color: {Colors.ACCENT_LIGHT}; 
                border-radius: 8px; padding: 10px 14px; font-size: 11px; font-weight: 700; }}
        """)
        layout.addWidget(badge)
        
        
        edit = QPushButton("Edit")
        edit.setCursor(Qt.CursorShape.PointingHandCursor)
        edit.setStyleSheet(f"""
            QPushButton {{ background: transparent; color: {Colors.TEXT_SECONDARY}; 
                border: 1px solid {Colors.BORDER_DEFAULT}; border-radius: 6px; padding: 8px 16px; }}
            QPushButton:hover {{ background: {Colors.PRIMARY_DARK}; color: {Colors.TEXT_PRIMARY}; }}
        """)
        edit.clicked.connect(lambda: self.editClicked.emit(self.binding.id))
        layout.addWidget(edit)
        
        export_btn = QPushButton("Exp")
        export_btn.setToolTip("Export this hotkey")
        export_btn.setCursor(Qt.CursorShape.PointingHandCursor)
        export_btn.setStyleSheet(f"""
            QPushButton {{ background: transparent; color: {Colors.TEXT_MUTED}; 
                border: 1px solid {Colors.BORDER_DEFAULT}; border-radius: 6px; padding: 8px; }}
            QPushButton:hover {{ background: {Colors.SECONDARY_DARKER}; color: {Colors.TEXT_PRIMARY}; }}
        """)
        export_btn.clicked.connect(lambda: self.exportClicked.emit(self.binding.id))
        layout.addWidget(export_btn)
        
        delete = QPushButton("✕")
        delete.setFixedSize(36, 36)
        delete.setCursor(Qt.CursorShape.PointingHandCursor)
        delete.setStyleSheet(f"""
            QPushButton {{ background: transparent; color: {Colors.TEXT_MUTED}; 
                border: 1px solid {Colors.BORDER_DEFAULT}; border-radius: 6px; }}
            QPushButton:hover {{ background: {Colors.ERROR}; color: white; }}
        """)
        delete.clicked.connect(lambda: self.deleteClicked.emit(self.binding.id))
        layout.addWidget(delete)



class EditableItemWidget(QWidget):
    """Custom list item widget dengan tombol edit on hover"""
    editClicked = pyqtSignal()
    
    def __init__(self, text, icon_text="🎯", parent=None):
        super().__init__(parent)
        layout = QHBoxLayout(self)
        layout.setContentsMargins(10, 4, 0, 4)
        layout.setSpacing(10)
        
        # Label
        self.label = QLabel(f"{icon_text}  {text}")
        self.label.setStyleSheet(f"background: transparent; border: none; font-weight: 500; color: {Colors.TEXT_PRIMARY};")
        layout.addWidget(self.label)
        
        layout.addStretch()
        
        # Edit Button (Pen)
        self.edit_btn = QPushButton("✐")
        self.edit_btn.setFixedSize(28, 28)
        self.edit_btn.setCursor(Qt.CursorShape.PointingHandCursor)
        self.edit_btn.setToolTip("Edit Trigger")
        self.edit_btn.setStyleSheet(f"""
            QPushButton {{ 
                background: transparent; color: {Colors.TEXT_MUTED}; 
                border: none; font-size: 20px; padding-bottom: 4px;
            }}
            QPushButton:hover {{ background: transparent; color: {Colors.ACCENT_LIGHT}; }}
        """)
        self.edit_btn.clicked.connect(self.editClicked.emit)
        self.edit_btn.hide() # Hide by default
        layout.addWidget(self.edit_btn)
    
    def enterEvent(self, event):
        self.edit_btn.show()
        super().enterEvent(event)
        
    def leaveEvent(self, event):
        self.edit_btn.hide()
        super().leaveEvent(event)


class AddEditHotkeyDialog(QDialog):
    """Dialog untuk add/edit hotkey dengan multiple triggers"""
    
    def __init__(self, binding: HotkeyBinding = None, parent=None):
        super().__init__(parent)
        self.binding = binding
        self.is_edit = binding is not None
        self.actions: list[KeyAction] = []
        self.trigger_keys: list[str] = []  # Multiple triggers
        self.result_data = None
        
        if self.is_edit:
            self.actions = [KeyAction(a.action_type, a.keys.copy(), a.duration) for a in binding.actions]
            self.trigger_keys = binding.trigger_keys.copy()
        
        self._setup_ui()
    
    def _setup_ui(self):
        self.setWindowTitle("Edit Hotkey" if self.is_edit else "Add New Hotkey")
        self.setMinimumSize(560, 700)
        self.setStyleSheet(f"QDialog {{ background: {Colors.SECONDARY_DARKEST}; }}")
        
        layout = QVBoxLayout(self)
        layout.setContentsMargins(28, 28, 28, 28)
        layout.setSpacing(14)
        
        title = QLabel("Edit Hotkey" if self.is_edit else "Add New Hotkey")
        title.setStyleSheet(f"color: {Colors.TEXT_PRIMARY}; font-size: 22px; font-weight: 700; background: transparent;")
        layout.addWidget(title)
        
        # Name
        self._label(layout, "Hotkey Name")
        self.name_input = QLineEdit()
        self.name_input.setPlaceholderText("e.g., Quick Attack Combo")
        if self.is_edit:
            self.name_input.setText(self.binding.name)
        self.name_input.setStyleSheet(f"""
            QLineEdit {{ background: {Colors.SECONDARY_DARK}; color: {Colors.TEXT_PRIMARY}; 
                border: 2px solid {Colors.BORDER_DEFAULT}; border-radius: 8px; padding: 12px; }}
            QLineEdit:focus {{ border-color: {Colors.PRIMARY}; }}
        """)
        layout.addWidget(self.name_input)
        
        # Triggers (multiple)
        self._label(layout, "Triggers - Any of these will activate the hotkey")
        
        self.triggers_list = QListWidget()
        self.triggers_list.setDragDropMode(QAbstractItemView.DragDropMode.InternalMove)
        self.triggers_list.setSelectionMode(QAbstractItemView.SelectionMode.SingleSelection)
        self.triggers_list.setStyleSheet(f"""
            QListWidget {{ background: {Colors.SECONDARY_DARK}; border: 2px solid {Colors.BORDER_DEFAULT}; 
                border-radius: 8px; min-height: 70px; max-height: 100px; outline: 0px; }}
            QListWidget::item {{ background: {Colors.SECONDARY_DARKER}; color: {Colors.TEXT_PRIMARY}; 
                border-radius: 4px; padding: 0px; margin: 2px; min-height: 30px; }}
            QListWidget::item:selected {{ background: {Colors.PRIMARY_DARK}; }}
            QListWidget::item:focus {{ outline: none; }}
        """)
        self.triggers_list.itemDoubleClicked.connect(self._edit_trigger_from_item)
        self.triggers_list.model().rowsMoved.connect(self._on_triggers_reordered)
        self._refresh_triggers()
        layout.addWidget(self.triggers_list)
        
        trigger_btns = QHBoxLayout()
        add_trigger = QPushButton("+ Add Trigger")
        add_trigger.clicked.connect(self._add_trigger)
        add_trigger.setCursor(Qt.CursorShape.PointingHandCursor)
        add_trigger.setStyleSheet(f"""
            QPushButton {{ background: {Colors.PRIMARY_DARK}; color: white; border: none; 
                border-radius: 6px; padding: 6px 12px; }}
            QPushButton:hover {{ background: {Colors.PRIMARY}; }}
        """)
        trigger_btns.addWidget(add_trigger)
        
        remove_trigger = QPushButton("Remove Selected")
        remove_trigger.clicked.connect(self._remove_trigger)
        remove_trigger.setCursor(Qt.CursorShape.PointingHandCursor)
        remove_trigger.setStyleSheet(f"""
            QPushButton {{ background: {Colors.SECONDARY_DARK}; color: {Colors.ERROR_LIGHT}; 
                border: 1px solid {Colors.BORDER_DEFAULT}; border-radius: 6px; padding: 6px 12px; }}
            QPushButton:hover {{ background: {Colors.ERROR}; color: white; border-color: {Colors.ERROR}; }}
        """)
        trigger_btns.addWidget(remove_trigger)
        trigger_btns.addStretch()
        layout.addLayout(trigger_btns)
        
        # Options
        opts = QHBoxLayout()
        # Repeat & Delay
        repeat_checked = self.binding.repeat if self.is_edit else False
        self.repeat_cb = GamingCheckbox("Repeat while held", checked=repeat_checked)
        self.repeat_cb.setMinimumWidth(200)
        opts.addWidget(self.repeat_cb)
        
        opts.addWidget(QLabel("Delay:"))
        self.delay_spin = QSpinBox()
        self.delay_spin.setRange(10, 5000)
        self.delay_spin.setValue(self.binding.repeat_delay if self.is_edit else 100)
        self.delay_spin.setSuffix(" ms")
        self.delay_spin.setStyleSheet(f"""
            QSpinBox {{ background: {Colors.SECONDARY_DARK}; color: {Colors.TEXT_PRIMARY}; 
                border: 2px solid {Colors.BORDER_DEFAULT}; border-radius: 6px; padding: 6px; }}
        """)
        opts.addWidget(self.delay_spin)
        
        opts.addStretch()
        layout.addLayout(opts)
        
        # Actions
        layout.addSpacing(10)
        self._label(layout, "Actions to Execute")
        self.actions_list = QListWidget()
        self.actions_list.setDragDropMode(QAbstractItemView.DragDropMode.InternalMove)
        self.actions_list.setSelectionMode(QAbstractItemView.SelectionMode.SingleSelection)
        self.actions_list.setStyleSheet(f"""
            QListWidget {{ background: {Colors.SECONDARY_DARK}; border: 2px solid {Colors.BORDER_DEFAULT}; border-radius: 8px; outline: 0px; }}
            QListWidget::item {{ background: {Colors.SECONDARY_DARKER}; color: {Colors.TEXT_PRIMARY}; 
                border-radius: 4px; padding: 0px; margin: 2px; min-height: 34px; }}
            QListWidget::item:selected {{ background: {Colors.PRIMARY_DARK}; }}
            QListWidget::item:focus {{ outline: none; }}
        """)
        self.actions_list.itemDoubleClicked.connect(self._edit_action_from_item)
        self.actions_list.model().rowsMoved.connect(self._on_actions_reordered)
        self.actions_list.setMinimumHeight(140)
        self._refresh_actions()
        layout.addWidget(self.actions_list)
        
        # Action buttons container
        btns_container = QWidget()
        btns_layout = QHBoxLayout(btns_container)
        btns_layout.setContentsMargins(0, 16, 0, 0) # Top padding for separation
        btns_layout.setSpacing(12)
        
        add_action = QPushButton("+ Add Action")
        add_action.clicked.connect(self._add_action)
        add_action.setCursor(Qt.CursorShape.PointingHandCursor)
        add_action.setStyleSheet(f"""
            QPushButton {{ background: {Colors.PRIMARY_DARK}; color: white; border: none; 
                border-radius: 6px; padding: 6px 12px; }}
            QPushButton:hover {{ background: {Colors.PRIMARY}; }}
        """)
        btns_layout.addWidget(add_action)
        
        remove = QPushButton("Remove Selected")
        remove.clicked.connect(self._remove)
        remove.setCursor(Qt.CursorShape.PointingHandCursor)
        remove.setStyleSheet(f"""
            QPushButton {{ background: {Colors.SECONDARY_DARK}; color: {Colors.ERROR_LIGHT}; 
                border: 1px solid {Colors.BORDER_DEFAULT}; border-radius: 6px; padding: 6px 12px; }}
            QPushButton:hover {{ background: {Colors.ERROR}; color: white; border-color: {Colors.ERROR}; }}
        """)
        btns_layout.addWidget(remove)
        btns_layout.addStretch()
        
        layout.addWidget(btns_container)
        
        # Block Input (Bottom)
        layout.addSpacing(20)
        checked = self.binding.block_input if self.is_edit else False
        self.block_cb = GamingCheckbox("Block Original Input", checked=checked)
        self.block_cb.setMinimumWidth(200)
        layout.addWidget(self.block_cb)
        
        layout.addStretch()
        
        layout.addStretch()

        # Dialog buttons
        dlg_btns = QHBoxLayout()
        dlg_btns.addStretch()
        
        cancel = QPushButton("Cancel")
        cancel.clicked.connect(self.reject)
        cancel.setStyleSheet(f"""
            QPushButton {{ background: transparent; color: {Colors.TEXT_SECONDARY}; 
                border: 1px solid {Colors.BORDER_DEFAULT}; border-radius: 8px; padding: 12px 24px; }}
        """)
        dlg_btns.addWidget(cancel)
        
        save = QPushButton("Save Hotkey")
        save.clicked.connect(self._save)
        save.setStyleSheet(f"""
            QPushButton {{ background: {Colors.PRIMARY}; color: white; border: none; 
                border-radius: 8px; padding: 12px 24px; font-weight: 600; }}
            QPushButton:hover {{ background: {Colors.PRIMARY_LIGHT}; }}
        """)
        dlg_btns.addWidget(save)
        layout.addLayout(dlg_btns)
    
    def _label(self, layout, text):
        lbl = QLabel(text)
        lbl.setStyleSheet(f"color: {Colors.TEXT_SECONDARY}; font-weight: 600; background: transparent;")
        layout.addWidget(lbl)
    
    def _refresh_triggers(self):
        """Refresh triggers list display"""
        self.triggers_list.clear()
        for i, key in enumerate(self.trigger_keys):
            item = QListWidgetItem(self.triggers_list)
            item.setData(Qt.ItemDataRole.UserRole, key)
            item.setSizeHint(QSize(0, 44))
            
            # Custom Widget
            widget = EditableItemWidget(key.upper(), "🎯", parent=self.triggers_list)
            # Safe binding in loop
            widget.editClicked.connect(lambda idx=i: self._edit_trigger(idx))
            
            self.triggers_list.setItemWidget(item, widget)

    def _edit_trigger_from_item(self, item):
        """Edit trigger from list double click"""
        row = self.triggers_list.row(item)
        if row >= 0:
            self._edit_trigger(row)
            
    def _edit_trigger(self, index):
        """Edit specific trigger"""
        if 0 <= index < len(self.trigger_keys):
            current_key = self.trigger_keys[index]
            dialog = InputCaptureDialog(f"Editing trigger '{current_key}'. Press new key...", parent=self)
            if dialog.exec() == QDialog.DialogCode.Accepted and dialog.captured_key:
                # Update key at index
                self.trigger_keys[index] = dialog.captured_key
                self._refresh_triggers()

    def _on_triggers_reordered(self, parent, start, end, destination, row):
        new_keys = []
        for i in range(self.triggers_list.count()):
            item = self.triggers_list.item(i)
            key = item.data(Qt.ItemDataRole.UserRole)
            if key:
                new_keys.append(key)
        
        self.trigger_keys = new_keys
        self._refresh_triggers()
    
    def _add_trigger(self):
        """Add a new trigger"""
        dialog = InputCaptureDialog("Press trigger key or click mouse...", parent=self)
        if dialog.exec() == QDialog.DialogCode.Accepted and dialog.captured_key:
            if dialog.captured_key not in self.trigger_keys:
                self.trigger_keys.append(dialog.captured_key)
                self._refresh_triggers()
    
    def _remove_trigger(self):
        """Remove selected trigger"""
        row = self.triggers_list.currentRow()
        if row >= 0:
            del self.trigger_keys[row]
            self._refresh_triggers()
    
    def _refresh_actions(self):
        self.actions_list.clear()
        for i, a in enumerate(self.actions):
            text = ""
            icon = "⚡"
            if a.action_type == ActionType.KEY_PRESS:
                icon = "🎹"
                text = f"Key Press: {', '.join(a.keys).upper()}"
            elif a.action_type == ActionType.KEY_DOWN:
                icon = "⬇️"
                text = f"Key Down: {', '.join(a.keys).upper()}"
            elif a.action_type == ActionType.KEY_UP:
                icon = "⬆️"
                text = f"Key Up: {', '.join(a.keys).upper()}"
            elif a.action_type == ActionType.DELAY:
                icon = "⏸️"
                if a.duration >= 60000:
                    text = f"Delay: {a.duration // 60000} minute(s)"
                elif a.duration >= 1000:
                    text = f"Delay: {a.duration // 1000} second(s)"
                else:
                    text = f"Delay: {a.duration} ms"
            
            item = QListWidgetItem(self.actions_list)
            item.setData(Qt.ItemDataRole.UserRole, a)
            item.setSizeHint(QSize(0, 48))
            
            widget = EditableItemWidget(text, icon, parent=self.actions_list)
            widget.editClicked.connect(lambda idx=i: self._edit_action(idx))
            
            self.actions_list.setItemWidget(item, widget)

    def _edit_action_from_item(self, item):
        row = self.actions_list.row(item)
        if row >= 0:
            self._edit_action(row)
            
    def _edit_action(self, index):
        if 0 <= index < len(self.actions):
            current_action = self.actions[index]
            dialog = AddActionDialog(action=current_action, parent=self)
            if dialog.exec() == QDialog.DialogCode.Accepted and dialog.result_action:
                self.actions[index] = dialog.result_action
                self._refresh_actions()

    def _on_actions_reordered(self, parent, start, end, destination, row):
        new_actions = []
        for i in range(self.actions_list.count()):
            item = self.actions_list.item(i)
            a = item.data(Qt.ItemDataRole.UserRole)
            if a:
                new_actions.append(a)
        
        self.actions = new_actions
        # Refresh list to update edit button indices in widgets (closure binding)
        self._refresh_actions()
    
    def _add_action(self):
        dialog = AddActionDialog(parent=self)
        if dialog.exec() == QDialog.DialogCode.Accepted and dialog.result_action:
            self.actions.append(dialog.result_action)
            self._refresh_actions()
    
    def _remove(self):
        row = self.actions_list.currentRow()
        if row >= 0:
            del self.actions[row]
            self._refresh_actions()
    
    def _save(self):
        name = self.name_input.text().strip()
        
        if not name:
            QMessageBox.warning(self, "Error", "Please enter a name.")
            return
        if not self.trigger_keys:
            QMessageBox.warning(self, "Error", "Please add at least one trigger.")
            return
        if not self.actions:
            QMessageBox.warning(self, "Error", "Please add at least one action.")
            return
        
        self.result_data = {
            "id": self.binding.id if self.is_edit else None,
            "name": name,
            "trigger_keys": self.trigger_keys,
            "actions": self.actions,
            "repeat": self.repeat_cb.isChecked(),
            "repeat_delay": self.delay_spin.value(),
            "block_input": self.block_cb.isChecked()
        }
        self.accept()

