"""
Preset Manager - Save/Load/Import/Export presets
"""
import json
import os
from pathlib import Path
from typing import List, Optional
from dataclasses import dataclass
from datetime import datetime


@dataclass
class Preset:
    """Preset configuration"""
    name: str
    description: str
    created_at: str
    data: dict  # Hotkey bindings data
    
    def to_dict(self) -> dict:
        return {
            "name": self.name,
            "description": self.description,
            "created_at": self.created_at,
            "data": self.data
        }
    
    @classmethod
    def from_dict(cls, data: dict) -> 'Preset':
        return cls(
            name=data.get("name", "Unnamed"),
            description=data.get("description", ""),
            created_at=data.get("created_at", ""),
            data=data.get("data", {})
        )


class PresetManager:
    """Manager untuk preset storage"""
    
    def __init__(self, presets_dir: str = None):
        if presets_dir:
            self.presets_dir = Path(presets_dir)
        else:
            # Default: di folder user/.tobelsoft_macro/presets
            self.presets_dir = Path.home() / ".tobelsoft_macro" / "presets"
        
        self.presets_dir.mkdir(parents=True, exist_ok=True)
    
    def get_presets(self) -> List[Preset]:
        """Get semua preset yang tersimpan"""
        presets = []
        
        for file in self.presets_dir.glob("*.json"):
            try:
                with open(file, 'r', encoding='utf-8') as f:
                    data = json.load(f)
                    presets.append(Preset.from_dict(data))
            except Exception as e:
                print(f"Error loading preset {file}: {e}")
        
        # Sort by created_at descending
        presets.sort(key=lambda p: p.created_at, reverse=True)
        return presets
    
    def save_preset(self, name: str, description: str, hotkey_data: dict) -> Preset:
        """Save preset baru"""
        preset = Preset(
            name=name,
            description=description,
            created_at=datetime.now().isoformat(),
            data=hotkey_data
        )
        
        # Generate safe filename
        safe_name = "".join(c for c in name if c.isalnum() or c in (' ', '-', '_')).strip()
        safe_name = safe_name.replace(' ', '_')
        filename = f"{safe_name}_{int(datetime.now().timestamp())}.json"
        
        filepath = self.presets_dir / filename
        
        with open(filepath, 'w', encoding='utf-8') as f:
            json.dump(preset.to_dict(), f, indent=2, ensure_ascii=False)
        
        return preset
    
    def delete_preset(self, preset_name: str) -> bool:
        """Hapus preset"""
        for file in self.presets_dir.glob("*.json"):
            try:
                with open(file, 'r', encoding='utf-8') as f:
                    data = json.load(f)
                    if data.get("name") == preset_name:
                        file.unlink()
                        return True
            except:
                pass
        return False
    
    def export_preset(self, preset: Preset, filepath: str) -> bool:
        """Export preset ke file external"""
        try:
            with open(filepath, 'w', encoding='utf-8') as f:
                json.dump(preset.to_dict(), f, indent=2, ensure_ascii=False)
            return True
        except Exception as e:
            print(f"Error exporting preset: {e}")
            return False
    
    def import_preset(self, filepath: str) -> Optional[Preset]:
        """Import preset dari file external"""
        try:
            with open(filepath, 'r', encoding='utf-8') as f:
                data = json.load(f)
            
            preset = Preset.from_dict(data)
            
            # Save to local presets
            self.save_preset(preset.name, preset.description, preset.data)
            
            return preset
        except Exception as e:
            print(f"Error importing preset: {e}")
            return None
    
    def export_to_string(self, hotkey_data: dict) -> str:
        """Export ke JSON string (untuk clipboard)"""
        return json.dumps(hotkey_data, indent=2)
    
    def import_from_string(self, json_string: str) -> Optional[dict]:
        """Import dari JSON string"""
        try:
            return json.loads(json_string)
        except:
            return None
