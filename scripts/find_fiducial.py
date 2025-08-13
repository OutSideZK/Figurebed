import argparse
import json
import math
from dataclasses import dataclass
from typing import List, Tuple

import cv2
import numpy as np


@dataclass
class Fiducial:
    center: Tuple[int, int]
    radius: int
    score: float


def load_to_u8_gray(path: str) -> np.ndarray:
    img = cv2.imread(path, cv2.IMREAD_UNCHANGED)
    if img is None:
        raise FileNotFoundError(f"Cannot read image: {path}")
    if img.ndim == 3:
        img = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    if img.dtype != np.uint8:
        img = cv2.normalize(img, None, 0, 255, cv2.NORM_MINMAX)
        img = img.astype(np.uint8)
    return img


def auto_canny(img_u8: np.ndarray, sigma: float = 0.33) -> Tuple[int, int]:
    v = np.median(img_u8)
    lower = int(max(0, (1.0 - sigma) * v))
    upper = int(min(255, (1.0 + sigma) * v))
    if lower == upper:
        lower = max(0, upper - 30)
    return lower, upper


def point_line_distance(px, py, x1, y1, x2, y2) -> float:
    A = px - x1
    B = py - y1
    C = x2 - x1
    D = y2 - y1
    dot = A * C + B * D
    len_sq = C * C + D * D
    if len_sq == 0:
        return math.hypot(px - x1, py - y1)
    t = max(0.0, min(1.0, dot / len_sq))
    xx = x1 + t * C
    yy = y1 + t * D
    return math.hypot(px - xx, py - yy)


def angle_deg(x1, y1, x2, y2) -> float:
    ang = math.degrees(math.atan2(y2 - y1, x2 - x1))
    if ang < 0:
        ang += 180
    return ang


def has_cross_in_circle(roi_gray: np.ndarray,
                        circle_center_roi: Tuple[int, int],
                        radius: int,
                        debug: bool = False):
    h, w = roi_gray.shape[:2]
    cx, cy = circle_center_roi

    blur = cv2.GaussianBlur(roi_gray, (5, 5), 0)
    lo, hi = auto_canny(blur, 0.33)
    edges = cv2.Canny(blur, lo, hi)

    diameter = 2 * radius
    min_line_len = max(10, int(0.6 * diameter))
    max_line_gap = max(2, int(0.08 * diameter))
    lines = cv2.HoughLinesP(edges, rho=1, theta=np.pi / 180, threshold=40,
                            minLineLength=min_line_len, maxLineGap=max_line_gap)

    if lines is None:
        return False, 0.0, []

    center_tol = max(2, int(0.12 * radius))
    center_lines = []
    for l in lines[:, 0, :]:
        x1, y1, x2, y2 = map(int, l)
        dist = point_line_distance(cx, cy, x1, y1, x2, y2)
        if dist <= center_tol:
            center_lines.append((x1, y1, x2, y2))

    if len(center_lines) < 2:
        return False, 0.0, []

    horiz_like = []
    vert_like = []
    for x1, y1, x2, y2 in center_lines:
        ang = angle_deg(x1, y1, x2, y2)
        d0 = min(abs(ang - 0), abs(ang - 180))
        d90 = abs(ang - 90)
        if d0 <= 15:
            horiz_like.append((x1, y1, x2, y2))
        elif d90 <= 15:
            vert_like.append((x1, y1, x2, y2))

    if not horiz_like or not vert_like:
        return False, 0.0, []

    def line_len(l):
        return math.hypot(l[2] - l[0], l[3] - l[1])

    best_h = max(horiz_like, key=line_len)
    best_v = max(vert_like, key=line_len)
    len_h = line_len(best_h)
    len_v = line_len(best_v)

    def center_offset(l):
        x1, y1, x2, y2 = l
        mx = (x1 + x2) / 2.0
        my = (y1 + y2) / 2.0
        return math.hypot(mx - cx, my - cy)

    off_h = center_offset(best_h) / (radius + 1e-6)
    off_v = center_offset(best_v) / (radius + 1e-6)

    span_score = 0.5 * (len_h / (2 * radius + 1e-6) + len_v / (2 * radius + 1e-6))
    center_score = 1.0 - 0.5 * (min(1.0, off_h) + min(1.0, off_v))
    score = max(0.0, min(1.0, 0.7 * span_score + 0.3 * center_score))

    if len_h < 0.55 * 2 * radius or len_v < 0.55 * 2 * radius:
        return False, score, [best_h, best_v]

    return True, score, [best_h, best_v]


def detect_crosshair_fiducials(gray_u8: np.ndarray,
                               dp: float = 1.2,
                               min_dist: float = 30,
                               param1: float = 120,
                               param2: float = 30,
                               min_radius: int = 0,
                               max_radius: int = 0,
                               debug: bool = False) -> List[Fiducial]:
    img = gray_u8.copy()

    clahe = cv2.createCLAHE(clipLimit=2.0, tileGridSize=(8, 8))
    img = clahe.apply(img)

    img_blur = cv2.GaussianBlur(img, (7, 7), 1.2)

    h, w = img.shape[:2]
    min_dim = min(h, w)
    if min_radius <= 0:
        min_radius = max(5, int(0.01 * min_dim))
    if max_radius <= 0:
        max_radius = int(0.15 * min_dim)
    if min_radius >= max_radius:
        max_radius = min_radius + 5

    circles = cv2.HoughCircles(img_blur,
                               cv2.HOUGH_GRADIENT,
                               dp=dp,
                               minDist=min_dist,
                               param1=param1,
                               param2=param2,
                               minRadius=min_radius,
                               maxRadius=max_radius)
    fiducials: List[Fiducial] = []
    if circles is None:
        return fiducials

    circles = np.uint16(np.around(circles[0]))
    for x, y, r in circles:
        pad = int(r * 0.25)
        x1 = max(0, x - r - pad)
        y1 = max(0, y - r - pad)
        x2 = min(w - 1, x + r + pad)
        y2 = min(h - 1, y + r + pad)

        roi = gray_u8[y1:y2 + 1, x1:x2 + 1]
        cx_roi = x - x1
        cy_roi = y - y1

        ok, score, _ = has_cross_in_circle(roi, (cx_roi, cy_roi), r, debug=debug)
        if ok:
            fiducials.append(Fiducial(center=(int(x), int(y)), radius=int(r), score=float(score)))

    fiducials.sort(key=lambda f: f.score, reverse=True)
    return fiducials


def draw_results(src_gray: np.ndarray, fiducials: List[Fiducial]) -> np.ndarray:
    color = cv2.cvtColor(src_gray, cv2.COLOR_GRAY2BGR)
    for f in fiducials:
        x, y = f.center
        r = f.radius
        cv2.circle(color, (x, y), r, (0, 255, 0), 2)
        cv2.circle(color, (x, y), 2, (0, 0, 255), -1)
        cv2.putText(color, f"{f.score:.2f}", (x + r + 3, y),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 0, 0), 1, cv2.LINE_AA)
    return color


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--image", "-i", required=True, help="Path to image (e.g., image/fiducial.tif)")
    ap.add_argument("--save", "-o", default="", help="Path to save visualization PNG")
    ap.add_argument("--json-out", default="", help="Path to save detection JSON (list of fiducials)")
    ap.add_argument("--show", action="store_true", help="Show visualization window")
    ap.add_argument("--min-radius", type=int, default=0, help="Min circle radius (pixels)")
    ap.add_argument("--max-radius", type=int, default=0, help="Max circle radius (pixels)")
    ap.add_argument("--dp", type=float, default=1.2, help="HoughCircles dp")
    ap.add_argument("--min-dist", type=float, default=30, help="HoughCircles minDist")
    ap.add_argument("--param1", type=float, default=120, help="Canny high threshold for HoughCircles")
    ap.add_argument("--param2", type=float, default=30, help="Accumulator threshold for HoughCircles")
    ap.add_argument("--debug", action="store_true", help="Debug mode")
    args = ap.parse_args()

    img_u8 = load_to_u8_gray(args.image)

    fiducials = detect_crosshair_fiducials(
        img_u8,
        dp=args.dp,
        min_dist=args.min_dist,
        param1=args.param1,
        param2=args.param2,
        min_radius=args.min_radius,
        max_radius=args.max_radius,
        debug=args.debug
    )

    print(f"Detected {len(fiducials)} fiducial(s).")
    for i, f in enumerate(fiducials, 1):
        print(f"[{i}] center=(x={f.center[0]}, y={f.center[1]}), radius={f.radius}, score={f.score:.3f}")

    vis = draw_results(img_u8, fiducials)

    if args.save:
        cv2.imwrite(args.save, vis)
        print(f"Saved visualization to {args.save}")

    if args.json_out:
        data = [
            {"center_x": f.center[0], "center_y": f.center[1], "radius": f.radius, "score": round(f.score, 6)}
            for f in fiducials
        ]
        with open(args.json_out, "w", encoding="utf-8") as f:
            json.dump(data, f, indent=2)
        print(f"Saved JSON to {args.json_out}")

    if args.show:
        cv2.imshow("fiducials", vis)
        cv2.waitKey(0)
        cv2.destroyAllWindows()


if __name__ == "__main__":
    main()