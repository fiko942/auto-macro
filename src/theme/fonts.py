"""
Font System untuk Gaming Tool
"""
from PyQt6.QtGui import QFont, QFontDatabase
from PyQt6.QtWidgets import QApplication


class Fonts:
    """Typography system untuk gaming tool UI"""
    
    # Font Families
    FAMILY_PRIMARY = "Segoe UI"      # Modern Windows font
    FAMILY_MONO = "Consolas"         # Monospace untuk code/logs
    FAMILY_GAMING = "Orbitron"       # Gaming style (jika tersedia)
    FAMILY_FALLBACK = "Arial"        # Fallback font
    
    # Font Sizes
    SIZE_TINY = 9
    SIZE_SMALL = 10
    SIZE_NORMAL = 11
    SIZE_MEDIUM = 12
    SIZE_LARGE = 14
    SIZE_XLARGE = 16
    SIZE_TITLE = 20
    SIZE_HEADING = 24
    SIZE_HERO = 32
    SIZE_DISPLAY = 48
    
    # Font Weights
    WEIGHT_THIN = QFont.Weight.Thin
    WEIGHT_LIGHT = QFont.Weight.Light
    WEIGHT_NORMAL = QFont.Weight.Normal
    WEIGHT_MEDIUM = QFont.Weight.Medium
    WEIGHT_SEMIBOLD = QFont.Weight.DemiBold
    WEIGHT_BOLD = QFont.Weight.Bold
    WEIGHT_EXTRABOLD = QFont.Weight.ExtraBold
    
    @classmethod
    def get_font(cls, size: int = SIZE_NORMAL, weight: QFont.Weight = None, 
                 family: str = None, italic: bool = False) -> QFont:
        """Create a QFont with specified parameters"""
        font = QFont()
        font.setFamily(family or cls.FAMILY_PRIMARY)
        font.setPointSize(size)
        if weight:
            font.setWeight(weight)
        font.setItalic(italic)
        return font
    
    @classmethod
    def heading(cls, level: int = 1) -> QFont:
        """Get heading font berdasarkan level (1-6)"""
        sizes = {
            1: cls.SIZE_HERO,
            2: cls.SIZE_HEADING,
            3: cls.SIZE_TITLE,
            4: cls.SIZE_XLARGE,
            5: cls.SIZE_LARGE,
            6: cls.SIZE_MEDIUM
        }
        size = sizes.get(level, cls.SIZE_LARGE)
        return cls.get_font(size=size, weight=cls.WEIGHT_BOLD)
    
    @classmethod
    def body(cls, size: str = "normal") -> QFont:
        """Get body text font"""
        sizes = {
            "tiny": cls.SIZE_TINY,
            "small": cls.SIZE_SMALL,
            "normal": cls.SIZE_NORMAL,
            "medium": cls.SIZE_MEDIUM,
            "large": cls.SIZE_LARGE
        }
        return cls.get_font(size=sizes.get(size, cls.SIZE_NORMAL))
    
    @classmethod
    def mono(cls, size: int = SIZE_NORMAL) -> QFont:
        """Get monospace font untuk code/logs"""
        return cls.get_font(size=size, family=cls.FAMILY_MONO)
    
    @classmethod
    def button(cls, size: str = "normal") -> QFont:
        """Get button font"""
        sizes = {
            "small": cls.SIZE_SMALL,
            "normal": cls.SIZE_NORMAL,
            "large": cls.SIZE_MEDIUM
        }
        return cls.get_font(size=sizes.get(size, cls.SIZE_NORMAL), 
                           weight=cls.WEIGHT_SEMIBOLD)
    
    @classmethod
    def label(cls) -> QFont:
        """Get label font"""
        return cls.get_font(size=cls.SIZE_SMALL, weight=cls.WEIGHT_MEDIUM)
    
    @classmethod
    def title(cls) -> QFont:
        """Get title font"""
        return cls.get_font(size=cls.SIZE_TITLE, weight=cls.WEIGHT_BOLD)
    
    @classmethod
    def caption(cls) -> QFont:
        """Get caption/small text font"""
        return cls.get_font(size=cls.SIZE_TINY, weight=cls.WEIGHT_NORMAL)
    
    @classmethod
    def gaming_title(cls) -> QFont:
        """Get gaming style title font"""
        font = cls.get_font(size=cls.SIZE_HEADING, weight=cls.WEIGHT_BOLD)
        # Try gaming font, fallback ke primary
        font.setFamily(cls.FAMILY_GAMING)
        if not QFontDatabase.hasFamily(cls.FAMILY_GAMING):
            font.setFamily(cls.FAMILY_PRIMARY)
        font.setLetterSpacing(QFont.SpacingType.AbsoluteSpacing, 2)
        return font
