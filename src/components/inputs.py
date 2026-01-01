"""
Input Components untuk Gaming Tool
Text inputs, text areas, spin boxes, combo boxes
"""
from PyQt6.QtWidgets import (QLineEdit, QTextEdit, QSpinBox, QDoubleSpinBox,
                              QComboBox, QFrame, QVBoxLayout, QLabel,
                              QGraphicsDropShadowEffect)
from PyQt6.QtCore import Qt
from PyQt6.QtGui import QColor

import sys
sys.path.append('..')
from src.theme import Colors, Styles


class GamingInput(QFrame):
    """
    Styled text input dengan label
    """
    
    def __init__(self, label: str = None, placeholder: str = "", 
                 password: bool = False, parent=None):
        super().__init__(parent)
        self.label_text = label
        
        self._setup_ui(placeholder, password)
        self._setup_style()
    
    def _setup_ui(self, placeholder: str, password: bool):
        """Setup UI components"""
        layout = QVBoxLayout(self)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.setSpacing(8)
        
        # Label
        if self.label_text:
            label = QLabel(self.label_text)
            label.setStyleSheet(f"""
                QLabel {{
                    color: {Colors.TEXT_SECONDARY};
                    font-size: 12px;
                    font-weight: 600;
                    background: transparent;
                    border: none;
                }}
            """)
            layout.addWidget(label)
        
        # Input
        self.input = QLineEdit()
        self.input.setPlaceholderText(placeholder)
        if password:
            self.input.setEchoMode(QLineEdit.EchoMode.Password)
        self.input.setStyleSheet(Styles.input_default())
        layout.addWidget(self.input)
    
    def _setup_style(self):
        """Style the container"""
        self.setStyleSheet("background: transparent; border: none;")
    
    def text(self) -> str:
        """Get input text"""
        return self.input.text()
    
    def setText(self, text: str):
        """Set input text"""
        self.input.setText(text)
    
    def setPlaceholderText(self, text: str):
        """Set placeholder"""
        self.input.setPlaceholderText(text)
    
    def clear(self):
        """Clear input"""
        self.input.clear()


class GamingTextArea(QFrame):
    """
    Styled text area dengan label
    """
    
    def __init__(self, label: str = None, placeholder: str = "", parent=None):
        super().__init__(parent)
        self.label_text = label
        
        self._setup_ui(placeholder)
    
    def _setup_ui(self, placeholder: str):
        """Setup UI"""
        layout = QVBoxLayout(self)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.setSpacing(8)
        
        # Label
        if self.label_text:
            label = QLabel(self.label_text)
            label.setStyleSheet(f"""
                QLabel {{
                    color: {Colors.TEXT_SECONDARY};
                    font-size: 12px;
                    font-weight: 600;
                    background: transparent;
                    border: none;
                }}
            """)
            layout.addWidget(label)
        
        # Text area
        self.textarea = QTextEdit()
        self.textarea.setPlaceholderText(placeholder)
        self.textarea.setStyleSheet(Styles.textarea_default())
        self.textarea.setMinimumHeight(100)
        layout.addWidget(self.textarea)
        
        self.setStyleSheet("background: transparent; border: none;")
    
    def toPlainText(self) -> str:
        """Get text"""
        return self.textarea.toPlainText()
    
    def setPlainText(self, text: str):
        """Set text"""
        self.textarea.setPlainText(text)
    
    def clear(self):
        """Clear textarea"""
        self.textarea.clear()


class GamingSpinBox(QFrame):
    """
    Styled spin box dengan label
    """
    
    def __init__(self, label: str = None, min_val: int = 0, max_val: int = 100,
                 default: int = 0, suffix: str = "", parent=None):
        super().__init__(parent)
        
        self._setup_ui(label, min_val, max_val, default, suffix)
    
    def _setup_ui(self, label: str, min_val: int, max_val: int, 
                  default: int, suffix: str):
        """Setup UI"""
        layout = QVBoxLayout(self)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.setSpacing(8)
        
        # Label
        if label:
            lbl = QLabel(label)
            lbl.setStyleSheet(f"""
                QLabel {{
                    color: {Colors.TEXT_SECONDARY};
                    font-size: 12px;
                    font-weight: 600;
                    background: transparent;
                    border: none;
                }}
            """)
            layout.addWidget(lbl)
        
        # Spin box
        self.spinbox = QSpinBox()
        self.spinbox.setRange(min_val, max_val)
        self.spinbox.setValue(default)
        if suffix:
            self.spinbox.setSuffix(f" {suffix}")
        self.spinbox.setStyleSheet(Styles.spinbox_default())
        layout.addWidget(self.spinbox)
        
        self.setStyleSheet("background: transparent; border: none;")
    
    def value(self) -> int:
        """Get value"""
        return self.spinbox.value()
    
    def setValue(self, value: int):
        """Set value"""
        self.spinbox.setValue(value)


class GamingComboBox(QFrame):
    """
    Styled combo box dengan label
    """
    
    def __init__(self, label: str = None, items: list = None, parent=None):
        super().__init__(parent)
        
        self._setup_ui(label, items or [])
    
    def _setup_ui(self, label: str, items: list):
        """Setup UI"""
        layout = QVBoxLayout(self)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.setSpacing(8)
        
        # Label
        if label:
            lbl = QLabel(label)
            lbl.setStyleSheet(f"""
                QLabel {{
                    color: {Colors.TEXT_SECONDARY};
                    font-size: 12px;
                    font-weight: 600;
                    background: transparent;
                    border: none;
                }}
            """)
            layout.addWidget(lbl)
        
        # Combo box
        self.combobox = QComboBox()
        self.combobox.addItems(items)
        self.combobox.setStyleSheet(Styles.combobox_default())
        layout.addWidget(self.combobox)
        
        self.setStyleSheet("background: transparent; border: none;")
    
    def currentText(self) -> str:
        """Get current text"""
        return self.combobox.currentText()
    
    def currentIndex(self) -> int:
        """Get current index"""
        return self.combobox.currentIndex()
    
    def setCurrentIndex(self, index: int):
        """Set current index"""
        self.combobox.setCurrentIndex(index)
    
    def addItem(self, text: str):
        """Add item"""
        self.combobox.addItem(text)
    
    def addItems(self, items: list):
        """Add items"""
        self.combobox.addItems(items)
    
    def clear(self):
        """Clear items"""
        self.combobox.clear()
