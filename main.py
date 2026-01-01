"""
Tobelsoft Macro - Gaming Tool Application
Main Entry Point
"""
import sys
from PyQt6.QtWidgets import QApplication
from PyQt6.QtCore import Qt

from src.theme import Styles
from src.windows.main_window import MainWindow


def main():
    # Enable high DPI scaling
    QApplication.setHighDpiScaleFactorRoundingPolicy(
        Qt.HighDpiScaleFactorRoundingPolicy.PassThrough
    )
    
    app = QApplication(sys.argv)
    app.setApplicationName("Tobelsoft Macro")
    app.setApplicationVersion("1.0.0")
    
    # Apply global stylesheet
    app.setStyleSheet(Styles.app_stylesheet())
    
    # Create and show main window
    window = MainWindow()
    window.show()
    
    sys.exit(app.exec())


if __name__ == "__main__":
    main()
