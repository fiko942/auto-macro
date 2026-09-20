#include "ui_icons.h"
#include "theme.h"
#include <math.h>
#include <stdio.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef WINGDIPAPI
#define WINGDIPAPI __stdcall
#endif

// ============================================================================
// GDI+ FLAT C API DEFINITIONS & TYPES
// High-performance sub-pixel anti-aliased vector rendering engine for Win32
// ============================================================================
typedef DWORD ARGB;
typedef float REAL;

typedef enum SmoothingMode {
    SmoothingModeInvalid     = -1,
    SmoothingModeDefault     = 0,
    SmoothingModeHighSpeed   = 1,
    SmoothingModeHighQuality = 2,
    SmoothingModeNone        = 3,
    SmoothingModeAntiAlias   = 4
} SmoothingMode;

typedef enum PixelOffsetMode {
    PixelOffsetModeInvalid     = -1,
    PixelOffsetModeDefault     = 0,
    PixelOffsetModeHighSpeed   = 1,
    PixelOffsetModeHighQuality = 2,
    PixelOffsetModeNone        = 3,
    PixelOffsetModeHalf        = 4
} PixelOffsetMode;

typedef enum LineCap {
    LineCapFlat        = 0,
    LineCapSquare      = 1,
    LineCapRound       = 2,
    LineCapTriangle    = 3
} LineCap;

typedef enum LineJoin {
    LineJoinMiter        = 0,
    LineJoinBevel        = 1,
    LineJoinRound        = 2,
    LineJoinMiterClipped = 3
} LineJoin;

typedef enum FillMode {
    FillModeAlternate = 0,
    FillModeWinding   = 1
} FillMode;

typedef enum GpUnit {
    UnitWorld      = 0,
    UnitDisplay    = 1,
    UnitPixel      = 2,
    UnitPoint      = 3,
    UnitInch       = 4,
    UnitDocument   = 5,
    UnitMillimeter = 6
} GpUnit;

typedef struct PointF {
    REAL X;
    REAL Y;
} PointF;

typedef struct GdiplusStartupInput {
    UINT32 GdiplusVersion;
    void *DebugEventCallback;
    BOOL SuppressBackgroundThread;
    BOOL SuppressExternalCodecs;
} GdiplusStartupInput;

typedef void GpGraphics;
typedef void GpPen;
typedef void GpBrush;
typedef void GpSolidFill;
typedef void GpPath;

// Flat GDI+ API prototypes
int WINGDIPAPI GdiplusStartup(ULONG_PTR *token, const GdiplusStartupInput *input, void *output);
void WINGDIPAPI GdiplusShutdown(ULONG_PTR token);

int WINGDIPAPI GdipCreateFromHDC(HDC hdc, GpGraphics **graphics);
int WINGDIPAPI GdipDeleteGraphics(GpGraphics *graphics);
int WINGDIPAPI GdipSetSmoothingMode(GpGraphics *graphics, SmoothingMode smoothingMode);
int WINGDIPAPI GdipSetPixelOffsetMode(GpGraphics *graphics, PixelOffsetMode pixelOffsetMode);

int WINGDIPAPI GdipCreatePen1(ARGB color, REAL width, GpUnit unit, GpPen **pen);
int WINGDIPAPI GdipDeletePen(GpPen *pen);
int WINGDIPAPI GdipSetPenLineJoin(GpPen *pen, LineJoin lineJoin);
int WINGDIPAPI GdipSetPenStartCap(GpPen *pen, LineCap startCap);
int WINGDIPAPI GdipSetPenEndCap(GpPen *pen, LineCap endCap);

int WINGDIPAPI GdipCreateSolidFill(ARGB color, GpSolidFill **brush);
int WINGDIPAPI GdipDeleteBrush(GpBrush *brush);

int WINGDIPAPI GdipDrawLine(GpGraphics *graphics, GpPen *pen, REAL x1, REAL y1, REAL x2, REAL y2);
int WINGDIPAPI GdipDrawLines(GpGraphics *graphics, GpPen *pen, const PointF *points, INT count);
int WINGDIPAPI GdipDrawPolygon(GpGraphics *graphics, GpPen *pen, const PointF *points, INT count);
int WINGDIPAPI GdipFillPolygon(GpGraphics *graphics, GpBrush *brush, const PointF *points, INT count, FillMode fillMode);
int WINGDIPAPI GdipDrawEllipse(GpGraphics *graphics, GpPen *pen, REAL x, REAL y, REAL width, REAL height);
int WINGDIPAPI GdipFillEllipse(GpGraphics *graphics, GpBrush *brush, REAL x, REAL y, REAL width, REAL height);
int WINGDIPAPI GdipDrawArc(GpGraphics *graphics, GpPen *pen, REAL x, REAL y, REAL width, REAL height, REAL startAngle, REAL sweepAngle);
int WINGDIPAPI GdipDrawRectangle(GpGraphics *graphics, GpPen *pen, REAL x, REAL y, REAL width, REAL height);
int WINGDIPAPI GdipFillRectangle(GpGraphics *graphics, GpBrush *brush, REAL x, REAL y, REAL width, REAL height);

int WINGDIPAPI GdipCreatePath(FillMode fillMode, GpPath **path);
int WINGDIPAPI GdipDeletePath(GpPath *path);
int WINGDIPAPI GdipAddPathArc(GpPath *path, REAL x, REAL y, REAL width, REAL height, REAL startAngle, REAL sweepAngle);
int WINGDIPAPI GdipClosePathFigure(GpPath *path);
int WINGDIPAPI GdipDrawPath(GpGraphics *graphics, GpPen *pen, GpPath *path);

// ============================================================================
// LIFECYCLE MANAGEMENT
// ============================================================================
static ULONG_PTR s_gdiplus_token = 0;
static bool s_gdiplus_ready = false;

void UiIcons_Init(void) {
    if (!s_gdiplus_ready) {
        GdiplusStartupInput input = { 1, NULL, FALSE, FALSE };
        if (GdiplusStartup(&s_gdiplus_token, &input, NULL) == 0) {
            s_gdiplus_ready = true;
        }
    }
}

void UiIcons_Cleanup(void) {
    if (s_gdiplus_ready) {
        GdiplusShutdown(s_gdiplus_token);
        s_gdiplus_token = 0;
        s_gdiplus_ready = false;
    }
}

static inline ARGB ColorrefToARGB(COLORREF col, BYTE alpha) {
    return ((ARGB)alpha << 24) |
           ((ARGB)GetRValue(col) << 16) |
           ((ARGB)GetGValue(col) << 8) |
           ((ARGB)GetBValue(col));
}

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
// CORE VECTOR ICON DRAWING ENGINE (GDI+ ANTI-ALIASED SUBPIXEL VECTOR)
// ============================================================================
void DrawVectorIcon(HDC hdc, IconId icon, int x, int y, int size, COLORREF color) {
    if (icon == ICON_NONE || size <= 2 || !hdc) return;

    if (!s_gdiplus_ready) {
        UiIcons_Init();
    }

    GpGraphics *gfx = NULL;
    if (GdipCreateFromHDC(hdc, &gfx) != 0 || !gfx) return;

    // Enable high-quality anti-aliasing
    GdipSetSmoothingMode(gfx, SmoothingModeAntiAlias);
    GdipSetPixelOffsetMode(gfx, PixelOffsetModeHalf);

    float stroke_w = (size >= 32) ? 2.5f : ((size >= 20) ? 1.8f : 1.4f);
    if (stroke_w < 1.2f) stroke_w = 1.2f;

    ARGB argb = ColorrefToARGB(color, 255);

    GpPen *pen = NULL;
    GdipCreatePen1(argb, stroke_w, UnitPixel, &pen);
    if (pen) {
        GdipSetPenLineJoin(pen, LineJoinRound);
        GdipSetPenStartCap(pen, LineCapRound);
        GdipSetPenEndCap(pen, LineCapRound);
    }

    GpSolidFill *fill = NULL;
    GdipCreateSolidFill(argb, &fill);

    float fx = (float)x;
    float fy = (float)y;
    float fs = (float)size;

    switch (icon) {
        case ICON_BOLT: {
            // Cyberpunk sharp lightning bolt (smooth anti-aliased solid fill + outline)
            PointF pts[7] = {
                { fx + fs * 0.58f, fy + fs * 0.04f },
                { fx + fs * 0.20f, fy + fs * 0.52f },
                { fx + fs * 0.48f, fy + fs * 0.52f },
                { fx + fs * 0.38f, fy + fs * 0.96f },
                { fx + fs * 0.80f, fy + fs * 0.42f },
                { fx + fs * 0.52f, fy + fs * 0.42f },
                { fx + fs * 0.62f, fy + fs * 0.04f }
            };
            if (fill) GdipFillPolygon(gfx, (GpBrush*)fill, pts, 7, FillModeWinding);
            if (pen) GdipDrawPolygon(gfx, pen, pts, 7);
            break;
        }

        case ICON_SETTINGS: {
            // Precision 8-tooth mechanical gear / cog
            float cx = fx + fs * 0.5f;
            float cy = fy + fs * 0.5f;
            float r_outer = fs * 0.44f;
            float r_inner = fs * 0.30f;
            float r_hole  = fs * 0.14f;
            if (r_hole < 2.0f) r_hole = 2.0f;

            // Draw outer circle
            if (pen) GdipDrawEllipse(gfx, pen, cx - r_inner, cy - r_inner, r_inner * 2.0f, r_inner * 2.0f);

            // Draw 4 through-bars (8 teeth) with rounded ends
            if (pen) {
                for (int a = 0; a < 4; a++) {
                    double angle = a * (M_PI / 4.0);
                    float dx = (float)(cos(angle) * r_outer);
                    float dy = (float)(sin(angle) * r_outer);
                    GdipDrawLine(gfx, pen, cx - dx, cy - dy, cx + dx, cy + dy);
                }
            }

            // Central axle hole
            if (pen) GdipDrawEllipse(gfx, pen, cx - r_hole, cy - r_hole, r_hole * 2.0f, r_hole * 2.0f);
            break;
        }

        case ICON_INFO: {
            // Info circle with centered "i"
            float pad = fs * 0.08f;
            if (pen) GdipDrawEllipse(gfx, pen, fx + pad, fy + pad, fs - pad * 2.0f, fs - pad * 2.0f);
            
            float cx = fx + fs * 0.5f;
            
            // Top dot
            float dot_y = fy + fs * 0.28f;
            float dot_r = (fs >= 20.0f) ? 1.5f : 1.1f;
            if (fill) GdipFillEllipse(gfx, (GpBrush*)fill, cx - dot_r, dot_y - dot_r, dot_r * 2.0f, dot_r * 2.0f);

            // Stem and bottom serif
            float stem_top = fy + fs * 0.42f;
            float stem_bot = fy + fs * 0.72f;
            if (pen) {
                GdipDrawLine(gfx, pen, cx, stem_top, cx, stem_bot);
                GdipDrawLine(gfx, pen, cx - fs * 0.10f, stem_bot, cx + fs * 0.10f, stem_bot);
            }
            break;
        }

        case ICON_PLUS: {
            // Clean centered plus sign
            float pad = fs * 0.20f;
            float cx = fx + fs * 0.5f;
            float cy = fy + fs * 0.5f;
            if (pen) {
                GdipDrawLine(gfx, pen, cx, fy + pad, cx, fy + fs - pad);
                GdipDrawLine(gfx, pen, fx + pad, cy, fx + fs - pad, cy);
            }
            break;
        }

        case ICON_CLOSE: {
            // Clean diagonal cross / X
            float pad = fs * 0.22f;
            if (pen) {
                GdipDrawLine(gfx, pen, fx + pad, fy + pad, fx + fs - pad, fy + fs - pad);
                GdipDrawLine(gfx, pen, fx + fs - pad, fy + pad, fx + pad, fy + fs - pad);
            }
            break;
        }

        case ICON_EDIT: {
            // Diagonal pencil with sharp tip
            PointF body[4] = {
                { fx + fs * 0.62f, fy + fs * 0.14f },
                { fx + fs * 0.86f, fy + fs * 0.38f },
                { fx + fs * 0.38f, fy + fs * 0.86f },
                { fx + fs * 0.14f, fy + fs * 0.62f }
            };
            if (pen) GdipDrawPolygon(gfx, pen, body, 4);

            PointF tip[3] = {
                { fx + fs * 0.38f, fy + fs * 0.86f },
                { fx + fs * 0.14f, fy + fs * 0.62f },
                { fx + fs * 0.08f, fy + fs * 0.92f }
            };
            if (fill) GdipFillPolygon(gfx, (GpBrush*)fill, tip, 3, FillModeWinding);
            if (pen) GdipDrawPolygon(gfx, pen, tip, 3);
            break;
        }

        case ICON_TRASH: {
            // Wastebasket container with lid & handle
            float y_lid = fy + fs * 0.28f;

            if (pen) {
                // Lid handle
                GdipDrawLine(gfx, pen, fx + fs * 0.38f, y_lid, fx + fs * 0.38f, fy + fs * 0.16f);
                GdipDrawLine(gfx, pen, fx + fs * 0.38f, fy + fs * 0.16f, fx + fs * 0.62f, fy + fs * 0.16f);
                GdipDrawLine(gfx, pen, fx + fs * 0.62f, fy + fs * 0.16f, fx + fs * 0.62f, y_lid);

                // Lid bar
                GdipDrawLine(gfx, pen, fx + fs * 0.16f, y_lid, fx + fs * 0.84f, y_lid);

                // Bin Body
                PointF body[4] = {
                    { fx + fs * 0.24f, y_lid + 2.0f },
                    { fx + fs * 0.28f, fy + fs * 0.88f },
                    { fx + fs * 0.72f, fy + fs * 0.88f },
                    { fx + fs * 0.76f, y_lid + 2.0f }
                };
                GdipDrawPolygon(gfx, pen, body, 4);

                // Center ribs
                GdipDrawLine(gfx, pen, fx + fs * 0.42f, y_lid + 4.0f, fx + fs * 0.42f, fy + fs * 0.80f);
                GdipDrawLine(gfx, pen, fx + fs * 0.58f, y_lid + 4.0f, fx + fs * 0.58f, fy + fs * 0.80f);
            }
            break;
        }

        case ICON_IMPORT: {
            // Bottom tray + downward arrow
            PointF tray[4] = {
                { fx + fs * 0.16f, fy + fs * 0.58f },
                { fx + fs * 0.16f, fy + fs * 0.86f },
                { fx + fs * 0.84f, fy + fs * 0.86f },
                { fx + fs * 0.84f, fy + fs * 0.58f }
            };
            if (pen) {
                GdipDrawLines(gfx, pen, tray, 4);

                // Arrow down
                float cx = fx + fs * 0.5f;
                GdipDrawLine(gfx, pen, cx, fy + fs * 0.14f, cx, fy + fs * 0.62f);
                
                PointF head[3] = {
                    { cx - fs * 0.20f, fy + fs * 0.44f },
                    { cx, fy + fs * 0.64f },
                    { cx + fs * 0.20f, fy + fs * 0.44f }
                };
                GdipDrawLines(gfx, pen, head, 3);
            }
            break;
        }

        case ICON_EXPORT: {
            // Bottom tray + upward arrow
            PointF tray[4] = {
                { fx + fs * 0.16f, fy + fs * 0.58f },
                { fx + fs * 0.16f, fy + fs * 0.86f },
                { fx + fs * 0.84f, fy + fs * 0.86f },
                { fx + fs * 0.84f, fy + fs * 0.58f }
            };
            if (pen) {
                GdipDrawLines(gfx, pen, tray, 4);

                // Arrow up
                float cx = fx + fs * 0.5f;
                GdipDrawLine(gfx, pen, cx, fy + fs * 0.64f, cx, fy + fs * 0.16f);
                
                PointF head[3] = {
                    { cx - fs * 0.20f, fy + fs * 0.36f },
                    { cx, fy + fs * 0.16f },
                    { cx + fs * 0.20f, fy + fs * 0.36f }
                };
                GdipDrawLines(gfx, pen, head, 3);
            }
            break;
        }

        case ICON_SAVE: {
            // Storage chip / floppy disk
            float x1 = fx + fs * 0.14f, y1 = fy + fs * 0.14f;
            float x2 = fx + fs * 0.86f, y2 = fy + fs * 0.86f;
            float chamfer = fs * 0.14f;

            PointF disk[6] = {
                { x1, y1 },
                { x2 - chamfer, y1 },
                { x2, y1 + chamfer },
                { x2, y2 },
                { x1, y2 },
                { x1, y1 }
            };
            if (pen) {
                GdipDrawPolygon(gfx, pen, disk, 6);
                GdipDrawRectangle(gfx, pen, x1 + fs * 0.14f, y1, (x2 - x1) - fs * 0.28f, fs * 0.26f);
                GdipDrawRectangle(gfx, pen, x1 + fs * 0.10f, y2 - fs * 0.32f, (x2 - x1) - fs * 0.20f, fs * 0.32f);
            }
            break;
        }

        case ICON_FOLDER: {
            // Directory folder with tab
            float x1 = fx + fs * 0.12f, y1 = fy + fs * 0.22f;
            float x2 = fx + fs * 0.88f, y2 = fy + fs * 0.84f;
            float tab_w = fs * 0.34f, tab_h = fs * 0.14f;

            PointF folder[7] = {
                { x1, y1 },
                { x1 + tab_w, y1 },
                { x1 + tab_w + fs * 0.08f, y1 + tab_h },
                { x2, y1 + tab_h },
                { x2, y2 },
                { x1, y2 },
                { x1, y1 }
            };
            if (pen) GdipDrawPolygon(gfx, pen, folder, 7);
            break;
        }

        case ICON_COPY: {
            // Dual overlapping clipboard sheets
            float off = fs * 0.16f;
            PointF back[5] = {
                { fx + off + fs * 0.14f, fy + fs * 0.12f },
                { fx + fs - fs * 0.12f, fy + fs * 0.12f },
                { fx + fs - fs * 0.12f, fy + fs - off - fs * 0.12f },
                { fx + off + fs * 0.14f, fy + fs - off - fs * 0.12f },
                { fx + off + fs * 0.14f, fy + fs * 0.12f }
            };
            if (pen) GdipDrawLines(gfx, pen, back, 5);

            float fx1 = fx + fs * 0.12f, fy1 = fy + off + fs * 0.12f;
            float fw = fs * 0.58f, fh = fs * 0.60f;

            GpSolidFill* bg_f = NULL;
            GdipCreateSolidFill(ColorrefToARGB(COLOR_BG_CARD, 255), &bg_f);
            if (bg_f) {
                GdipFillRectangle(gfx, (GpBrush*)bg_f, fx1, fy1, fw, fh);
                GdipDeleteBrush((GpBrush*)bg_f);
            }
            if (pen) GdipDrawRectangle(gfx, pen, fx1, fy1, fw, fh);
            break;
        }

        case ICON_SHIELD: {
            // Security shield outline + vertical dividing crest
            float x1 = fx + fs * 0.18f;
            float x2 = fx + fs * 0.82f;
            float y1 = fy + fs * 0.14f;
            float ym = fy + fs * 0.52f;
            float y2 = fy + fs * 0.90f;
            float cx = fx + fs * 0.5f;

            PointF shield[6] = {
                { x1, y1 },
                { x2, y1 },
                { x2, ym },
                { cx, y2 },
                { x1, ym },
                { x1, y1 }
            };
            if (pen) {
                GdipDrawPolygon(gfx, pen, shield, 6);
                GdipDrawLine(gfx, pen, cx, y1 + 2.0f, cx, y2 - 2.0f);
            }
            break;
        }

        case ICON_TARGET: {
            // Precision crosshair reticle
            float cx = fx + fs * 0.5f;
            float cy = fy + fs * 0.5f;
            float r_outer = fs * 0.38f;
            float r_inner = fs * 0.16f;

            if (pen) {
                // Outer circle
                GdipDrawEllipse(gfx, pen, cx - r_outer, cy - r_outer, r_outer * 2.0f, r_outer * 2.0f);
                // Inner circle
                GdipDrawEllipse(gfx, pen, cx - r_inner, cy - r_inner, r_inner * 2.0f, r_inner * 2.0f);

                // 4 Crosshair ticks
                GdipDrawLine(gfx, pen, cx, fy + fs * 0.04f, cx, cy - r_inner);
                GdipDrawLine(gfx, pen, cx, cy + r_inner, cx, fy + fs * 0.96f);
                GdipDrawLine(gfx, pen, fx + fs * 0.04f, cy, cx - r_inner, cy);
                GdipDrawLine(gfx, pen, cx + r_inner, cy, fx + fs * 0.96f, cy);
            }
            break;
        }

        case ICON_CLOCK: {
            // Clock face with hour and minute hands
            float pad = fs * 0.10f;
            float cx = fx + fs * 0.5f;
            float cy = fy + fs * 0.5f;
            if (pen) {
                GdipDrawEllipse(gfx, pen, fx + pad, fy + pad, fs - pad * 2.0f, fs - pad * 2.0f);
                // Hour hand (pointing to 12)
                GdipDrawLine(gfx, pen, cx, cy, cx, cy - fs * 0.26f);
                // Minute hand (pointing to 3)
                GdipDrawLine(gfx, pen, cx, cy, cx + fs * 0.22f, cy);
            }
            break;
        }

        case ICON_LOCK: {
            // Padlock: Top shackle + rounded body + keyhole
            float bx1 = fx + fs * 0.22f;
            float bx2 = fx + fs * 0.78f;
            float by1 = fy + fs * 0.44f;
            float by2 = fy + fs * 0.88f;

            float sx1 = fx + fs * 0.32f;
            float sx2 = fx + fs * 0.68f;
            float sy1 = fy + fs * 0.16f;

            if (pen) {
                // Shackle
                PointF shackle[4] = {
                    { sx1, by1 },
                    { sx1, sy1 },
                    { sx2, sy1 },
                    { sx2, by1 }
                };
                GdipDrawLines(gfx, pen, shackle, 4);

                // Body (rounded rectangle)
                GpPath* lock_path = NULL;
                if (GdipCreatePath(FillModeWinding, &lock_path) == 0 && lock_path) {
                    float rad = fs * 0.08f;
                    GdipAddPathArc(lock_path, bx1, by1, rad * 2.0f, rad * 2.0f, 180.0f, 90.0f);
                    GdipAddPathArc(lock_path, bx2 - rad * 2.0f, by1, rad * 2.0f, rad * 2.0f, 270.0f, 90.0f);
                    GdipAddPathArc(lock_path, bx2 - rad * 2.0f, by2 - rad * 2.0f, rad * 2.0f, rad * 2.0f, 0.0f, 90.0f);
                    GdipAddPathArc(lock_path, bx1, by2 - rad * 2.0f, rad * 2.0f, rad * 2.0f, 90.0f, 90.0f);
                    GdipClosePathFigure(lock_path);
                    GdipDrawPath(gfx, pen, lock_path);
                    GdipDeletePath(lock_path);
                }

                // Keyhole
                float cx = fx + fs * 0.5f;
                GdipDrawLine(gfx, pen, cx, by1 + fs * 0.12f, cx, by1 + fs * 0.26f);
            }
            break;
        }

        case ICON_ARROW_DOWN: {
            // Downward arrow
            float cx = fx + fs * 0.5f;
            if (pen) {
                GdipDrawLine(gfx, pen, cx, fy + fs * 0.16f, cx, fy + fs * 0.82f);

                PointF head[3] = {
                    { cx - fs * 0.26f, fy + fs * 0.54f },
                    { cx, fy + fs * 0.82f },
                    { cx + fs * 0.26f, fy + fs * 0.54f }
                };
                GdipDrawLines(gfx, pen, head, 3);
            }
            break;
        }

        case ICON_ARROW_UP: {
            // Upward arrow
            float cx = fx + fs * 0.5f;
            if (pen) {
                GdipDrawLine(gfx, pen, cx, fy + fs * 0.82f, cx, fy + fs * 0.16f);

                PointF head[3] = {
                    { cx - fs * 0.26f, fy + fs * 0.44f },
                    { cx, fy + fs * 0.16f },
                    { cx + fs * 0.26f, fy + fs * 0.44f }
                };
                GdipDrawLines(gfx, pen, head, 3);
            }
            break;
        }

        case ICON_SEQUENCE: {
            // 3 Stepping block nodes connected by a flow line
            float r = (fs >= 24.0f) ? 2.5f : 1.8f;
            float p1_x = fx + fs * 0.22f, p1_y = fy + fs * 0.28f;
            float p2_x = fx + fs * 0.50f, p2_y = fy + fs * 0.50f;
            float p3_x = fx + fs * 0.78f, p3_y = fy + fs * 0.72f;

            if (pen) {
                PointF flow[3] = { { p1_x, p1_y }, { p2_x, p2_y }, { p3_x, p3_y } };
                GdipDrawLines(gfx, pen, flow, 3);
            }

            // Node dots
            if (fill) {
                GdipFillEllipse(gfx, (GpBrush*)fill, p1_x - r, p1_y - r, r * 2.0f, r * 2.0f);
                GdipFillEllipse(gfx, (GpBrush*)fill, p2_x - r, p2_y - r, r * 2.0f, r * 2.0f);
                GdipFillEllipse(gfx, (GpBrush*)fill, p3_x - r, p3_y - r, r * 2.0f, r * 2.0f);
            }
            break;
        }

        case ICON_BELL: {
            // Tactical chime bell
            float bx1 = fx + fs * 0.20f;
            float bx2 = fx + fs * 0.80f;
            float by_rim = fy + fs * 0.72f;
            float cx = fx + fs * 0.5f;

            PointF bell[5] = {
                { bx1, by_rim },
                { cx - fs * 0.16f, fy + fs * 0.26f },
                { cx + fs * 0.16f, fy + fs * 0.26f },
                { bx2, by_rim },
                { bx1, by_rim }
            };
            if (pen) {
                GdipDrawPolygon(gfx, pen, bell, 5);
                // Top loop
                GdipDrawLine(gfx, pen, cx, fy + fs * 0.26f, cx, fy + fs * 0.14f);
                // Clapper
                GdipDrawLine(gfx, pen, cx - fs * 0.08f, by_rim + 1.0f, cx + fs * 0.08f, by_rim + 1.0f);
            }
            break;
        }

        case ICON_STATS: {
            // 3-Bar histogram with baseline
            float y_base = fy + fs * 0.84f;
            if (pen) GdipDrawLine(gfx, pen, fx + fs * 0.12f, y_base, fx + fs * 0.88f, y_base);

            if (fill) {
                float bw = fs * 0.14f;
                // Bar 1
                GdipFillRectangle(gfx, (GpBrush*)fill, fx + fs * 0.20f, fy + fs * 0.54f, bw, y_base - (fy + fs * 0.54f));
                // Bar 2
                GdipFillRectangle(gfx, (GpBrush*)fill, fx + fs * 0.42f, fy + fs * 0.32f, bw, y_base - (fy + fs * 0.32f));
                // Bar 3
                GdipFillRectangle(gfx, (GpBrush*)fill, fx + fs * 0.64f, fy + fs * 0.16f, bw, y_base - (fy + fs * 0.16f));
            }
            break;
        }

        case ICON_TOOLS: {
            // Diagonal wrench
            float x1 = fx + fs * 0.22f;
            float y1 = fy + fs * 0.78f;
            float x2 = fx + fs * 0.72f;
            float y2 = fy + fs * 0.28f;

            if (pen) {
                // Handle
                GdipDrawLine(gfx, pen, x1, y1, x2, y2);

                // Wrench jaw at top-right
                GdipDrawLine(gfx, pen, x2 - fs * 0.12f, y2 + fs * 0.06f, x2 + fs * 0.12f, y2 - fs * 0.14f);
                GdipDrawLine(gfx, pen, x2 + fs * 0.12f, y2 - fs * 0.14f, x2 + fs * 0.18f, y2 - fs * 0.04f);

                // Ring at bottom-left
                float r = (fs >= 20.0f) ? 2.5f : 1.8f;
                GdipDrawEllipse(gfx, pen, x1 - r, y1 - r, r * 2.0f, r * 2.0f);
            }
            break;
        }

        case ICON_CHECK: {
            // Crisp verification checkmark
            PointF check[3] = {
                { fx + fs * 0.16f, fy + fs * 0.52f },
                { fx + fs * 0.40f, fy + fs * 0.78f },
                { fx + fs * 0.84f, fy + fs * 0.22f }
            };
            if (pen) GdipDrawLines(gfx, pen, check, 3);
            break;
        }

        case ICON_PLAY: {
            // Right-pointing play triangle
            PointF tri[3] = {
                { fx + fs * 0.26f, fy + fs * 0.18f },
                { fx + fs * 0.82f, fy + fs * 0.50f },
                { fx + fs * 0.26f, fy + fs * 0.82f }
            };
            if (fill) GdipFillPolygon(gfx, (GpBrush*)fill, tri, 3, FillModeWinding);
            if (pen) GdipDrawPolygon(gfx, pen, tri, 3);
            break;
        }

        case ICON_STOP: {
            // Solid stop square
            float pad = fs * 0.24f;
            if (fill) GdipFillRectangle(gfx, (GpBrush*)fill, fx + pad, fy + pad, fs - pad * 2.0f, fs - pad * 2.0f);
            if (pen) GdipDrawRectangle(gfx, pen, fx + pad, fy + pad, fs - pad * 2.0f, fs - pad * 2.0f);
            break;
        }

        case ICON_KEYBOARD: {
            // Physical keyboard frame + keys
            float x1 = fx + fs * 0.12f;
            float y1 = fy + fs * 0.24f;
            float x2 = fx + fs * 0.88f;
            float y2 = fy + fs * 0.76f;

            if (pen) {
                GdipDrawRectangle(gfx, pen, x1, y1, x2 - x1, y2 - y1);

                // Key dots / spacebar
                float my = fy + fs * 0.44f;
                GdipDrawLine(gfx, pen, x1 + fs * 0.10f, my, x1 + fs * 0.18f, my);
                GdipDrawLine(gfx, pen, x1 + fs * 0.28f, my, x1 + fs * 0.36f, my);
                GdipDrawLine(gfx, pen, x1 + fs * 0.46f, my, x1 + fs * 0.54f, my);

                // Spacebar
                float sy = fy + fs * 0.60f;
                GdipDrawLine(gfx, pen, x1 + fs * 0.16f, sy, x2 - fs * 0.16f, sy);
            }
            break;
        }

        case ICON_RESET: {
            // Circular reload / reset arrow
            float cx = fx + fs * 0.5f;
            float cy = fy + fs * 0.5f;
            float r = fs * 0.34f;

            if (pen) {
                GdipDrawArc(gfx, pen, cx - r, cy - r, r * 2.0f, r * 2.0f, 0.0f, 270.0f);

                // Arrowhead at top right
                PointF arr[3] = {
                    { cx + r - fs * 0.16f, cy - fs * 0.18f },
                    { cx + r + fs * 0.04f, cy - fs * 0.02f },
                    { cx + r + fs * 0.18f, cy - fs * 0.18f }
                };
                GdipDrawLines(gfx, pen, arr, 3);
            }
            break;
        }

        case ICON_CPU: {
            // Microprocessor chip with 8 pins
            float x1 = fx + fs * 0.26f;
            float y1 = fy + fs * 0.26f;
            float w = fs * 0.48f;
            float h = fs * 0.48f;

            if (pen) {
                GdipDrawRectangle(gfx, pen, x1, y1, w, h);
                if (fill) GdipFillRectangle(gfx, (GpBrush*)fill, x1 + fs * 0.10f, y1 + fs * 0.10f, w - fs * 0.20f, h - fs * 0.20f);

                // 8 Pins (2 on each of 4 edges)
                float p1 = fs * 0.38f;
                float p2 = fs * 0.62f;

                // Top pins
                GdipDrawLine(gfx, pen, fx + p1, fy + fs * 0.10f, fx + p1, y1);
                GdipDrawLine(gfx, pen, fx + p2, fy + fs * 0.10f, fx + p2, y1);

                // Bottom pins
                GdipDrawLine(gfx, pen, fx + p1, y1 + h, fx + p1, fy + fs * 0.90f);
                GdipDrawLine(gfx, pen, fx + p2, y1 + h, fx + p2, fy + fs * 0.90f);

                // Left pins
                GdipDrawLine(gfx, pen, fx + fs * 0.10f, fy + p1, x1, fy + p1);
                GdipDrawLine(gfx, pen, fx + fs * 0.10f, fy + p2, x1, fy + p2);

                // Right pins
                GdipDrawLine(gfx, pen, x1 + w, fy + p1, fx + fs * 0.90f, fy + p1);
                GdipDrawLine(gfx, pen, x1 + w, fy + p2, fx + fs * 0.90f, fy + p2);
            }
            break;
        }

        case ICON_GAUGE: {
            // Speedometer / Tachometer
            float cx = fx + fs * 0.5f;
            float cy = fy + fs * 0.65f;
            float r = fs * 0.36f;

            if (pen) {
                // Arch
                GdipDrawArc(gfx, pen, cx - r, cy - r, r * 2.0f, r * 2.0f, 180.0f, 180.0f);

                // Needle pointing top-right (high velocity)
                GdipDrawLine(gfx, pen, cx, cy, cx + fs * 0.24f, cy - fs * 0.24f);
            }
            break;
        }

        case ICON_MINIMIZE: {
            // Titlebar minimize horizontal bar
            float pad = fs * 0.20f;
            float cy = fy + fs * 0.5f;
            if (pen) GdipDrawLine(gfx, pen, fx + pad, cy, fx + fs - pad, cy);
            break;
        }

        case ICON_MAXIMIZE: {
            // Titlebar maximize square frame
            float pad = fs * 0.20f;
            if (pen) GdipDrawRectangle(gfx, pen, fx + pad, fy + pad, fs - pad * 2.0f, fs - pad * 2.0f);
            break;
        }

        case ICON_RESTORE: {
            // Titlebar restore dual overlapping squares
            float pad = fs * 0.18f;
            float d = fs * 0.22f;
            if (pen) {
                GdipDrawRectangle(gfx, pen, fx + pad + d, fy + pad, fs - pad * 2.0f - d, fs - pad * 2.0f - d);
                GdipDrawRectangle(gfx, pen, fx + pad, fy + pad + d, fs - pad * 2.0f - d, fs - pad * 2.0f - d);
            }
            break;
        }

        case ICON_GRIP: {
            // 6-dot vertical drag reordering handle
            float r = (fs >= 24.0f) ? 1.8f : 1.2f;
            float col1 = fx + fs * 0.35f;
            float col2 = fx + fs * 0.65f;
            float row1 = fy + fs * 0.25f;
            float row2 = fy + fs * 0.50f;
            float row3 = fy + fs * 0.75f;

            if (fill) {
                GdipFillEllipse(gfx, (GpBrush*)fill, col1 - r, row1 - r, r * 2.0f, r * 2.0f);
                GdipFillEllipse(gfx, (GpBrush*)fill, col2 - r, row1 - r, r * 2.0f, r * 2.0f);
                GdipFillEllipse(gfx, (GpBrush*)fill, col1 - r, row2 - r, r * 2.0f, r * 2.0f);
                GdipFillEllipse(gfx, (GpBrush*)fill, col2 - r, row2 - r, r * 2.0f, r * 2.0f);
                GdipFillEllipse(gfx, (GpBrush*)fill, col1 - r, row3 - r, r * 2.0f, r * 2.0f);
                GdipFillEllipse(gfx, (GpBrush*)fill, col2 - r, row3 - r, r * 2.0f, r * 2.0f);
            }
            break;
        }

        case ICON_REPEAT: {
            // Loop / repeat arrows
            float cx = fx + fs * 0.5f;
            float cy = fy + fs * 0.5f;
            float r = fs * 0.30f;

            if (pen) {
                // Upper clockwise arc
                GdipDrawArc(gfx, pen, cx - r, cy - r, r * 2.0f, r * 2.0f, 200.0f, 140.0f);
                // Lower clockwise arc
                GdipDrawArc(gfx, pen, cx - r, cy - r, r * 2.0f, r * 2.0f, 20.0f, 140.0f);

                // Upper arrowhead
                PointF arr1[3] = {
                    { cx + r - fs * 0.12f, cy - fs * 0.12f },
                    { cx + r, cy },
                    { cx + r + fs * 0.12f, cy - fs * 0.12f }
                };
                GdipDrawLines(gfx, pen, arr1, 3);

                // Lower arrowhead
                PointF arr2[3] = {
                    { cx - r - fs * 0.12f, cy + fs * 0.12f },
                    { cx - r, cy },
                    { cx - r + fs * 0.12f, cy + fs * 0.12f }
                };
                GdipDrawLines(gfx, pen, arr2, 3);
            }
            break;
        }

        default:
            break;
    }

    if (fill) GdipDeleteBrush((GpBrush*)fill);
    if (pen) GdipDeletePen(pen);
    if (gfx) GdipDeleteGraphics(gfx);
}

void DrawVectorIconCentered(HDC hdc, IconId icon, const RECT* rect, int size, COLORREF color) {
    if (!rect || icon == ICON_NONE || size <= 0 || !hdc) return;
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
    if (!rect || !hdc) return;
    
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
    if (!text || !hdc) return;
    
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
    if (out_rect) *out_rect = rc;
}
