"""
Form Controls - Checkbox, Radio, Slider (Custom Painted)
"""
from PyQt6.QtWidgets import (QWidget, QSlider, QVBoxLayout, QHBoxLayout, 
                              QLabel, QButtonGroup, QSizePolicy)
from PyQt6.QtCore import Qt, pyqtSignal, QRectF
from PyQt6.QtGui import QPainter, QColor, QPen, QBrush, QPainterPath

import sys
sys.path.append('..')
from src.theme import Colors


class GamingCheckbox(QWidget):
    """Custom painted checkbox"""
    
    stateChanged = pyqtSignal(bool)
    
    def __init__(self, text: str = "", checked: bool = False, parent=None):
        super().__init__(parent)
        self._checked = checked
        self._text = text
        self._hovered = False
        
        self.setFixedHeight(28)
        self.setCursor(Qt.CursorShape.PointingHandCursor)
        self.setMouseTracking(True)
    
    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing)
        
        # Box dimensions
        box_size = 20
        box_x = 4
        box_y = (self.height() - box_size) // 2
        box_rect = QRectF(box_x, box_y, box_size, box_size)
        
        # Draw box
        if self._checked:
            painter.setBrush(QBrush(QColor(Colors.PRIMARY)))
            painter.setPen(QPen(QColor(Colors.PRIMARY), 2))
        else:
            painter.setBrush(Qt.BrushStyle.NoBrush)
            if self._hovered:
                painter.setPen(QPen(QColor(Colors.PRIMARY), 2))
            else:
                painter.setPen(QPen(QColor(Colors.SECONDARY_LIGHT), 2))
        
        painter.drawRoundedRect(box_rect, 4, 4)
        
        # Draw checkmark
        if self._checked:
            painter.setPen(QPen(QColor(Colors.TEXT_PRIMARY), 2.5, Qt.PenStyle.SolidLine, 
                               Qt.PenCapStyle.RoundCap, Qt.PenJoinStyle.RoundJoin))
            
            # Checkmark path
            check_x = box_x + 5
            check_y = box_y + box_size // 2
            
            path = QPainterPath()
            path.moveTo(check_x, check_y)
            path.lineTo(check_x + 4, check_y + 4)
            path.lineTo(check_x + 10, check_y - 4)
            painter.drawPath(path)
        
        # Draw text
        painter.setPen(QColor(Colors.TEXT_PRIMARY))
        font = painter.font()
        font.setPointSize(10)
        painter.setFont(font)
        
        text_x = box_x + box_size + 12
        text_rect = QRectF(text_x, 0, self.width() - text_x, self.height())
        painter.drawText(text_rect, Qt.AlignmentFlag.AlignVCenter, self._text)
    
    def mousePressEvent(self, event):
        self._checked = not self._checked
        self.stateChanged.emit(self._checked)
        self.update()
    
    def enterEvent(self, event):
        self._hovered = True
        self.update()
    
    def leaveEvent(self, event):
        self._hovered = False
        self.update()
    
    def isChecked(self) -> bool:
        return self._checked
    
    def setChecked(self, checked: bool):
        self._checked = checked
        self.update()
    
    def text(self) -> str:
        return self._text


class GamingRadio(QWidget):
    """Custom painted radio button"""
    
    toggled = pyqtSignal(bool)
    clicked = pyqtSignal()
    
    def __init__(self, text: str = "", checked: bool = False, parent=None):
        super().__init__(parent)
        self._checked = checked
        self._text = text
        self._hovered = False
        self._group = None
        
        self.setFixedHeight(28)
        self.setCursor(Qt.CursorShape.PointingHandCursor)
        self.setMouseTracking(True)
    
    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing)
        
        # Circle dimensions
        circle_size = 20
        circle_x = 4
        circle_y = (self.height() - circle_size) // 2
        
        # Outer circle
        if self._hovered:
            painter.setPen(QPen(QColor(Colors.PRIMARY), 2))
        else:
            painter.setPen(QPen(QColor(Colors.SECONDARY_LIGHT), 2))
        painter.setBrush(Qt.BrushStyle.NoBrush)
        painter.drawEllipse(circle_x, circle_y, circle_size, circle_size)
        
        # Inner circle (when checked)
        if self._checked:
            inner_size = 10
            inner_x = circle_x + (circle_size - inner_size) // 2
            inner_y = circle_y + (circle_size - inner_size) // 2
            
            painter.setPen(Qt.PenStyle.NoPen)
            painter.setBrush(QBrush(QColor(Colors.PRIMARY)))
            painter.drawEllipse(inner_x, inner_y, inner_size, inner_size)
        
        # Draw text
        painter.setPen(QColor(Colors.TEXT_PRIMARY))
        font = painter.font()
        font.setPointSize(10)
        painter.setFont(font)
        
        text_x = circle_x + circle_size + 12
        text_rect = QRectF(text_x, 0, self.width() - text_x, self.height())
        painter.drawText(text_rect, Qt.AlignmentFlag.AlignVCenter, self._text)
    
    def mousePressEvent(self, event):
        if not self._checked:
            self._checked = True
            self.toggled.emit(True)
            self.clicked.emit()
            self.update()
    
    def enterEvent(self, event):
        self._hovered = True
        self.update()
    
    def leaveEvent(self, event):
        self._hovered = False
        self.update()
    
    def isChecked(self) -> bool:
        return self._checked
    
    def setChecked(self, checked: bool):
        self._checked = checked
        self.update()
    
    def setGroup(self, group):
        self._group = group


class GamingRadioGroup(QWidget):
    """Group of radio buttons"""
    
    selectionChanged = pyqtSignal(int, str)
    
    def __init__(self, options: list = None, label: str = None, parent=None):
        super().__init__(parent)
        self.options = options or []
        self.label_text = label
        self.radios = []
        self._selected_index = 0
        
        self._setup_ui()
    
    def _setup_ui(self):
        layout = QVBoxLayout(self)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.setSpacing(6)
        
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
        
        for i, option in enumerate(self.options):
            radio = GamingRadio(option, checked=(i == 0))
            radio.clicked.connect(lambda idx=i: self._on_radio_clicked(idx))
            self.radios.append(radio)
            layout.addWidget(radio)
    
    def _on_radio_clicked(self, index: int):
        # Uncheck all others
        for i, radio in enumerate(self.radios):
            radio.setChecked(i == index)
        
        self._selected_index = index
        self.selectionChanged.emit(index, self.options[index])
    
    def selectedIndex(self) -> int:
        return self._selected_index


class GamingSlider(QWidget):
    """Styled slider with label and value"""
    
    valueChanged = pyqtSignal(int)
    
    def __init__(self, label: str = None, min_val: int = 0, max_val: int = 100,
                 default: int = 50, suffix: str = "", parent=None):
        super().__init__(parent)
        self.label_text = label
        self.suffix = suffix
        
        self._setup_ui(min_val, max_val, default)
        self.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Fixed)
    
    def _setup_ui(self, min_val: int, max_val: int, default: int):
        layout = QVBoxLayout(self)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.setSpacing(8)
        
        # Header
        header = QHBoxLayout()
        
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
            header.addWidget(label)
        
        header.addStretch()
        
        self.value_label = QLabel(f"{default}{self.suffix}")
        self.value_label.setStyleSheet(f"""
            QLabel {{
                color: {Colors.ACCENT_LIGHT};
                font-size: 13px;
                font-weight: 700;
                background: transparent;
                border: none;
            }}
        """)
        header.addWidget(self.value_label)
        layout.addLayout(header)
        
        # Slider
        self.slider = QSlider(Qt.Orientation.Horizontal)
        self.slider.setMinimum(min_val)
        self.slider.setMaximum(max_val)
        self.slider.setValue(default)
        self.slider.valueChanged.connect(self._on_value_change)
        
        self.slider.setStyleSheet(f"""
            QSlider::groove:horizontal {{
                background: {Colors.SECONDARY_DARK};
                height: 6px;
                border-radius: 3px;
            }}
            QSlider::handle:horizontal {{
                background: {Colors.ACCENT_LIGHT};
                width: 18px;
                height: 18px;
                margin: -6px 0;
                border-radius: 9px;
            }}
            QSlider::handle:horizontal:hover {{
                background: {Colors.ACCENT_LIGHTER};
            }}
            QSlider::sub-page:horizontal {{
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0, 
                    stop:0 {Colors.PRIMARY}, stop:1 {Colors.ACCENT});
                border-radius: 3px;
            }}
        """)
        
        layout.addWidget(self.slider)
    
    def _on_value_change(self, value: int):
        self.value_label.setText(f"{value}{self.suffix}")
        self.valueChanged.emit(value)
    
    def value(self) -> int:
        return self.slider.value()
    
    def setValue(self, value: int):
        self.slider.setValue(value)


class ChecklistGroup(QWidget):
    """Group of checkboxes"""
    
    selectionChanged = pyqtSignal(list)
    
    def __init__(self, options: list = None, label: str = None, parent=None):
        super().__init__(parent)
        self.options = options or []
        self.label_text = label
        self.checkboxes = []
        
        self._setup_ui()
    
    def _setup_ui(self):
        layout = QVBoxLayout(self)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.setSpacing(6)
        
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
        
        for option in self.options:
            cb = GamingCheckbox(option)
            cb.stateChanged.connect(self._on_change)
            self.checkboxes.append(cb)
            layout.addWidget(cb)
    
    def _on_change(self):
        self.selectionChanged.emit(self.selectedItems())
    
    def selectedItems(self) -> list:
        return [cb.text() for cb in self.checkboxes if cb.isChecked()]
