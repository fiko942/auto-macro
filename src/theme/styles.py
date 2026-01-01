"""
Stylesheet Generator untuk Gaming Tool Components
"""
from .colors import Colors


class Styles:
    """Stylesheet templates untuk semua komponen UI"""
    
    # ========================
    # GLOBAL APPLICATION STYLE
    # ========================
    
    @staticmethod
    def app_stylesheet() -> str:
        """Main application stylesheet"""
        return f"""
            QMainWindow {{
                background: {Colors.GRADIENT_DARK};
            }}
            
            QWidget {{
                color: {Colors.TEXT_PRIMARY};
                font-family: 'Segoe UI', Arial, sans-serif;
                font-size: 11px;
            }}
            
            QToolTip {{
                background-color: {Colors.SECONDARY_DARK};
                color: {Colors.TEXT_PRIMARY};
                border: 1px solid {Colors.PRIMARY};
                border-radius: 4px;
                padding: 6px 10px;
            }}
            
            QScrollBar:vertical {{
                background: {Colors.SECONDARY_DARKEST};
                width: 10px;
                border-radius: 5px;
                margin: 0;
            }}
            
            QScrollBar::handle:vertical {{
                background: {Colors.PRIMARY_DARK};
                border-radius: 5px;
                min-height: 30px;
            }}
            
            QScrollBar::handle:vertical:hover {{
                background: {Colors.PRIMARY};
            }}
            
            QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {{
                height: 0;
            }}
            
            QScrollBar:horizontal {{
                background: {Colors.SECONDARY_DARKEST};
                height: 10px;
                border-radius: 5px;
                margin: 0;
            }}
            
            QScrollBar::handle:horizontal {{
                background: {Colors.PRIMARY_DARK};
                border-radius: 5px;
                min-width: 30px;
            }}
            
            QScrollBar::handle:horizontal:hover {{
                background: {Colors.PRIMARY};
            }}
            
            QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {{
                width: 0;
            }}
        """
    
    # ========================
    # BUTTON STYLES
    # ========================
    
    @staticmethod
    def button_primary() -> str:
        """Primary button dengan gradient ungu"""
        return f"""
            QPushButton {{
                background: {Colors.GRADIENT_BUTTON};
                color: {Colors.TEXT_PRIMARY};
                border: none;
                border-radius: 8px;
                padding: 12px 24px;
                font-weight: 600;
                font-size: 12px;
                min-height: 20px;
            }}
            QPushButton:hover {{
                background: {Colors.GRADIENT_BUTTON_HOVER};
            }}
            QPushButton:pressed {{
                background: {Colors.PRIMARY_DARK};
            }}
            QPushButton:disabled {{
                background: {Colors.SECONDARY_DARK};
                color: {Colors.TEXT_DISABLED};
            }}
        """
    
    @staticmethod
    def button_secondary() -> str:
        """Secondary button dengan border"""
        return f"""
            QPushButton {{
                background: transparent;
                color: {Colors.TEXT_PRIMARY};
                border: 2px solid {Colors.PRIMARY};
                border-radius: 8px;
                padding: 10px 22px;
                font-weight: 600;
                font-size: 12px;
                min-height: 20px;
            }}
            QPushButton:hover {{
                background: {Colors.rgba(Colors.PRIMARY, 0.2)};
                border-color: {Colors.PRIMARY_LIGHT};
            }}
            QPushButton:pressed {{
                background: {Colors.rgba(Colors.PRIMARY, 0.3)};
            }}
            QPushButton:disabled {{
                border-color: {Colors.SECONDARY_LIGHT};
                color: {Colors.TEXT_DISABLED};
            }}
        """
    
    @staticmethod
    def button_accent() -> str:
        """Accent button dengan warna cyan"""
        return f"""
            QPushButton {{
                background: {Colors.GRADIENT_ACCENT};
                color: {Colors.SECONDARY_DARKEST};
                border: none;
                border-radius: 8px;
                padding: 12px 24px;
                font-weight: 700;
                font-size: 12px;
                min-height: 20px;
            }}
            QPushButton:hover {{
                background: {Colors.ACCENT_LIGHT};
            }}
            QPushButton:pressed {{
                background: {Colors.ACCENT_DARK};
                color: {Colors.TEXT_PRIMARY};
            }}
            QPushButton:disabled {{
                background: {Colors.SECONDARY_DARK};
                color: {Colors.TEXT_DISABLED};
            }}
        """
    
    @staticmethod
    def button_danger() -> str:
        """Danger button untuk aksi destruktif"""
        return f"""
            QPushButton {{
                background: {Colors.ERROR};
                color: {Colors.TEXT_PRIMARY};
                border: none;
                border-radius: 8px;
                padding: 12px 24px;
                font-weight: 600;
                font-size: 12px;
                min-height: 20px;
            }}
            QPushButton:hover {{
                background: {Colors.ERROR_LIGHT};
            }}
            QPushButton:pressed {{
                background: {Colors.ERROR_DARK};
            }}
            QPushButton:disabled {{
                background: {Colors.SECONDARY_DARK};
                color: {Colors.TEXT_DISABLED};
            }}
        """
    
    @staticmethod
    def button_ghost() -> str:
        """Ghost button - minimal style"""
        return f"""
            QPushButton {{
                background: transparent;
                color: {Colors.TEXT_SECONDARY};
                border: none;
                border-radius: 6px;
                padding: 8px 16px;
                font-weight: 500;
                font-size: 12px;
            }}
            QPushButton:hover {{
                background: {Colors.rgba(Colors.TEXT_PRIMARY, 0.1)};
                color: {Colors.TEXT_PRIMARY};
            }}
            QPushButton:pressed {{
                background: {Colors.rgba(Colors.TEXT_PRIMARY, 0.15)};
            }}
        """
    
    @staticmethod
    def button_icon(size: int = 40) -> str:
        """Icon button - circular"""
        return f"""
            QPushButton {{
                background: {Colors.SECONDARY_DARK};
                color: {Colors.TEXT_PRIMARY};
                border: 1px solid {Colors.BORDER_DEFAULT};
                border-radius: {size // 2}px;
                min-width: {size}px;
                max-width: {size}px;
                min-height: {size}px;
                max-height: {size}px;
                padding: 0;
            }}
            QPushButton:hover {{
                background: {Colors.PRIMARY_DARK};
                border-color: {Colors.PRIMARY};
            }}
            QPushButton:pressed {{
                background: {Colors.PRIMARY};
            }}
        """
    
    # ========================
    # INPUT STYLES
    # ========================
    
    @staticmethod
    def input_default() -> str:
        """Default input/line edit style"""
        return f"""
            QLineEdit {{
                background: {Colors.SECONDARY_DARKEST};
                color: {Colors.TEXT_PRIMARY};
                border: 2px solid {Colors.BORDER_DEFAULT};
                border-radius: 8px;
                padding: 10px 14px;
                font-size: 12px;
                selection-background-color: {Colors.PRIMARY};
            }}
            QLineEdit:hover {{
                border-color: {Colors.PRIMARY_DARK};
            }}
            QLineEdit:focus {{
                border-color: {Colors.PRIMARY};
                background: {Colors.SECONDARY_DARKER};
            }}
            QLineEdit:disabled {{
                background: {Colors.SECONDARY_DARK};
                color: {Colors.TEXT_DISABLED};
                border-color: {Colors.SECONDARY};
            }}
            QLineEdit::placeholder {{
                color: {Colors.TEXT_MUTED};
            }}
        """
    
    @staticmethod
    def textarea_default() -> str:
        """Text area / Text edit style"""
        return f"""
            QTextEdit, QPlainTextEdit {{
                background: {Colors.SECONDARY_DARKEST};
                color: {Colors.TEXT_PRIMARY};
                border: 2px solid {Colors.BORDER_DEFAULT};
                border-radius: 8px;
                padding: 10px;
                font-size: 12px;
                selection-background-color: {Colors.PRIMARY};
            }}
            QTextEdit:hover, QPlainTextEdit:hover {{
                border-color: {Colors.PRIMARY_DARK};
            }}
            QTextEdit:focus, QPlainTextEdit:focus {{
                border-color: {Colors.PRIMARY};
                background: {Colors.SECONDARY_DARKER};
            }}
        """
    
    @staticmethod
    def spinbox_default() -> str:
        """Spin box style"""
        return f"""
            QSpinBox, QDoubleSpinBox {{
                background: {Colors.SECONDARY_DARKEST};
                color: {Colors.TEXT_PRIMARY};
                border: 2px solid {Colors.BORDER_DEFAULT};
                border-radius: 8px;
                padding: 8px 12px;
                font-size: 12px;
            }}
            QSpinBox:hover, QDoubleSpinBox:hover {{
                border-color: {Colors.PRIMARY_DARK};
            }}
            QSpinBox:focus, QDoubleSpinBox:focus {{
                border-color: {Colors.PRIMARY};
            }}
            QSpinBox::up-button, QDoubleSpinBox::up-button {{
                background: {Colors.PRIMARY_DARK};
                border: none;
                border-top-right-radius: 6px;
                width: 20px;
            }}
            QSpinBox::up-button:hover, QDoubleSpinBox::up-button:hover {{
                background: {Colors.PRIMARY};
            }}
            QSpinBox::down-button, QDoubleSpinBox::down-button {{
                background: {Colors.PRIMARY_DARK};
                border: none;
                border-bottom-right-radius: 6px;
                width: 20px;
            }}
            QSpinBox::down-button:hover, QDoubleSpinBox::down-button:hover {{
                background: {Colors.PRIMARY};
            }}
            QSpinBox::up-arrow, QDoubleSpinBox::up-arrow {{
                width: 10px;
                height: 10px;
            }}
            QSpinBox::down-arrow, QDoubleSpinBox::down-arrow {{
                width: 10px;
                height: 10px;
            }}
        """
    
    @staticmethod
    def combobox_default() -> str:
        """Combo box / dropdown style"""
        return f"""
            QComboBox {{
                background: {Colors.SECONDARY_DARKEST};
                color: {Colors.TEXT_PRIMARY};
                border: 2px solid {Colors.BORDER_DEFAULT};
                border-radius: 8px;
                padding: 10px 14px;
                font-size: 12px;
                min-width: 120px;
            }}
            QComboBox:hover {{
                border-color: {Colors.PRIMARY_DARK};
            }}
            QComboBox:focus {{
                border-color: {Colors.PRIMARY};
            }}
            QComboBox::drop-down {{
                border: none;
                width: 30px;
            }}
            QComboBox::down-arrow {{
                width: 12px;
                height: 12px;
            }}
            QComboBox QAbstractItemView {{
                background: {Colors.SECONDARY_DARK};
                color: {Colors.TEXT_PRIMARY};
                border: 2px solid {Colors.PRIMARY_DARK};
                border-radius: 8px;
                selection-background-color: {Colors.PRIMARY};
                outline: none;
            }}
            QComboBox QAbstractItemView::item {{
                padding: 8px 12px;
                min-height: 30px;
            }}
            QComboBox QAbstractItemView::item:hover {{
                background: {Colors.PRIMARY_DARK};
            }}
        """
    
    # ========================
    # CHECKBOX & RADIO
    # ========================
    
    @staticmethod 
    def checkbox_default() -> str:
        """Checkbox style"""
        return f"""
            QCheckBox {{
                color: {Colors.TEXT_PRIMARY};
                spacing: 10px;
                font-size: 12px;
            }}
            QCheckBox::indicator {{
                width: 20px;
                height: 20px;
                border: 2px solid {Colors.BORDER_DEFAULT};
                border-radius: 4px;
                background: {Colors.SECONDARY_DARKEST};
            }}
            QCheckBox::indicator:hover {{
                border-color: {Colors.PRIMARY};
            }}
            QCheckBox::indicator:checked {{
                background: {Colors.PRIMARY};
                border-color: {Colors.PRIMARY};
            }}
            QCheckBox::indicator:checked:hover {{
                background: {Colors.PRIMARY_LIGHT};
                border-color: {Colors.PRIMARY_LIGHT};
            }}
            QCheckBox:disabled {{
                color: {Colors.TEXT_DISABLED};
            }}
            QCheckBox::indicator:disabled {{
                background: {Colors.SECONDARY_DARK};
                border-color: {Colors.SECONDARY};
            }}
        """
    
    @staticmethod
    def radio_default() -> str:
        """Radio button style"""
        return f"""
            QRadioButton {{
                color: {Colors.TEXT_PRIMARY};
                spacing: 10px;
                font-size: 12px;
            }}
            QRadioButton::indicator {{
                width: 20px;
                height: 20px;
                border: 2px solid {Colors.BORDER_DEFAULT};
                border-radius: 10px;
                background: {Colors.SECONDARY_DARKEST};
            }}
            QRadioButton::indicator:hover {{
                border-color: {Colors.PRIMARY};
            }}
            QRadioButton::indicator:checked {{
                background: {Colors.PRIMARY};
                border-color: {Colors.PRIMARY};
            }}
            QRadioButton::indicator:checked:hover {{
                background: {Colors.PRIMARY_LIGHT};
            }}
        """
    
    @staticmethod
    def toggle_switch() -> str:
        """Toggle switch (using checkbox)"""
        return f"""
            QCheckBox {{
                spacing: 12px;
            }}
            QCheckBox::indicator {{
                width: 44px;
                height: 24px;
                border-radius: 12px;
                background: {Colors.SECONDARY_LIGHT};
            }}
            QCheckBox::indicator:checked {{
                background: {Colors.PRIMARY};
            }}
            QCheckBox::indicator:hover {{
                background: {Colors.SECONDARY};
            }}
            QCheckBox::indicator:checked:hover {{
                background: {Colors.PRIMARY_LIGHT};
            }}
        """
    
    # ========================
    # SLIDER & PROGRESS
    # ========================
    
    @staticmethod
    def slider_default() -> str:
        """Slider style"""
        return f"""
            QSlider::groove:horizontal {{
                background: {Colors.SECONDARY_DARK};
                height: 8px;
                border-radius: 4px;
            }}
            QSlider::handle:horizontal {{
                background: {Colors.PRIMARY};
                width: 20px;
                height: 20px;
                margin: -6px 0;
                border-radius: 10px;
            }}
            QSlider::handle:horizontal:hover {{
                background: {Colors.PRIMARY_LIGHT};
            }}
            QSlider::sub-page:horizontal {{
                background: {Colors.GRADIENT_PRIMARY};
                border-radius: 4px;
            }}
            QSlider::add-page:horizontal {{
                background: {Colors.SECONDARY_DARK};
                border-radius: 4px;
            }}
        """
    
    @staticmethod
    def progress_bar() -> str:
        """Progress bar style"""
        return f"""
            QProgressBar {{
                background: {Colors.SECONDARY_DARK};
                border: none;
                border-radius: 6px;
                height: 12px;
                text-align: center;
                color: {Colors.TEXT_PRIMARY};
                font-size: 10px;
                font-weight: 600;
            }}
            QProgressBar::chunk {{
                background: {Colors.GRADIENT_PRIMARY};
                border-radius: 6px;
            }}
        """
    
    @staticmethod
    def progress_bar_accent() -> str:
        """Progress bar dengan warna accent"""
        return f"""
            QProgressBar {{
                background: {Colors.SECONDARY_DARK};
                border: none;
                border-radius: 6px;
                height: 12px;
                text-align: center;
                color: {Colors.SECONDARY_DARKEST};
                font-size: 10px;
                font-weight: 600;
            }}
            QProgressBar::chunk {{
                background: {Colors.GRADIENT_ACCENT};
                border-radius: 6px;
            }}
        """
    
    # ========================
    # PANEL & CARD STYLES
    # ========================
    
    @staticmethod
    def panel_default() -> str:
        """Default panel/frame style"""
        return f"""
            QFrame {{
                background: {Colors.SECONDARY_DARKER};
                border: 1px solid {Colors.BORDER_DEFAULT};
                border-radius: 12px;
            }}
        """
    
    @staticmethod
    def panel_glass() -> str:
        """Glass morphism panel"""
        return f"""
            QFrame {{
                background: {Colors.GLASS_BACKGROUND};
                border: 1px solid {Colors.rgba(Colors.PRIMARY, 0.3)};
                border-radius: 16px;
            }}
        """
    
    @staticmethod 
    def card_default() -> str:
        """Card component style"""
        return f"""
            QFrame {{
                background: {Colors.SECONDARY_DARK};
                border: 1px solid {Colors.BORDER_DEFAULT};
                border-radius: 12px;
                padding: 16px;
            }}
            QFrame:hover {{
                border-color: {Colors.PRIMARY_DARK};
            }}
        """
    
    @staticmethod
    def card_highlighted() -> str:
        """Highlighted card dengan glow effect"""
        return f"""
            QFrame {{
                background: {Colors.PRIMARY_DARKEST};
                border: 2px solid {Colors.PRIMARY};
                border-radius: 12px;
                padding: 16px;
            }}
        """
    
    # ========================
    # GROUP BOX
    # ========================
    
    @staticmethod
    def groupbox_default() -> str:
        """Group box style"""
        return f"""
            QGroupBox {{
                background: {Colors.SECONDARY_DARKER};
                border: 1px solid {Colors.BORDER_DEFAULT};
                border-radius: 12px;
                margin-top: 16px;
                padding: 20px 16px 16px 16px;
                font-weight: 600;
            }}
            QGroupBox::title {{
                subcontrol-origin: margin;
                subcontrol-position: top left;
                left: 16px;
                top: 6px;
                color: {Colors.TEXT_PRIMARY};
                background: {Colors.SECONDARY_DARKER};
                padding: 0 8px;
            }}
        """
    
    # ========================
    # TAB WIDGET
    # ========================
    
    @staticmethod
    def tab_widget() -> str:
        """Tab widget style"""
        return f"""
            QTabWidget::pane {{
                background: {Colors.SECONDARY_DARKER};
                border: 1px solid {Colors.BORDER_DEFAULT};
                border-radius: 0 12px 12px 12px;
                top: -1px;
            }}
            QTabBar::tab {{
                background: {Colors.SECONDARY_DARK};
                color: {Colors.TEXT_SECONDARY};
                border: 1px solid {Colors.BORDER_DEFAULT};
                border-bottom: none;
                border-radius: 8px 8px 0 0;
                padding: 10px 20px;
                margin-right: 4px;
                font-weight: 500;
            }}
            QTabBar::tab:selected {{
                background: {Colors.SECONDARY_DARKER};
                color: {Colors.TEXT_PRIMARY};
                border-color: {Colors.PRIMARY};
            }}
            QTabBar::tab:hover:!selected {{
                background: {Colors.PRIMARY_DARKEST};
                color: {Colors.TEXT_PRIMARY};
            }}
        """
    
    # ========================
    # TABLE & LIST
    # ========================
    
    @staticmethod
    def table_default() -> str:
        """Table widget style"""
        return f"""
            QTableWidget, QTableView {{
                background: {Colors.SECONDARY_DARKEST};
                alternate-background-color: {Colors.SECONDARY_DARKER};
                color: {Colors.TEXT_PRIMARY};
                border: 1px solid {Colors.BORDER_DEFAULT};
                border-radius: 8px;
                gridline-color: {Colors.SECONDARY_DARK};
                selection-background-color: {Colors.PRIMARY_DARK};
            }}
            QTableWidget::item, QTableView::item {{
                padding: 8px;
            }}
            QTableWidget::item:selected, QTableView::item:selected {{
                background: {Colors.PRIMARY_DARK};
            }}
            QHeaderView::section {{
                background: {Colors.SECONDARY_DARK};
                color: {Colors.TEXT_PRIMARY};
                border: none;
                border-bottom: 2px solid {Colors.PRIMARY};
                padding: 10px;
                font-weight: 600;
            }}
            QHeaderView::section:hover {{
                background: {Colors.PRIMARY_DARKEST};
            }}
        """
    
    @staticmethod
    def list_widget() -> str:
        """List widget style"""
        return f"""
            QListWidget {{
                background: {Colors.SECONDARY_DARKEST};
                color: {Colors.TEXT_PRIMARY};
                border: 1px solid {Colors.BORDER_DEFAULT};
                border-radius: 8px;
                outline: none;
            }}
            QListWidget::item {{
                padding: 10px 12px;
                border-bottom: 1px solid {Colors.SECONDARY_DARK};
            }}
            QListWidget::item:hover {{
                background: {Colors.PRIMARY_DARKEST};
            }}
            QListWidget::item:selected {{
                background: {Colors.PRIMARY_DARK};
            }}
        """
    
    # ========================
    # MENU & MENUBAR
    # ========================
    
    @staticmethod
    def menu_bar() -> str:
        """Menu bar style"""
        return f"""
            QMenuBar {{
                background: {Colors.SECONDARY_DARKEST};
                color: {Colors.TEXT_PRIMARY};
                border-bottom: 1px solid {Colors.BORDER_DEFAULT};
                padding: 4px;
            }}
            QMenuBar::item {{
                background: transparent;
                padding: 8px 16px;
                border-radius: 6px;
            }}
            QMenuBar::item:selected {{
                background: {Colors.PRIMARY_DARK};
            }}
            QMenu {{
                background: {Colors.SECONDARY_DARK};
                color: {Colors.TEXT_PRIMARY};
                border: 1px solid {Colors.BORDER_DEFAULT};
                border-radius: 8px;
                padding: 8px;
            }}
            QMenu::item {{
                padding: 10px 30px 10px 20px;
                border-radius: 4px;
            }}
            QMenu::item:selected {{
                background: {Colors.PRIMARY_DARK};
            }}
            QMenu::separator {{
                height: 1px;
                background: {Colors.BORDER_DEFAULT};
                margin: 6px 10px;
            }}
        """
    
    # ========================
    # STATUS & LABELS
    # ========================
    
    @staticmethod
    def status_bar() -> str:
        """Status bar style"""
        return f"""
            QStatusBar {{
                background: {Colors.SECONDARY_DARKEST};
                color: {Colors.TEXT_SECONDARY};
                border-top: 1px solid {Colors.BORDER_DEFAULT};
            }}
            QStatusBar::item {{
                border: none;
            }}
        """
    
    @staticmethod
    def label_title() -> str:
        """Title label style"""
        return f"""
            QLabel {{
                color: {Colors.TEXT_PRIMARY};
                font-size: 20px;
                font-weight: 700;
                background: transparent;
                border: none;
            }}
        """
    
    @staticmethod
    def label_subtitle() -> str:
        """Subtitle label style"""
        return f"""
            QLabel {{
                color: {Colors.TEXT_SECONDARY};
                font-size: 14px;
                font-weight: 400;
                background: transparent;
                border: none;
            }}
        """
    
    @staticmethod
    def label_accent() -> str:
        """Accent colored label"""
        return f"""
            QLabel {{
                color: {Colors.TEXT_ACCENT};
                font-weight: 600;
                background: transparent;
                border: none;
            }}
        """
    
    @staticmethod
    def label_glow() -> str:
        """Glowing text effect label"""
        return f"""
            QLabel {{
                color: {Colors.ACCENT_LIGHT};
                font-size: 16px;
                font-weight: 600;
                background: transparent;
                border: none;
            }}
        """
    
    @staticmethod
    def badge(color: str = "primary") -> str:
        """Badge/chip style"""
        colors_map = {
            "primary": (Colors.PRIMARY, Colors.TEXT_PRIMARY),
            "accent": (Colors.ACCENT, Colors.SECONDARY_DARKEST),
            "success": (Colors.SUCCESS, Colors.TEXT_PRIMARY),
            "warning": (Colors.WARNING, Colors.SECONDARY_DARKEST),
            "error": (Colors.ERROR, Colors.TEXT_PRIMARY),
            "info": (Colors.INFO, Colors.TEXT_PRIMARY)
        }
        bg, fg = colors_map.get(color, (Colors.PRIMARY, Colors.TEXT_PRIMARY))
        return f"""
            QLabel {{
                background: {bg};
                color: {fg};
                border-radius: 10px;
                padding: 4px 10px;
                font-size: 10px;
                font-weight: 600;
            }}
        """
