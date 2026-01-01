"""
Hotkey Controller - Bridge antara UI dan Core Logic
Mengelola state dan business logic untuk Hotkey Page
"""
import uuid
import json
import os
import sys
from typing import Optional, Callable, List
from PyQt6.QtCore import QObject, pyqtSignal

from src.core.hotkey_manager import HotkeyManager, HotkeyBinding, KeyAction, ActionType


class HotkeyController(QObject):
    """
    Controller untuk mengelola hotkey bindings.
    Menyediakan interface bersih antara UI dan core logic.
    """
    
    # Signals untuk update UI
    bindingsChanged = pyqtSignal()  # Emit saat list berubah
    statusChanged = pyqtSignal(bool)  # Emit saat status aktif/nonaktif berubah
    bindingTriggered = pyqtSignal(str)  # Emit saat binding dieksekusi (binding_id)
    error = pyqtSignal(str)  # Emit saat terjadi error
    success = pyqtSignal(str)  # Emit saat operasi berhasil
    
    def __init__(self, parent=None):
        super().__init__(parent)
        
        # Core managers
        self._hotkey_manager = HotkeyManager()
        
        # Persistence Setup
        self._init_data_file()
        self._load_data()
        
        # Start background listeners (for global toggles)
        self._hotkey_manager.start_listeners()
        
        # Connect core callbacks to signals
        self._hotkey_manager.on_status_changed = self._on_status_changed
        self._hotkey_manager.on_binding_triggered = self._on_binding_triggered
    
    # ==================
    # PROPERTIES
    # ==================
    
    @property
    def is_active(self) -> bool:
        """Apakah hotkey system sedang aktif"""
        return self._hotkey_manager.is_active
    
    @property
    def bindings(self) -> List[HotkeyBinding]:
        """Get list of active bindings"""
        return self._hotkey_manager.bindings
    
    @property
    def binding_count(self) -> int:
        """Jumlah binding aktif"""
        return len(self.bindings)
        
    # ==================
    # CORE OPERATIONS
    # ==================
    
    def toggle_active(self):
        """Toggle global status start/stop"""
        if self.is_active:
            self.stop()
        else:
            self.start()
    
    def start(self):
        """Start hotkey system"""
        try:
            self._hotkey_manager.start()
        except Exception as e:
            self.error.emit(f"Failed to start hotkey system: {str(e)}")
            
    def stop(self):
        """Stop hotkey system"""
        try:
            self._hotkey_manager.stop()
        except Exception as e:
            self.error.emit(f"Failed to stop hotkey system: {str(e)}")

    def add_binding(self, name: str, trigger_keys: List[str], 
                   actions: List[KeyAction], repeat: bool = False, 
                   repeat_delay: int = 100, block_input: bool = False) -> Optional[str]:
        """Tambah binding baru"""
        try:
            binding_id = str(uuid.uuid4())
            binding = HotkeyBinding(
                id=binding_id,
                name=name,
                trigger_keys=trigger_keys if isinstance(trigger_keys, list) else [trigger_keys],
                actions=actions,
                enabled=True,
                repeat=repeat,
                repeat_delay=repeat_delay,
                block_input=block_input
            )
            
            if self._hotkey_manager.add_binding(binding):
                self.bindingsChanged.emit()
                self._save_data()
                return binding.id
            else:
                self.error.emit("Failed to add binding.")
                return None
        except Exception as e:
            self.error.emit(f"Failed to add binding: {str(e)}")
            return None
    
    def update_binding(self, binding_id: str, name: str, trigger_keys: List[str], 
                       actions: List[KeyAction], repeat: bool = False, 
                       repeat_delay: int = 100, block_input: bool = False) -> bool:
        """Update binding yang ada"""
        try:
            binding = HotkeyBinding(
                id=binding_id,
                name=name,
                trigger_keys=trigger_keys if isinstance(trigger_keys, list) else [trigger_keys],
                actions=actions,
                enabled=True,
                repeat=repeat,
                repeat_delay=repeat_delay,
                block_input=block_input
            )
            
            self._hotkey_manager.update_binding(binding)
            self.bindingsChanged.emit()
            self._save_data()
            return True
        except Exception as e:
            self.error.emit(f"Failed to update binding: {str(e)}")
            return False
    
    def remove_binding(self, binding_id: str) -> bool:
        """Hapus binding"""
        try:
            self._hotkey_manager.remove_binding(binding_id)
            self.bindingsChanged.emit()
            self._save_data()
            return True
        except Exception as e:
            self.error.emit(f"Failed to remove binding: {str(e)}")
            return False
    
    def get_binding(self, binding_id: str) -> Optional[HotkeyBinding]:
        """Get binding by ID"""
        return self._hotkey_manager.get_binding(binding_id)
    
    def toggle_binding(self, binding_id: str, enabled: bool):
        """Toggle enabled/disabled pada binding tertentu"""
        self._hotkey_manager.toggle_binding(binding_id, enabled)
        self._save_data()
    
    def set_master_triggers(self, keys: List[str]):
        """Set keys untuk global toggle"""
        self._hotkey_manager.set_master_triggers(keys)
        self._save_data()
        
    @property
    def master_triggers(self) -> List[str]:
        return self._hotkey_manager.master_trigger_keys
    
    # ==================
    # IMPORT / EXPORT OPERATIONS (Per Item)
    # ==================
    
    def export_hotkey(self, binding_id: str, filepath: str) -> bool:
        """Export single hotkey to file"""
        binding = self.get_binding(binding_id)
        if not binding:
            self.error.emit(f"Binding {binding_id} not found.")
            return False
            
        try:
            with open(filepath, 'w') as f:
                json.dump(binding.to_dict(), f, indent=4)
            
            self.success.emit(f"Hotkey '{binding.name}' exported!")
            return True
        except Exception as e:
            self.error.emit(f"Failed to export hotkey: {str(e)}")
            return False
            
    def import_hotkey(self, filepath: str) -> bool:
        """Import single hotkey from file and add to list"""
        try:
            with open(filepath, 'r') as f:
                data = json.load(f)
            
            # Basic Validation check
            # Just check if critical keys exist
            if "actions" in data or "trigger_keys" in data or "trigger_key" in data:
                # Import as new binding
                binding = HotkeyBinding.from_dict(data)
                
                # Prevent duplicate ID
                if self.get_binding(binding.id):
                    binding.id = str(uuid.uuid4())
                    binding.name = f"{binding.name} (Imported)"
                
                self._hotkey_manager.add_binding(binding)
                self.bindingsChanged.emit()
                self._save_data()
                self.success.emit(f"Hotkey '{binding.name}' imported!")
                return True
            else:
                 raise ValueError("Invalid format: Not a hotkey file")
            
        except Exception as e:
            self.error.emit(f"Failed to import hotkey: {str(e)}")
            return False
    
    # ==================
    # PERSISTENCE
    # ==================
    
    def _init_data_file(self):
        """Initialize data file path in appdata folder"""
        if getattr(sys, 'frozen', False):
            # If EXE, use folder of executable
            base_path = os.path.dirname(sys.executable)
        else:
            # If DEV, use project root (assuming main.py is in root)
            # Current file: src/controllers/hotkey_controller.py
            current_dir = os.path.dirname(os.path.abspath(__file__))
            base_path = os.path.abspath(os.path.join(current_dir, "..", ".."))
            
        self.app_data_dir = os.path.join(base_path, "appdata")
        os.makedirs(self.app_data_dir, exist_ok=True)
        self.data_file = os.path.join(self.app_data_dir, "tobelsoft_macro_data.json")
        print(f"[DEBUG] Data file path: {self.data_file}")

    def _load_data(self):
        """Load data from JSON file"""
        if os.path.exists(self.data_file):
            try:
                print(f"[DEBUG] Loading data from {self.data_file}")
                with open(self.data_file, 'r') as f:
                    data = json.load(f)
                    self._hotkey_manager.from_dict(data)
                    self._hotkey_manager.set_master_triggers(data.get("master_trigger_keys", []))
                self.bindingsChanged.emit()
            except Exception as e:
                print(f"[ERROR] Loading data failed: {e}")
                self.error.emit(f"Failed to load data: {str(e)}")

    def _save_data(self):
        """Save data to JSON file"""
        try:
            print(f"[DEBUG] Saving data to {self.data_file}")
            data = self._hotkey_manager.to_dict()
            data["master_trigger_keys"] = self._hotkey_manager.master_trigger_keys
            with open(self.data_file, 'w') as f:
                json.dump(data, f, indent=4)
        except Exception as e:
            print(f"[ERROR] Saving data failed: {e}")
            self.error.emit(f"Failed to save data: {str(e)}")

    # ==================
    # INTERNAL CALLBACKS
    # ==================
    
    def _on_status_changed(self, active: bool):
        """Callback dari HotkeyManager"""
        self.statusChanged.emit(active)
    
    def _on_binding_triggered(self, binding: HotkeyBinding):
        """Callback saat binding dieksekusi"""
        self.bindingTriggered.emit(binding.id)
    
    # ==================
    # HELPER METHODS
    # ==================
    
    @staticmethod
    def create_key_press_action(key: str) -> KeyAction:
        """Helper untuk buat KeyAction type KEY_PRESS"""
        return KeyAction(action_type=ActionType.KEY_PRESS, keys=[key])
    
    @staticmethod
    def create_key_hold_action(key: str, duration_ms: int) -> KeyAction:
        """Helper untuk buat KeyAction type KEY_HOLD"""
        return KeyAction(action_type=ActionType.KEY_HOLD, keys=[key], duration=duration_ms)
    
    @staticmethod
    def create_delay_action(duration_ms: int) -> KeyAction:
        """Helper untuk buat KeyAction type DELAY"""
        return KeyAction(action_type=ActionType.DELAY, duration=duration_ms)
    
    @staticmethod
    def create_sequence_action(keys: List[str]) -> KeyAction:
        """Helper untuk buat KeyAction type KEY_SEQUENCE"""
        return KeyAction(action_type=ActionType.KEY_SEQUENCE, keys=keys)
