#include "ui_icons.h"
#include "theme.h"
#include <math.h>
#include <stdio.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ============================================================================
// HELPER: ACTION TYPE TO VECTOR ICON MAPPING
// ============================================================================
IconId GetActionTypeIcon(ActionType type) {
    switch (type) {
        case ACTION_KEY_PRESS:    return ICON_BOLT;
        case ACTION_KEY_HOLD:     return ICON_LOCK;
        case ACTION_DELAY:        return ICON_CLOCK;
        case ACTION_KEY_DOWN:     return ICON_ARROW_DOWN;
        case ACTION_KEY_UP:       return ICON_ARROW_UP;
        case ACTION_KEY_SEQUENCE: return ICON_SEQUENCE;
        default:                  return ICON_BOLT;
    }
}

// ============================================================================
// CORE VECTOR ICON DRAWING ENGINE (PURE WIN32 GDI GEOMETRY)
// ============================================================================
void DrawVectorIcon(HDC hdc, IconId icon, int x, int y, int size, COLORREF color) {
    if (icon == ICON_NONE || size <= 2) return;

    int stroke = (size >= 32) ? 3 : ((size >= 15) ? 2 : 1);
    
    HPEN pen = CreatePen(PS_SOLID, stroke, color);
    HBRUSH fill_brush = CreateSolidBrush(color);
    HGDIOBJ old_pen = SelectObject(hdc, pen);
    HGDIOBJ old_brush = SelectObject(hdc, GetStockObject(NULL_BRUSH));

    switch (icon) {
        case ICON_BOLT: {
            // Cyberpunk sharp lightning bolt (solid filled)
            SelectObject(hdc, fill_brush);
            POINT pts[7] = {
                { x + (int)(size * 0.58f), y + (int)(size * 0.05f) },
                { x + (int)(size * 0.20f), y + (int)(size * 0.52f) },
                { x + (int)(size * 0.48f), y + (int)(size * 0.52f) },
                { x + (int)(size * 0.38f), y + (int)(size * 0.95f) },
                { x + (int)(size * 0.80f), y + (int)(size * 0.42f) },
                { x + (int)(size * 0.52f), y + (int)(size * 0.42f) },
                { x + (int)(size * 0.62f), y + (int)(size * 0.05f) }
            };
            Polygon(hdc, pts, 7);
            break;
        }

        case ICON_SETTINGS: {
            // Precision 8-tooth mechanical gear / cog
            int cx = x + size / 2;
            int cy = y + size / 2;
            int r_outer = (int)(size * 0.42f);
            int r_inner = (int)(size * 0.28f);
            int r_hole  = (int)(size * 0.14f);
            if (r_hole < 2) r_hole = 2;

            // Draw outer circle
            Ellipse(hdc, cx - r_inner, cy - r_inner, cx + r_inner + 1, cy + r_inner + 1);

            // Draw 4 through-bars (8 teeth)
            for (int a = 0; a < 4; a++) {
                double angle = a * (M_PI / 4.0);
                int dx = (int)(cos(angle) * r_outer);
                int dy = (int)(sin(angle) * r_outer);
                MoveToEx(hdc, cx - dx, cy - dy, NULL);
                LineTo(hdc, cx + dx, cy + dy);
            }

            // Central axle hole
            Ellipse(hdc, cx - r_hole, cy - r_hole, cx + r_hole + 1, cy + r_hole + 1);
            break;
        }

        case ICON_INFO: {
            // Info circle with centered "i"
            int pad = (int)(size * 0.08f);
            Ellipse(hdc, x + pad, y + pad, x + size - pad, y + size - pad);
            int cx = x + size / 2;
            
            // Top dot
            SelectObject(hdc, fill_brush);
            int dot_y = y + (int)(size * 0.28f);
            int dot_r = (size >= 20) ? 2 : 1;
            Ellipse(hdc, cx - dot_r, dot_y - dot_r, cx + dot_r + 1, dot_y + dot_r + 1);
            SelectObject(hdc, GetStockObject(NULL_BRUSH));

            // Stem and bottom serif
            int stem_top = y + (int)(size * 0.42f);
            int stem_bot = y + (int)(size * 0.74f);
            MoveToEx(hdc, cx, stem_top, NULL);
            LineTo(hdc, cx, stem_bot);
            MoveToEx(hdc, cx - (int)(size * 0.12f), stem_bot, NULL);
            LineTo(hdc, cx + (int)(size * 0.12f), stem_bot);
            break;
        }

        case ICON_PLUS: {
            // Clean centered plus sign
            int pad = (int)(size * 0.20f);
            int cx = x + size / 2;
            int cy = y + size / 2;
            MoveToEx(hdc, cx, y + pad, NULL);
            LineTo(hdc, cx, y + size - pad + 1);
            MoveToEx(hdc, x + pad, cy, NULL);
            LineTo(hdc, x + size - pad + 1, cy);
            break;
        }

        case ICON_CLOSE: {
            // Clean diagonal cross / X
            int pad = (int)(size * 0.22f);
            MoveToEx(hdc, x + pad, y + pad, NULL);
            LineTo(hdc, x + size - pad, y + size - pad);
            MoveToEx(hdc, x + size - pad, y + pad, NULL);
            LineTo(hdc, x + pad, y + size - pad);
            break;
        }

        case ICON_EDIT: {
            // Diagonal pencil with sharp tip
            POINT body[4] = {
                { x + (int)(size * 0.62f), y + (int)(size * 0.14f) },
                { x + (int)(size * 0.86f), y + (int)(size * 0.38f) },
                { x + (int)(size * 0.38f), y + (int)(size * 0.86f) },
                { x + (int)(size * 0.14f), y + (int)(size * 0.62f) }
            };
            Polygon(hdc, body, 4);

            // Pencil tip
            POINT tip[3] = {
                { x + (int)(size * 0.38f), y + (int)(size * 0.86f) },
                { x + (int)(size * 0.14f), y + (int)(size * 0.62f) },
                { x + (int)(size * 0.10f), y + (int)(size * 0.90f) }
            };
            SelectObject(hdc, fill_brush);
            Polygon(hdc, tip, 3);
            SelectObject(hdc, GetStockObject(NULL_BRUSH));
            break;
        }

        case ICON_TRASH: {
            // Wastebasket container with lid & handle
            int x1 = x + (int)(size * 0.22f);
            int x2 = x + (int)(size * 0.78f);
            int y_lid = y + (int)(size * 0.28f);

            // Lid handle
            MoveToEx(hdc, x + (int)(size * 0.38f), y_lid, NULL);
            LineTo(hdc, x + (int)(size * 0.38f), y + (int)(size * 0.16f));
            LineTo(hdc, x + (int)(size * 0.62f), y + (int)(size * 0.16f));
            LineTo(hdc, x + (int)(size * 0.62f), y_lid);

            // Lid bar
            MoveToEx(hdc, x + (int)(size * 0.16f), y_lid, NULL);
            LineTo(hdc, x + (int)(size * 0.84f), y_lid);

            // Bin Body
            POINT body[4] = {
                { x1 + (int)(size * 0.04f), y_lid + 2 },
                { x1 + (int)(size * 0.08f), y + (int)(size * 0.88f) },
                { x2 - (int)(size * 0.08f), y + (int)(size * 0.88f) },
                { x2 - (int)(size * 0.04f), y_lid + 2 }
            };
            Polyline(hdc, body, 4);

            // Center ribs
            int cx = x + size / 2;
            MoveToEx(hdc, cx - (int)(size * 0.10f), y_lid + 4, NULL);
            LineTo(hdc, cx - (int)(size * 0.10f), y + (int)(size * 0.80f));
            MoveToEx(hdc, cx + (int)(size * 0.10f), y_lid + 4, NULL);
            LineTo(hdc, cx + (int)(size * 0.10f), y + (int)(size * 0.80f));
            break;
        }

        case ICON_IMPORT: {
            // Bottom tray + downward arrow
            int bx1 = x + (int)(size * 0.16f);
            int bx2 = x + (int)(size * 0.84f);
            int by1 = y + (int)(size * 0.58f);
            int by2 = y + (int)(size * 0.86f);

            POINT tray[4] = {
                { bx1, by1 },
                { bx1, by2 },
                { bx2, by2 },
                { bx2, by1 }
            };
            Polyline(hdc, tray, 4);

            // Arrow down
            int cx = x + size / 2;
            MoveToEx(hdc, cx, y + (int)(size * 0.14f), NULL);
            LineTo(hdc, cx, y + (int)(size * 0.62f));
            
            POINT head[3] = {
                { cx - (int)(size * 0.20f), y + (int)(size * 0.44f) },
                { cx, y + (int)(size * 0.64f) },
                { cx + (int)(size * 0.20f), y + (int)(size * 0.44f) }
            };
            Polyline(hdc, head, 3);
            break;
        }

        case ICON_EXPORT: {
            // Bottom tray + upward arrow
            int bx1 = x + (int)(size * 0.16f);
            int bx2 = x + (int)(size * 0.84f);
            int by1 = y + (int)(size * 0.58f);
            int by2 = y + (int)(size * 0.86f);

            POINT tray[4] = {
                { bx1, by1 },
                { bx1, by2 },
                { bx2, by2 },
                { bx2, by1 }
            };
            Polyline(hdc, tray, 4);

            // Arrow up
            int cx = x + size / 2;
            MoveToEx(hdc, cx, y + (int)(size * 0.64f), NULL);
            LineTo(hdc, cx, y + (int)(size * 0.16f));
            
            POINT head[3] = {
                { cx - (int)(size * 0.20f), y + (int)(size * 0.36f) },
                { cx, y + (int)(size * 0.16f) },
                { cx + (int)(size * 0.20f), y + (int)(size * 0.36f) }
            };
            Polyline(hdc, head, 3);
            break;
        }

        case ICON_SAVE: {
            // Floppy disk / storage chip
            int x1 = x + (int)(size * 0.14f);
            int y1 = y + (int)(size * 0.14f);
            int x2 = x + (int)(size * 0.86f);
            int y2 = y + (int)(size * 0.86f);
            int chamfer = (int)(size * 0.14f);

            POINT disk[6] = {
                { x1, y1 },
                { x2 - chamfer, y1 },
                { x2, y1 + chamfer },
                { x2, y2 },
                { x1, y2 },
                { x1, y1 }
            };
            Polyline(hdc, disk, 6);

            // Top shutter
            Rectangle(hdc, x1 + (int)(size * 0.14f), y1, x2 - (int)(size * 0.14f), y1 + (int)(size * 0.28f));
            // Bottom label
            Rectangle(hdc, x1 + (int)(size * 0.10f), y2 - (int)(size * 0.32f), x2 - (int)(size * 0.10f), y2);
            break;
        }

        case ICON_FOLDER: {
            // Directory folder with tab
            int x1 = x + (int)(size * 0.12f);
            int y1 = y + (int)(size * 0.22f);
            int x2 = x + (int)(size * 0.88f);
            int y2 = y + (int)(size * 0.84f);
            int tab_w = (int)(size * 0.34f);
            int tab_h = (int)(size * 0.14f);

            POINT folder[7] = {
                { x1, y1 },
                { x1 + tab_w, y1 },
                { x1 + tab_w + (int)(size * 0.08f), y1 + tab_h },
                { x2, y1 + tab_h },
                { x2, y2 },
                { x1, y2 },
                { x1, y1 }
            };
            Polygon(hdc, folder, 7);
            break;
        }

        case ICON_COPY: {
            // Dual overlapping clipboard sheets
            int off = (int)(size * 0.16f);
            // Back sheet
            POINT back[5] = {
                { x + off + (int)(size * 0.14f), y + (int)(size * 0.12f) },
                { x + size - (int)(size * 0.12f), y + (int)(size * 0.12f) },
                { x + size - (int)(size * 0.12f), y + size - off - (int)(size * 0.12f) },
                { x + off + (int)(size * 0.14f), y + size - off - (int)(size * 0.12f) },
                { x + off + (int)(size * 0.14f), y + (int)(size * 0.12f) }
            };
            Polyline(hdc, back, 5);

            // Front sheet (solid backdrop)
            SelectObject(hdc, fill_brush);
            int fx1 = x + (int)(size * 0.12f);
            int fy1 = y + off + (int)(size * 0.12f);
            int fx2 = x + size - off - (int)(size * 0.14f);
            int fy2 = y + size - (int)(size * 0.12f);
            
            // Draw background cutout
            HBRUSH bg_b = CreateSolidBrush(COLOR_BG_CARD);
            HGDIOBJ b_old = SelectObject(hdc, bg_b);
            Rectangle(hdc, fx1, fy1, fx2, fy2);
            SelectObject(hdc, b_old);
            DeleteObject(bg_b);

            // Front sheet border
            Rectangle(hdc, fx1, fy1, fx2, fy2);
            break;
        }

        case ICON_SHIELD: {
            // Security shield outline + vertical dividing crest
            int x1 = x + (int)(size * 0.18f);
            int x2 = x + (int)(size * 0.82f);
            int y1 = y + (int)(size * 0.14f);
            int ym = y + (int)(size * 0.52f);
            int y2 = y + (int)(size * 0.90f);
            int cx = x + size / 2;

            POINT shield[6] = {
                { x1, y1 },
                { x2, y1 },
                { x2, ym },
                { cx, y2 },
                { x1, ym },
                { x1, y1 }
            };
            Polyline(hdc, shield, 6);

            // Vertical crest
            MoveToEx(hdc, cx, y1 + 2, NULL);
            LineTo(hdc, cx, y2 - 2);
            break;
        }

        case ICON_TARGET: {
            // Precision crosshair reticle
            int cx = x + size / 2;
            int cy = y + size / 2;
            int r_outer = (int)(size * 0.38f);
            int r_inner = (int)(size * 0.16f);

            // Outer circle
            Ellipse(hdc, cx - r_outer, cy - r_outer, cx + r_outer + 1, cy + r_outer + 1);
            // Inner circle
            Ellipse(hdc, cx - r_inner, cy - r_inner, cx + r_inner + 1, cy + r_inner + 1);

            // 4 Crosshair ticks
            MoveToEx(hdc, cx, y + (int)(size * 0.04f), NULL);
            LineTo(hdc, cx, cy - r_inner);

            MoveToEx(hdc, cx, cy + r_inner, NULL);
            LineTo(hdc, cx, y + size - (int)(size * 0.04f));

            MoveToEx(hdc, x + (int)(size * 0.04f), cy, NULL);
            LineTo(hdc, cx - r_inner, cy);

            MoveToEx(hdc, cx + r_inner, cy, NULL);
            LineTo(hdc, x + size - (int)(size * 0.04f), cy);
            break;
        }

        case ICON_CLOCK: {
            // Clock face with hour and minute hands
            int pad = (int)(size * 0.10f);
            int cx = x + size / 2;
            int cy = y + size / 2;
            Ellipse(hdc, x + pad, y + pad, x + size - pad, y + size - pad);

            // Hour hand (pointing to 12)
            MoveToEx(hdc, cx, cy, NULL);
            LineTo(hdc, cx, cy - (int)(size * 0.26f));

            // Minute hand (pointing to 3)
            MoveToEx(hdc, cx, cy, NULL);
            LineTo(hdc, cx + (int)(size * 0.22f), cy);
            break;
        }

        case ICON_LOCK: {
            // Padlock: Top shackle + rounded body + keyhole
            int bx1 = x + (int)(size * 0.22f);
            int bx2 = x + (int)(size * 0.78f);
            int by1 = y + (int)(size * 0.44f);
            int by2 = y + (int)(size * 0.88f);

            // Shackle
            int sx1 = x + (int)(size * 0.32f);
            int sx2 = x + (int)(size * 0.68f);
            int sy1 = y + (int)(size * 0.16f);

            POINT shackle[4] = {
                { sx1, by1 },
                { sx1, sy1 },
                { sx2, sy1 },
                { sx2, by1 }
            };
            Polyline(hdc, shackle, 4);

            // Body
            RoundRect(hdc, bx1, by1, bx2, by2, 4, 4);

            // Keyhole
            int cx = x + size / 2;
            MoveToEx(hdc, cx, by1 + (int)(size * 0.12f), NULL);
            LineTo(hdc, cx, by1 + (int)(size * 0.26f));
            break;
        }

        case ICON_ARROW_DOWN: {
            // Downward arrow
            int cx = x + size / 2;
            MoveToEx(hdc, cx, y + (int)(size * 0.16f), NULL);
            LineTo(hdc, cx, y + (int)(size * 0.82f));

            POINT head[3] = {
                { cx - (int)(size * 0.26f), y + (int)(size * 0.54f) },
                { cx, y + (int)(size * 0.82f) },
                { cx + (int)(size * 0.26f), y + (int)(size * 0.54f) }
            };
            Polyline(hdc, head, 3);
            break;
        }

        case ICON_ARROW_UP: {
            // Upward arrow
            int cx = x + size / 2;
            MoveToEx(hdc, cx, y + (int)(size * 0.82f), NULL);
            LineTo(hdc, cx, y + (int)(size * 0.16f));

            POINT head[3] = {
                { cx - (int)(size * 0.26f), y + (int)(size * 0.44f) },
                { cx, y + (int)(size * 0.16f) },
                { cx + (int)(size * 0.26f), y + (int)(size * 0.44f) }
            };
            Polyline(hdc, head, 3);
            break;
        }

        case ICON_SEQUENCE: {
            // 3 Stepping block nodes connected by a flow line
            int r = (size >= 24) ? 3 : 2;
            int p1_x = x + (int)(size * 0.22f), p1_y = y + (int)(size * 0.28f);
            int p2_x = x + (int)(size * 0.50f), p2_y = y + (int)(size * 0.50f);
            int p3_x = x + (int)(size * 0.78f), p3_y = y + (int)(size * 0.72f);

            // Flow line
            MoveToEx(hdc, p1_x, p1_y, NULL);
            LineTo(hdc, p2_x, p2_y);
            LineTo(hdc, p3_x, p3_y);

            // Node dots
            SelectObject(hdc, fill_brush);
            Ellipse(hdc, p1_x - r, p1_y - r, p1_x + r + 1, p1_y + r + 1);
            Ellipse(hdc, p2_x - r, p2_y - r, p2_x + r + 1, p2_y + r + 1);
            Ellipse(hdc, p3_x - r, p3_y - r, p3_x + r + 1, p3_y + r + 1);
            break;
        }

        case ICON_BELL: {
            // Tactical chime bell
            int bx1 = x + (int)(size * 0.20f);
            int bx2 = x + (int)(size * 0.80f);
            int by_rim = y + (int)(size * 0.72f);
            int cx = x + size / 2;

            // Bell dome
            POINT bell[5] = {
                { bx1, by_rim },
                { cx - (int)(size * 0.16f), y + (int)(size * 0.26f) },
                { cx + (int)(size * 0.16f), y + (int)(size * 0.26f) },
                { bx2, by_rim },
                { bx1, by_rim }
            };
            Polyline(hdc, bell, 5);

            // Top loop
            MoveToEx(hdc, cx, y + (int)(size * 0.26f), NULL);
            LineTo(hdc, cx, y + (int)(size * 0.16f));

            // Clapper
            MoveToEx(hdc, cx - (int)(size * 0.08f), by_rim + 1, NULL);
            LineTo(hdc, cx + (int)(size * 0.08f), by_rim + 1);
            break;
        }

        case ICON_STATS: {
            // 3-Bar histogram with baseline
            int y_base = y + (int)(size * 0.84f);
            MoveToEx(hdc, x + (int)(size * 0.12f), y_base, NULL);
            LineTo(hdc, x + (int)(size * 0.88f), y_base);

            // Bar 1
            SelectObject(hdc, fill_brush);
            Rectangle(hdc, x + (int)(size * 0.20f), y + (int)(size * 0.54f), x + (int)(size * 0.34f), y_base);
            // Bar 2
            Rectangle(hdc, x + (int)(size * 0.42f), y + (int)(size * 0.32f), x + (int)(size * 0.56f), y_base);
            // Bar 3
            Rectangle(hdc, x + (int)(size * 0.64f), y + (int)(size * 0.16f), x + (int)(size * 0.78f), y_base);
            break;
        }

        case ICON_TOOLS: {
            // Diagonal wrench
            int x1 = x + (int)(size * 0.22f);
            int y1 = y + (int)(size * 0.78f);
            int x2 = x + (int)(size * 0.72f);
            int y2 = y + (int)(size * 0.28f);

            // Handle
            MoveToEx(hdc, x1, y1, NULL);
            LineTo(hdc, x2, y2);

            // Wrench jaw at top-right
            MoveToEx(hdc, x2 - (int)(size * 0.12f), y2 + (int)(size * 0.06f), NULL);
            LineTo(hdc, x2 + (int)(size * 0.12f), y2 - (int)(size * 0.14f));
            LineTo(hdc, x2 + (int)(size * 0.18f), y2 - (int)(size * 0.04f));

            // Ring at bottom-left
            int r = (size >= 20) ? 3 : 2;
            Ellipse(hdc, x1 - r, y1 - r, x1 + r + 1, y1 + r + 1);
            break;
        }

        case ICON_CHECK: {
            // Crisp verification checkmark
            POINT check[3] = {
                { x + (int)(size * 0.16f), y + (int)(size * 0.52f) },
                { x + (int)(size * 0.42f), y + (int)(size * 0.78f) },
                { x + (int)(size * 0.84f), y + (int)(size * 0.22f) }
            };
            Polyline(hdc, check, 3);
            break;
        }

        case ICON_PLAY: {
            // Right-pointing play triangle
            SelectObject(hdc, fill_brush);
            POINT tri[3] = {
                { x + (int)(size * 0.26f), y + (int)(size * 0.18f) },
                { x + (int)(size * 0.82f), y + (int)(size * 0.50f) },
                { x + (int)(size * 0.26f), y + (int)(size * 0.82f) }
            };
            Polygon(hdc, tri, 3);
            break;
        }

        case ICON_STOP: {
            // Solid stop square
            SelectObject(hdc, fill_brush);
            int pad = (int)(size * 0.24f);
            Rectangle(hdc, x + pad, y + pad, x + size - pad, y + size - pad);
            break;
        }

        case ICON_KEYBOARD: {
            // Physical keyboard frame + keys
            int x1 = x + (int)(size * 0.12f);
            int y1 = y + (int)(size * 0.24f);
            int x2 = x + (int)(size * 0.88f);
            int y2 = y + (int)(size * 0.76f);
            RoundRect(hdc, x1, y1, x2, y2, 4, 4);

            // Key dots / spacebar
            int my = y + (int)(size * 0.44f);
            MoveToEx(hdc, x1 + (int)(size * 0.10f), my, NULL);
            LineTo(hdc, x1 + (int)(size * 0.18f), my);

            MoveToEx(hdc, x1 + (int)(size * 0.28f), my, NULL);
            LineTo(hdc, x1 + (int)(size * 0.36f), my);

            MoveToEx(hdc, x1 + (int)(size * 0.46f), my, NULL);
            LineTo(hdc, x1 + (int)(size * 0.54f), my);

            // Spacebar
            int sy = y + (int)(size * 0.60f);
            MoveToEx(hdc, x1 + (int)(size * 0.16f), sy, NULL);
            LineTo(hdc, x2 - (int)(size * 0.16f), sy);
            break;
        }

        case ICON_RESET: {
            // Circular reload / reset arrow
            int cx = x + size / 2;
            int cy = y + size / 2;
            int r = (int)(size * 0.34f);
            Arc(hdc, cx - r, cy - r, cx + r + 1, cy + r + 1, cx + r, cy, cx, cy - r);

            // Arrowhead at top right
            POINT arr[3] = {
                { cx + r - (int)(size * 0.16f), cy - (int)(size * 0.18f) },
                { cx + r + (int)(size * 0.04f), cy - (int)(size * 0.02f) },
                { cx + r + (int)(size * 0.18f), cy - (int)(size * 0.18f) }
            };
            Polyline(hdc, arr, 3);
            break;
        }

        case ICON_CPU: {
            // Microprocessor chip with 8 pins
            int x1 = x + (int)(size * 0.26f);
            int y1 = y + (int)(size * 0.26f);
            int x2 = x + (int)(size * 0.74f);
            int y2 = y + (int)(size * 0.74f);

            // Core chip body
            Rectangle(hdc, x1, y1, x2, y2);

            // Inner core die
            SelectObject(hdc, fill_brush);
            Rectangle(hdc, x1 + (int)(size * 0.12f), y1 + (int)(size * 0.12f), x2 - (int)(size * 0.12f), y2 - (int)(size * 0.12f));
            SelectObject(hdc, GetStockObject(NULL_BRUSH));

            // 8 Pins (2 on each of 4 edges)
            int pin_off1 = (int)(size * 0.38f);
            int pin_off2 = (int)(size * 0.62f);

            // Top pins
            MoveToEx(hdc, x + pin_off1, y + (int)(size * 0.10f), NULL); LineTo(hdc, x + pin_off1, y1);
            MoveToEx(hdc, x + pin_off2, y + (int)(size * 0.10f), NULL); LineTo(hdc, x + pin_off2, y1);

            // Bottom pins
            MoveToEx(hdc, x + pin_off1, y2, NULL); LineTo(hdc, x + pin_off1, y + (int)(size * 0.90f));
            MoveToEx(hdc, x + pin_off2, y2, NULL); LineTo(hdc, x + pin_off2, y + (int)(size * 0.90f));

            // Left pins
            MoveToEx(hdc, x + (int)(size * 0.10f), y + pin_off1, NULL); LineTo(hdc, x1, y + pin_off1);
            MoveToEx(hdc, x + (int)(size * 0.10f), y + pin_off2, NULL); LineTo(hdc, x1, y + pin_off2);

            // Right pins
            MoveToEx(hdc, x2, y + pin_off1, NULL); LineTo(hdc, x + (int)(size * 0.90f), y + pin_off1);
            MoveToEx(hdc, x2, y + pin_off2, NULL); LineTo(hdc, x + (int)(size * 0.90f), y + pin_off2);
            break;
        }

        case ICON_GAUGE: {
            // Speedometer / Tachometer
            int cx = x + size / 2;
            int cy = y + (int)(size * 0.65f);
            int r = (int)(size * 0.36f);

            // Arch
            Arc(hdc, cx - r, cy - r, cx + r + 1, cy + r + 1, cx + r, cy, cx - r, cy);

            // Needle pointing top-right (high velocity)
            MoveToEx(hdc, cx, cy, NULL);
            LineTo(hdc, cx + (int)(size * 0.24f), cy - (int)(size * 0.24f));
            break;
        }

        case ICON_MINIMIZE: {
            // Titlebar minimize horizontal bar
            int pad = (int)(size * 0.20f);
            int cy = y + size / 2;
            MoveToEx(hdc, x + pad, cy, NULL);
            LineTo(hdc, x + size - pad, cy);
            break;
        }

        case ICON_MAXIMIZE: {
            // Titlebar maximize square frame
            int pad = (int)(size * 0.20f);
            Rectangle(hdc, x + pad, y + pad, x + size - pad, y + size - pad);
            break;
        }

        case ICON_RESTORE: {
            // Titlebar restore dual overlapping squares
            int pad = (int)(size * 0.18f);
            int d = (int)(size * 0.22f);
            Rectangle(hdc, x + pad + d, y + pad, x + size - pad, y + size - pad - d);
            Rectangle(hdc, x + pad, y + pad + d, x + size - pad - d, y + size - pad);
            break;
        }

        case ICON_GRIP: {
            // 6-dot vertical drag reordering handle
            SelectObject(hdc, fill_brush);
            int r = (size >= 24) ? 2 : 1;
            int col1 = x + (int)(size * 0.35f);
            int col2 = x + (int)(size * 0.65f);
            int row1 = y + (int)(size * 0.25f);
            int row2 = y + (int)(size * 0.50f);
            int row3 = y + (int)(size * 0.75f);

            Ellipse(hdc, col1 - r, row1 - r, col1 + r + 1, row1 + r + 1);
            Ellipse(hdc, col2 - r, row1 - r, col2 + r + 1, row1 + r + 1);
            Ellipse(hdc, col1 - r, row2 - r, col1 + r + 1, row2 + r + 1);
            Ellipse(hdc, col2 - r, row2 - r, col2 + r + 1, row2 + r + 1);
            Ellipse(hdc, col1 - r, row3 - r, col1 + r + 1, row3 + r + 1);
            Ellipse(hdc, col2 - r, row3 - r, col2 + r + 1, row3 + r + 1);
            break;
        }

        case ICON_REPEAT: {
            // Loop / repeat arrows
            int cx = x + size / 2;
            int cy = y + size / 2;
            int r = (int)(size * 0.30f);

            // Upper clockwise arc
            Arc(hdc, cx - r, cy - r, cx + r + 1, cy + r + 1, cx + r, cy, cx - r, cy);
            // Lower clockwise arc
            Arc(hdc, cx - r, cy - r, cx + r + 1, cy + r + 1, cx - r, cy, cx + r, cy);

            // Upper arrowhead
            POINT arr1[3] = {
                { cx + r - (int)(size * 0.12f), cy - (int)(size * 0.12f) },
                { cx + r, cy },
                { cx + r + (int)(size * 0.12f), cy - (int)(size * 0.12f) }
            };
            Polyline(hdc, arr1, 3);

            // Lower arrowhead
            POINT arr2[3] = {
                { cx - r - (int)(size * 0.12f), cy + (int)(size * 0.12f) },
                { cx - r, cy },
                { cx - r + (int)(size * 0.12f), cy + (int)(size * 0.12f) }
            };
            Polyline(hdc, arr2, 3);
            break;
        }

        default:
            break;
    }

    SelectObject(hdc, old_brush);
    SelectObject(hdc, old_pen);
    DeleteObject(pen);
    DeleteObject(fill_brush);
}

void DrawVectorIconCentered(HDC hdc, IconId icon, const RECT* rect, int size, COLORREF color) {
    if (!rect || icon == ICON_NONE || size <= 0) return;
    int w = rect->right - rect->left;
    int h = rect->bottom - rect->top;
    int ix = rect->left + (w - size) / 2;
    int iy = rect->top + (h - size) / 2;
    DrawVectorIcon(hdc, icon, ix, iy, size, color);
}

// ============================================================================
// INTEGRATED THEME BUTTON WITH VECTOR ICON
// ============================================================================
void DrawIndustrialButtonWithIcon(HDC hdc, const RECT* rect, IconId icon, const wchar_t* text, 
                                 bool is_hovered, bool is_primary, bool is_danger, bool is_active_toggle) {
    if (!rect) return;
    
    COLORREF fill_top, fill_bottom, border, text_col;
    
    if (is_active_toggle) {
        fill_top = is_hovered ? RGB(20, 83, 45) : RGB(14, 55, 32);
        fill_bottom = is_hovered ? RGB(10, 48, 26) : RGB(8, 36, 18);
        border = is_hovered ? COLOR_NEON_GREEN : RGB(16, 185, 129);
        text_col = COLOR_TEXT_PRIMARY;
    } else if (is_danger) {
        fill_top = is_hovered ? RGB(153, 27, 27) : RGB(127, 29, 29);
        fill_bottom = is_hovered ? RGB(95, 18, 18) : RGB(69, 10, 10);
        border = is_hovered ? RGB(248, 113, 113) : COLOR_NEON_PINK;
        text_col = COLOR_TEXT_PRIMARY;
    } else if (is_primary) {
        fill_top = is_hovered ? RGB(14, 165, 233) : RGB(2, 132, 199);
        fill_bottom = is_hovered ? RGB(3, 105, 161) : RGB(2, 84, 130);
        border = is_hovered ? RGB(224, 242, 254) : COLOR_NEON_CYAN;
        text_col = RGB(255, 255, 255);
    } else {
        fill_top = is_hovered ? COLOR_BG_CARD_HOVER : COLOR_BG_CARD;
        fill_bottom = is_hovered ? RGB(24, 31, 46) : RGB(16, 21, 31);
        border = is_hovered ? COLOR_BORDER_STRONG : COLOR_BORDER_SUBTLE;
        text_col = is_hovered ? COLOR_TEXT_PRIMARY : COLOR_TEXT_SECONDARY;
    }
    
    DrawRoundedRect(hdc, rect, 6, fill_bottom, border, is_hovered ? 2 : 1);
    
    RECT grad_rc = { rect->left + 1, rect->top + 1, rect->right - 1, rect->bottom - 1 };
    DrawGradientVertical(hdc, &grad_rc, fill_top, fill_bottom);
    
    // Top highlight rim for 3D glass sheen
    if (rect->right - rect->left > 4 && rect->bottom - rect->top > 4) {
        HPEN rim_pen = CreatePen(PS_SOLID, 1, is_primary ? RGB(224, 242, 254) : (is_active_toggle ? RGB(110, 231, 183) : (is_danger ? RGB(248, 113, 113) : RGB(45, 56, 80))));
        HGDIOBJ old_pen = SelectObject(hdc, rim_pen);
        MoveToEx(hdc, rect->left + 4, rect->top + 1, NULL);
        LineTo(hdc, rect->right - 4, rect->top + 1);
        SelectObject(hdc, old_pen);
        DeleteObject(rim_pen);
    }
    
    int btn_w = rect->right - rect->left;
    int btn_h = rect->bottom - rect->top;
    
    SetBkMode(hdc, TRANSPARENT);
    HGDIOBJ old_font = SelectObject(hdc, g_theme_fonts.font_body_bold);

    if (text && text[0] != L'\0') {
        SIZE txt_sz;
        GetTextExtentPoint32W(hdc, text, (int)wcslen(text), &txt_sz);
        
        int icon_sz = (btn_h >= 36) ? 16 : 14;
        int gap = 8;
        
        if (icon != ICON_NONE) {
            int total_content_w = icon_sz + gap + txt_sz.cx;
            int start_x = rect->left + (btn_w - total_content_w) / 2;
            int icon_y = rect->top + (btn_h - icon_sz) / 2;
            
            // Draw vector icon
            DrawVectorIcon(hdc, icon, start_x, icon_y, icon_sz, is_primary ? RGB(255, 255, 255) : text_col);
            
            // Draw text
            RECT text_rect = { start_x + icon_sz + gap, rect->top, rect->right, rect->bottom };
            if (is_primary) {
                RECT shadow_rc = text_rect;
                OffsetRect(&shadow_rc, 1, 1);
                SetTextColor(hdc, RGB(1, 45, 75));
                DrawTextW(hdc, text, -1, &shadow_rc, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
            }
            SetTextColor(hdc, text_col);
            DrawTextW(hdc, text, -1, &text_rect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
        } else {
            // Text only centered
            if (is_primary) {
                RECT shadow_rc = *rect;
                OffsetRect(&shadow_rc, 1, 1);
                SetTextColor(hdc, RGB(1, 45, 75));
                DrawTextW(hdc, text, -1, &shadow_rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
            }
            SetTextColor(hdc, text_col);
            RECT text_rect = *rect;
            DrawTextW(hdc, text, -1, &text_rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
        }
    } else if (icon != ICON_NONE) {
        // Icon-only button (centered)
        int icon_sz = (btn_h >= 32) ? 16 : (btn_h >= 24 ? 14 : 12);
        DrawVectorIconCentered(hdc, icon, rect, icon_sz, is_primary ? RGB(255, 255, 255) : text_col);
    }
    
    SelectObject(hdc, old_font);
}

// ============================================================================
// INTEGRATED THEME HUD BADGE WITH VECTOR ICON
// ============================================================================
void DrawHudBadgeWithIcon(HDC hdc, int x, int y, IconId icon, const wchar_t* text, 
                          COLORREF bg_col, COLORREF text_col, COLORREF border_col, RECT* out_rect) {
    if (!text) return;
    
    SetBkMode(hdc, TRANSPARENT);
    HGDIOBJ old_font = SelectObject(hdc, g_theme_fonts.font_mono_small);
    
    SIZE sz;
    GetTextExtentPoint32W(hdc, text, (int)wcslen(text), &sz);
    
    int icon_sz = 12;
    int gap = (icon != ICON_NONE) ? 6 : 0;
    int icon_space = (icon != ICON_NONE) ? (icon_sz + gap) : 0;
    
    int w = sz.cx + 16 + icon_space;
    int h = 20;
    
    RECT rc = { x, y, x + w, y + h };
    DrawRoundedRect(hdc, &rc, 4, bg_col, border_col, 1);
    
    if (icon != ICON_NONE) {
        int icon_x = x + 6;
        int icon_y = y + (h - icon_sz) / 2;
        DrawVectorIcon(hdc, icon, icon_x, icon_y, icon_sz, text_col);
    }
    
    SetTextColor(hdc, text_col);
    RECT tr = { x + 8 + icon_space, y, x + w - 4, y + h };
    DrawTextW(hdc, text, -1, &tr, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    
    SelectObject(hdc, old_font);
    
    if (out_rect) {
        *out_rect = rc;
    }
}
