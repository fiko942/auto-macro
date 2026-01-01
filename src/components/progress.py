"""
Progress Components untuk Gaming Tool
"""
from PyQt6.QtWidgets import (QProgressBar, QWidget, QVBoxLayout, QHBoxLayout, 
                              QLabel, QSizePolicy)
from PyQt6.QtCore import Qt, pyqtProperty
from PyQt6.QtGui import QPainter, QColor, QPen

import sys
sys.path.append('..')
from src.theme import Colors


class GamingProgressBar(QWidget):
    """Styled progress bar dengan label di samping"""
    
    def __init__(self, parent=None, variant: str = "primary", show_text: bool = True):
        super().__init__(parent)
        self.variant = variant
        self.show_text = show_text
        self._value = 0
        
        self._setup_ui()
        self.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Fixed)
        self.setMinimumHeight(24)
    
    def _setup_ui(self):
        layout = QHBoxLayout(self)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.setSpacing(12)
        
        # Progress bar
        self.bar = QProgressBar()
        self.bar.setMinimum(0)
        self.bar.setMaximum(100)
        self.bar.setTextVisible(False)  # We'll show text separately
        self.bar.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Fixed)
        self.bar.setFixedHeight(12)
        self._apply_bar_style()
        
        layout.addWidget(self.bar, 1)
        
        # Percentage label
        if self.show_text:
            self.percent_label = QLabel("0%")
            self.percent_label.setFixedWidth(45)
            self.percent_label.setAlignment(Qt.AlignmentFlag.AlignRight | Qt.AlignmentFlag.AlignVCenter)
            self.percent_label.setStyleSheet(f"""
                QLabel {{
                    color: {Colors.TEXT_PRIMARY};
                    font-size: 13px;
                    font-weight: 600;
                    background: transparent;
                    border: none;
                }}
            """)
            layout.addWidget(self.percent_label)
    
    def _apply_bar_style(self):
        if self.variant == "accent":
            chunk_bg = Colors.GRADIENT_ACCENT
        else:
            chunk_bg = Colors.GRADIENT_PRIMARY
        
        self.bar.setStyleSheet(f"""
            QProgressBar {{
                background: {Colors.SECONDARY_DARK};
                border: none;
                border-radius: 6px;
            }}
            QProgressBar::chunk {{
                background: {chunk_bg};
                border-radius: 6px;
            }}
        """)
    
    def setValue(self, value: int):
        self._value = max(0, min(100, value))
        self.bar.setValue(self._value)
        if self.show_text:
            self.percent_label.setText(f"{self._value}%")
    
    def value(self) -> int:
        return self._value


class CircularProgress(QWidget):
    """Circular progress indicator"""
    
    def __init__(self, parent=None, size: int = 100, thickness: int = 8):
        super().__init__(parent)
        self._progress = 0
        self._size = size
        self._thickness = thickness
        self.setFixedSize(size, size)
    
    def _get_progress(self) -> int:
        return self._progress
    
    def _set_progress(self, value: int):
        self._progress = max(0, min(100, value))
        self.update()
    
    progress = pyqtProperty(int, _get_progress, _set_progress)
    
    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing)
        
        rect = self.rect()
        adjusted_rect = rect.adjusted(
            self._thickness // 2 + 2, 
            self._thickness // 2 + 2,
            -self._thickness // 2 - 2, 
            -self._thickness // 2 - 2
        )
        
        # Background circle
        bg_pen = QPen(QColor(Colors.SECONDARY_DARK))
        bg_pen.setWidth(self._thickness)
        bg_pen.setCapStyle(Qt.PenCapStyle.RoundCap)
        painter.setPen(bg_pen)
        painter.drawArc(adjusted_rect, 0, 360 * 16)
        
        # Progress arc
        if self._progress > 0:
            progress_pen = QPen(QColor(Colors.ACCENT_LIGHT))
            progress_pen.setWidth(self._thickness)
            progress_pen.setCapStyle(Qt.PenCapStyle.RoundCap)
            painter.setPen(progress_pen)
            span = int(-self._progress * 360 / 100 * 16)
            painter.drawArc(adjusted_rect, 90 * 16, span)
        
        # Text
        painter.setPen(QColor(Colors.TEXT_PRIMARY))
        font = painter.font()
        font.setPointSize(self._size // 5)
        font.setBold(True)
        painter.setFont(font)
        painter.drawText(rect, Qt.AlignmentFlag.AlignCenter, f"{self._progress}%")


class LabeledProgress(QWidget):
    """Progress bar dengan label di atas"""
    
    def __init__(self, label: str = "", parent=None, variant: str = "primary"):
        super().__init__(parent)
        self._setup_ui(label, variant)
        self.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Fixed)
    
    def _setup_ui(self, label: str, variant: str):
        layout = QVBoxLayout(self)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.setSpacing(8)
        
        if label:
            label_widget = QLabel(label)
            label_widget.setStyleSheet(f"""
                QLabel {{
                    color: {Colors.TEXT_SECONDARY};
                    font-size: 12px;
                    font-weight: 500;
                    background: transparent;
                    border: none;
                }}
            """)
            layout.addWidget(label_widget)
        
        self.progress = GamingProgressBar(variant=variant)
        layout.addWidget(self.progress)
    
    def setValue(self, value: int):
        self.progress.setValue(value)
    
    def value(self) -> int:
        return self.progress.value()
