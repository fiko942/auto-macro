"""
Switch Components untuk Gaming Tool
Toggle switches dengan animasi
"""
from PyQt6.QtWidgets import QWidget, QHBoxLayout, QLabel
from PyQt6.QtCore import Qt, QPropertyAnimation, QEasingCurve, pyqtProperty, pyqtSignal, QRectF
from PyQt6.QtGui import QPainter, QColor, QBrush

import sys
sys.path.append('..')
from src.theme import Colors


class GamingSwitch(QWidget):
    """Animated toggle switch"""
    
    toggled = pyqtSignal(bool)
    
    def __init__(self, parent=None, label: str = None):
        super().__init__(parent)
        self._checked = False
        self._handle_position = 4
        self.label_text = label
        
        self._setup_ui()
        self._setup_animation()
        self.setCursor(Qt.CursorShape.PointingHandCursor)
    
    def _setup_ui(self):
        if self.label_text:
            layout = QHBoxLayout(self)
            layout.setContentsMargins(0, 0, 0, 0)
            layout.setSpacing(12)
            
            label = QLabel(self.label_text)
            label.setStyleSheet(f"color: {Colors.TEXT_PRIMARY}; background: transparent;")
            layout.addWidget(label)
            layout.addStretch()
            
            self.setMinimumHeight(28)
        else:
            self.setFixedSize(52, 28)
    
    def _setup_animation(self):
        self._anim = QPropertyAnimation(self, b"handlePosition")
        self._anim.setDuration(150)
        self._anim.setEasingCurve(QEasingCurve.Type.OutCubic)
    
    def _get_handle_position(self) -> int:
        return self._handle_position
    
    def _set_handle_position(self, value: int):
        self._handle_position = value
        self.update()
    
    handlePosition = pyqtProperty(int, _get_handle_position, _set_handle_position)
    
    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing)
        
        # Calculate switch area
        if self.label_text:
            switch_x = self.width() - 52
            switch_rect = QRectF(switch_x, 0, 52, 28)
        else:
            switch_rect = QRectF(0, 0, 52, 28)
        
        # Background
        if self._checked:
            painter.setBrush(QBrush(QColor(Colors.ACCENT)))
        else:
            painter.setBrush(QBrush(QColor(Colors.SECONDARY_LIGHT)))
        painter.setPen(Qt.PenStyle.NoPen)
        painter.drawRoundedRect(switch_rect, 14, 14)
        
        # Handle
        handle_x = switch_rect.x() + self._handle_position
        handle_rect = QRectF(handle_x, 4, 20, 20)
        painter.setBrush(QBrush(QColor(Colors.TEXT_PRIMARY)))
        painter.drawEllipse(handle_rect)
    
    def mousePressEvent(self, event):
        self._checked = not self._checked
        self._anim.stop()
        self._anim.setStartValue(self._handle_position)
        self._anim.setEndValue(28 if self._checked else 4)
        self._anim.start()
        self.toggled.emit(self._checked)
    
    @property
    def isChecked(self) -> bool:
        return self._checked
    
    def setChecked(self, checked: bool):
        self._checked = checked
        self._handle_position = 28 if checked else 4
        self.update()
