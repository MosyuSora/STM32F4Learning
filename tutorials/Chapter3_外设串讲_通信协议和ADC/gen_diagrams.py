#!/usr/bin/env python3
"""
Timing diagram generator for STM32F4Learning – Chapter 3
Generates (overwrites):
  img/9.png   – SPI shift-register exchange concept
  img/12.png  – I²C protocol basics  (START / STOP / data bit / ACK / NACK)
  img/13.png  – I²C AT24C02 random-read transaction (with Repeated START)

Run from the chapter directory:
  python gen_diagrams.py
"""

import os
import sys

# Fix Windows GBK console encoding
if sys.stdout.encoding and sys.stdout.encoding.lower() not in ('utf-8', 'utf8'):
    try:
        sys.stdout.reconfigure(encoding='utf-8', errors='replace')
    except Exception:
        pass

try:
    import matplotlib
    matplotlib.use('Agg')
    import matplotlib.pyplot as plt
    import matplotlib.patches as mpatches
    from matplotlib.patches import FancyArrowPatch
    from matplotlib import font_manager
    import numpy as np
except ImportError as e:
    print(f"Missing dependency: {e}")
    print("Install with:  pip install matplotlib numpy")
    sys.exit(1)


def _setup_cjk_font():
    """Detect and activate a CJK-capable font so Chinese labels render correctly."""
    candidates = [
        'Microsoft YaHei', 'SimHei', 'SimSun', 'NSimSun',
        'WenQuanYi Micro Hei', 'Noto Sans CJK SC', 'Source Han Sans CN',
        'PingFang SC', 'Heiti SC',
    ]
    available = {f.name for f in font_manager.fontManager.ttflist}
    for name in candidates:
        if name in available:
            matplotlib.rcParams['font.family'] = name
            matplotlib.rcParams['axes.unicode_minus'] = False
            return name
    return None


_setup_cjk_font()

# ── output directory ───────────────────────────────────────────────────────────
IMG_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "img")
os.makedirs(IMG_DIR, exist_ok=True)

# ── colour palette ─────────────────────────────────────────────────────────────
SCL_C      = '#1e6fb5'
SDA_C      = '#d4760a'
MOSI_C     = '#2e8b57'
MISO_C     = '#d4760a'
NSS_C      = '#c0392b'
SAMPLE_BG  = '#cfe2ff'     # light-blue  – SCL-high sampling window
START_BG   = '#fde8e8'     # pink        – START condition
STOP_BG    = '#e8f5e9'     # light-green – STOP condition
ACK_BG     = '#d4edda'     # green       – ACK
NACK_BG    = '#fff3cd'     # yellow      – NACK
RS_BG      = '#ede0f5'     # purple      – Repeated START
WRITE_BG   = '#ddeeff'     # blue-tint   – write phase
READ_BG    = '#f3e5f5'     # purple-tint – read phase
LW         = 2.2


# ══════════════════════════════════════════════════════════════════════════════
# I²C WAVEFORM BUILDER
# ══════════════════════════════════════════════════════════════════════════════
class I2CBuilder:
    """
    Incrementally builds SCL and SDA sample arrays from high-level I²C events.
    Time unit U = one SCL half-period.
    """
    U = 1.0

    def __init__(self, t_idle=1.2):
        self.t = 0.0
        self._scl: list[tuple[float, float]] = []
        self._sda: list[tuple[float, float]] = []
        self.regions: list[dict] = []   # annotation info
        self._pt(self._scl, 0.0, 1.0)
        self._pt(self._sda, 0.0, 1.0)
        self.t = t_idle * self.U
        self._pt(self._scl, self.t, 1.0)
        self._pt(self._sda, self.t, 1.0)

    # ── internal ───────────────────────────────────────────────────────────────
    @staticmethod
    def _pt(lst, t, v): lst.append((t, v))

    def _scl_v(self): return self._scl[-1][1]
    def _sda_v(self): return self._sda[-1][1]

    def _scl_set(self, v):
        if self._scl_v() != v:
            self._pt(self._scl, self.t,                  self._scl_v())
            self._pt(self._scl, self.t + 0.04*self.U, float(v))

    def _sda_set(self, v, rc=False):
        if self._sda_v() == v:
            return
        self._pt(self._sda, self.t, self._sda_v())
        if rc and v == 1:
            # Simulate open-drain RC rise (exponential)
            n = 14
            for i in range(1, n + 1):
                frac = i / n
                vi = 1.0 - np.exp(-3.8 * frac)
                self._pt(self._sda, self.t + frac * 0.55 * self.U, vi)
        else:
            self._pt(self._sda, self.t + 0.035 * self.U, float(v))

    def _reg(self, t0, label, kind, extra=None):
        self.regions.append(dict(t0=t0, t1=self.t, label=label,
                                 kind=kind, extra=extra or {}))

    # ── public events ──────────────────────────────────────────────────────────
    def start(self, label='S'):
        t0 = self.t
        self._scl_set(1); self._sda_set(1)
        self.t += 0.25 * self.U
        fall_t = self.t
        self._sda_set(0)                    # SDA H→L while SCL=H  → START
        self.t += 0.55 * self.U
        self._scl_set(0)                    # SCL falls after SDA
        self.t += 0.2 * self.U
        self._reg(t0, label, 'start', {'fall_t': fall_t})

    def repeated_start(self, label='RS'):
        """Repeated START: SCL low → SCL high → SDA H→L → SCL low."""
        t0 = self.t
        # after last ACK Sda should already be H (released), SCL L
        self._sda_set(1, rc=True)
        self.t += 0.35 * self.U
        self._scl_set(1)                    # SCL rises
        self.t += 0.5 * self.U
        fall_t = self.t
        self._sda_set(0)                    # SDA H→L while SCL=H  → RS!
        self.t += 0.5 * self.U
        self._scl_set(0)                    # SCL falls
        self.t += 0.2 * self.U
        self._reg(t0, label, 'rs', {'fall_t': fall_t})

    def stop(self, label='P', idle=1.2):
        t0 = self.t
        self._scl_set(0); self._sda_set(0)
        self.t += 0.3 * self.U
        self._scl_set(1)                    # SCL rises
        self.t += 0.5 * self.U
        rise_t = self.t
        self._sda_set(1, rc=True)           # SDA L→H while SCL=H  → STOP
        self.t += 0.65 * self.U
        self._pt(self._scl, self.t, 1.0)
        self._pt(self._sda, self.t, 1.0)
        self.t += idle * self.U
        self._pt(self._scl, self.t, 1.0)
        self._pt(self._sda, self.t, 1.0)
        self._reg(t0, label, 'stop', {'rise_t': rise_t})

    def bit(self, v, label=None):
        t0 = self.t
        self._sda_set(v, rc=(v == 1))
        self.t += 0.85 * self.U
        self._scl_set(1)                    # SCL rises → sampling window
        sample_t = self.t + 0.5 * self.U
        self.t += self.U
        self._scl_set(0)
        self.t += 0.1 * self.U
        lbl = label if label is not None else str(v)
        self._reg(t0, lbl, 'bit', {'value': v, 'sample_t': sample_t})

    def ack(self, sender='slave', label='ACK'):
        t0 = self.t
        self._sda_set(0)                    # receiver pulls SDA low
        self.t += 0.85 * self.U
        self._scl_set(1)
        sample_t = self.t + 0.5 * self.U
        self.t += self.U
        self._scl_set(0)
        self.t += 0.1 * self.U
        self._sda_set(1, rc=True)           # SDA released after ACK
        self.t += 0.12 * self.U
        self._reg(t0, label, 'ack', {'sample_t': sample_t, 'sender': sender})

    def nack(self, sender='master', label='NACK'):
        t0 = self.t
        self._sda_set(1, rc=True)           # SDA stays high (not pulled)
        self.t += 0.85 * self.U
        self._scl_set(1)
        sample_t = self.t + 0.5 * self.U
        self.t += self.U
        self._scl_set(0)
        self.t += 0.1 * self.U
        self._reg(t0, label, 'nack', {'sample_t': sample_t})

    def byte(self, val, label=None):
        t0 = self.t
        bits = [(val >> (7 - i)) & 1 for i in range(8)]
        for b in bits:
            self.bit(b)
        self._reg(t0, label or f'0x{val:02X}', 'byte', {'value': val})

    def get(self):
        scl = np.array(self._scl)
        sda = np.array(self._sda)
        return scl[:, 0], scl[:, 1], sda[:, 0], sda[:, 1], self.t


# ══════════════════════════════════════════════════════════════════════════════
# SHARED PLOTTING HELPERS
# ══════════════════════════════════════════════════════════════════════════════
def _style_ax(ax, signal_label, color, T):
    ax.set_ylim(-0.28, 1.52)
    ax.set_yticks([0, 1])
    ax.set_yticklabels(['L', 'H'], fontsize=10, fontweight='bold')
    ax.set_xlim(0, T)
    ax.set_ylabel(signal_label, rotation=0, labelpad=30,
                  fontsize=12, fontweight='bold', color=color, va='center')
    for spine in ('top', 'right'):
        ax.spines[spine].set_visible(False)
    ax.tick_params(axis='x', which='both', bottom=False, labelbottom=False)
    ax.tick_params(axis='y', length=3)
    ax.yaxis.set_tick_params(labelcolor='#444')


def _draw_regions(ax_scl, ax_sda, regions, T, height=1.8, y0=-0.28):
    """Draw colored backgrounds and labels for each I²C region."""
    kind_color = {
        'start':  START_BG,
        'rs':     RS_BG,
        'stop':   STOP_BG,
        'ack':    ACK_BG,
        'nack':   NACK_BG,
        'bit':    None,          # individual bits: no background
        'byte':   None,          # byte groups handled separately
    }
    label_color = {
        'start': '#c0392b', 'rs': '#7b2d8b', 'stop': '#27ae60',
        'ack':   '#1a7a3c', 'nack': '#b7950b',
        'bit':   '#444444', 'byte': '#1a5276',
    }
    for reg in regions:
        kind = reg['kind']
        t0, t1 = reg['t0'], reg['t1']

        # Sampling window backgrounds inside each bit/ack/nack region
        if kind in ('bit', 'ack', 'nack') and 'sample_t' in reg['extra']:
            st = reg['extra']['sample_t']
            sw = 0.9  # sampling window width (in units)
            for ax in (ax_scl, ax_sda):
                ax.axvspan(st - sw * 0.5, st + sw * 0.5,
                           color=SAMPLE_BG, alpha=0.6, zorder=0)

        # Region background
        bg = kind_color.get(kind)
        if bg:
            for ax in (ax_scl, ax_sda):
                ax.axvspan(t0, t1, color=bg, alpha=0.45, zorder=0)

        # Region label (drawn on SDA axes, top area)
        lc = label_color.get(kind, '#333')
        fw = 'bold' if kind in ('start', 'rs', 'stop') else 'normal'
        fs = 9.5 if kind in ('byte',) else 9
        ax_sda.text((t0 + t1) * 0.5, 1.38, reg['label'],
                    ha='center', va='bottom', fontsize=fs,
                    color=lc, fontweight=fw, clip_on=True)

    # Bit-level labels on SCL axes (show bit value)
    for reg in regions:
        if reg['kind'] == 'bit':
            v = reg['extra']['value']
            t0, t1 = reg['t0'], reg['t1']
            ax_scl.text((t0 + t1) * 0.5, 1.38, str(v),
                        ha='center', va='bottom', fontsize=8.5,
                        color='#1a5276', fontweight='bold', clip_on=True)


def _annotate_start_stop(ax_scl, ax_sda, regions):
    """Add concise annotation arrows for START, RS, STOP conditions."""
    arrowprops = dict(arrowstyle='->', color='#555', lw=1.2)
    for reg in regions:
        kind = reg['kind']
        if kind in ('start', 'rs'):
            ft = reg['extra'].get('fall_t', reg['t0'])
            # Arrow pointing to the SDA falling edge
            ax_sda.annotate(
                'SDA↓\n(SCL=H)',
                xy=(ft + 0.1, 0.5), xytext=(ft - 0.6, 1.2),
                fontsize=7.5, color='#c0392b' if kind == 'start' else '#7b2d8b',
                arrowprops=dict(arrowstyle='->', color='#999', lw=1.0),
                ha='center',
            )
        elif kind == 'stop':
            rt = reg['extra'].get('rise_t', reg['t0'])
            ax_sda.annotate(
                'SDA↑\n(SCL=H)',
                xy=(rt + 0.1, 0.5), xytext=(rt + 0.9, 1.2),
                fontsize=7.5, color='#27ae60',
                arrowprops=dict(arrowstyle='->', color='#999', lw=1.0),
                ha='center',
            )


# ══════════════════════════════════════════════════════════════════════════════
# DIAGRAM 1 – SPI SHIFT REGISTER EXCHANGE  (img/9.png)
# ══════════════════════════════════════════════════════════════════════════════
def gen_spi_shift_register():
    """
    Conceptual diagram: master and slave each have an 8-bit shift register.
    One SCK pulse shifts one bit out (MOSI/MISO) and one bit in simultaneously.
    After 8 pulses the two registers have effectively swapped their contents.
    """
    fig, ax = plt.subplots(figsize=(12, 5.5), facecolor='white')
    ax.set_xlim(0, 12); ax.set_ylim(0, 5.5)
    ax.axis('off')
    fig.suptitle('SPI 全双工原理：主从移位寄存器字节交换',
                 fontsize=13, fontweight='bold', y=0.97)

    # ── colours / style ─────────────────────────────────────────────────────
    BOX_OUTER  = '#e8f0fe'
    BOX_INNER  = '#ffffff'
    BORDER     = '#4472C4'
    BIT_FILL_M = '#cfe2ff'
    BIT_FILL_S = '#fce8d5'
    TEXT_M     = '#1a4480'
    TEXT_S     = '#7b3800'
    ARROW_C    = '#333333'

    def draw_reg(ax, x, y, bits, fill, border, label, val_hex, sub):
        """Draw an 8-cell shift register at (x, y)."""
        bw, bh = 0.55, 0.55
        for i, b in enumerate(bits):
            rx = x + i * bw
            rect = mpatches.FancyBboxPatch(
                (rx, y), bw, bh,
                boxstyle='round,pad=0.02',
                facecolor=fill, edgecolor=border, linewidth=1.2)
            ax.add_patch(rect)
            ax.text(rx + bw/2, y + bh/2, str(b),
                    ha='center', va='center',
                    fontsize=10, fontweight='bold', color=border)
        # MSB / LSB labels
        total_w = 8 * bw
        ax.text(x,           y - 0.18, 'MSB', ha='center', va='top', fontsize=7.5, color='#777')
        ax.text(x + total_w, y - 0.18, 'LSB', ha='center', va='top', fontsize=7.5, color='#777')
        # Register label
        ax.text(x + total_w/2, y + bh + 0.15, label,
                ha='center', va='bottom', fontsize=9, color=border, fontweight='bold')
        # Hex value
        ax.text(x + total_w/2, y - 0.42, val_hex,
                ha='center', va='top', fontsize=10, color=border, fontweight='bold',
                bbox=dict(boxstyle='round,pad=0.3', facecolor='white', edgecolor=border, alpha=0.8))
        # Sub-label (e.g. "before" / "after")
        ax.text(x + total_w/2, y - 0.82, sub,
                ha='center', va='top', fontsize=8, color='#555')

    # Master bits: 0xAA = 10101010; Slave bits: 0x55 = 01010101
    master_bits = [1, 0, 1, 0, 1, 0, 1, 0]
    slave_bits  = [0, 1, 0, 1, 0, 1, 0, 1]

    # ── BEFORE box ───────────────────────────────────────────────────────────
    # Master register (left)
    mx, my = 0.5, 3.4
    draw_reg(ax, mx, my, master_bits, BIT_FILL_M, BORDER,
             'Master 移位寄存器', '0xAA = 10101010', '传输前')
    # Slave register (right)
    sx, sy = 6.6, 3.4
    draw_reg(ax, sx, sy, slave_bits, BIT_FILL_S, '#c0762a',
             'Slave 移位寄存器',  '0x55 = 01010101', '传输前')

    # ── AFTER box ────────────────────────────────────────────────────────────
    mx2, my2 = 0.5, 1.2
    draw_reg(ax, mx2, my2, slave_bits, BIT_FILL_M, BORDER,
             'Master 移位寄存器', '0x55 = 01010101', '8个SCK后')
    sx2, sy2 = 6.6, 1.2
    draw_reg(ax, sx2, sy2, master_bits, BIT_FILL_S, '#c0762a',
             'Slave 移位寄存器',  '0xAA = 10101010', '8个SCK后')

    # ── MOSI arrow (Master MSB → Slave) ──────────────────────────────────────
    # Top row: from Master[0] (left edge) to Slave[0]
    bw = 0.55
    m_msb_x = mx + bw/2
    s_msb_x = sx + bw/2
    ax.annotate('', xy=(s_msb_x, sy + 0.275),
                xytext=(m_msb_x + 8*bw, my + 0.275),
                arrowprops=dict(arrowstyle='->', color=MOSI_C, lw=2.0))
    ax.text((m_msb_x + 8*bw + s_msb_x)/2, my + 0.45,
            'MOSI  (Master→Slave)', ha='center', va='bottom',
            fontsize=9, color=MOSI_C, fontweight='bold')

    # ── MISO arrow (Slave MSB → Master) ──────────────────────────────────────
    ax.annotate('', xy=(m_msb_x + 8*bw, my + 0.275 - 0.55),
                xytext=(s_msb_x, sy + 0.275 - 0.55),
                arrowprops=dict(arrowstyle='->', color=MISO_C, lw=2.0))
    ax.text((m_msb_x + 8*bw + s_msb_x)/2, my - 0.5,
            'MISO  (Slave→Master)', ha='center', va='bottom',
            fontsize=9, color=MISO_C, fontweight='bold')

    # ── SCK label ─────────────────────────────────────────────────────────────
    ax.text(6.0, my - 1.15, '← 8 个 SCK 脉冲 →', ha='center', va='center',
            fontsize=9.5, color='#555',
            bbox=dict(boxstyle='round,pad=0.35', facecolor='#f8f9fa',
                      edgecolor='#bbb', alpha=0.9))

    # ── down arrow ────────────────────────────────────────────────────────────
    ax.annotate('', xy=(0.9, my2 + 0.75), xytext=(0.9, my - 0.1),
                arrowprops=dict(arrowstyle='->', color='#555', lw=1.5))
    ax.annotate('', xy=(11.1, sy2 + 0.75), xytext=(11.1, sy - 0.1),
                arrowprops=dict(arrowstyle='->', color='#555', lw=1.5))

    # ── key insight box ───────────────────────────────────────────────────────
    ax.text(6.0, 0.25,
            '关键：每个 SCK 周期，两个移位寄存器同时移出 MSB、同时移入对方的 MSB\n'
            '8 个时钟后，两寄存器完成"字节交换"——发 N 字节，必然同时收 N 字节',
            ha='center', va='center', fontsize=9, color='#1a3a5c',
            bbox=dict(boxstyle='round,pad=0.5', facecolor='#e8f4fd',
                      edgecolor='#4472C4', linewidth=1.5))

    plt.tight_layout(rect=[0, 0, 1, 0.96])
    out = os.path.join(IMG_DIR, '9.png')
    fig.savefig(out, dpi=160, bbox_inches='tight', facecolor='white')
    plt.close(fig)
    print(f"  [OK]  {out}")


# ══════════════════════════════════════════════════════════════════════════════
# DIAGRAM 2 – I²C BASICS  (img/12.png)
# ══════════════════════════════════════════════════════════════════════════════
def gen_i2c_basics():
    """
    Shows: IDLE | START | bit(1) | bit(0) | bit(1) | ACK | NACK | STOP | IDLE
    Highlights:
      - SCL=H while SDA falls (START)
      - SDA stable during SCL=H (sampling windows, shaded)
      - RC-rounded SDA rise edges (open-drain)
      - ACK pulled low by receiver, NACK left high
      - SCL=H while SDA rises (STOP)
    """
    b = I2CBuilder(t_idle=1.0)
    b.start()
    b.bit(1, label='1')
    b.bit(0, label='0')
    b.bit(1, label='1')
    b.ack(sender='slave')
    b.nack(sender='master')
    b.stop(idle=1.0)

    scl_t, scl_v, sda_t, sda_v, T = b.get()

    fig, axes = plt.subplots(
        2, 1, figsize=(15, 4.8), sharex=True,
        gridspec_kw={'height_ratios': [1, 1], 'hspace': 0.06})
    ax_scl, ax_sda = axes

    _draw_regions(ax_scl, ax_sda, b.regions, T)
    _annotate_start_stop(ax_scl, ax_sda, b.regions)

    ax_scl.plot(scl_t, scl_v, color=SCL_C, lw=LW, zorder=3)
    ax_sda.plot(sda_t, sda_v, color=SDA_C, lw=LW, zorder=3)

    _style_ax(ax_scl, 'SCL', SCL_C, T)
    _style_ax(ax_sda, 'SDA', SDA_C, T)

    # ── extra annotations ────────────────────────────────────────────────────
    # "SDA changes here (SCL=L)"
    bit1 = next(r for r in b.regions if r['kind'] == 'bit')
    t_chg = bit1['t0'] + 0.2
    ax_sda.annotate(
        'SDA 切换\n(SCL=L)',
        xy=(t_chg, 0.5), xytext=(t_chg + 0.3, -0.22),
        fontsize=7.5, color='#555', ha='center',
        arrowprops=dict(arrowstyle='->', color='#aaa', lw=0.9))

    # "Sampling window" bracket above SCL
    sample_regs = [r for r in b.regions if r['kind'] in ('bit', 'ack', 'nack')]
    if sample_regs:
        sr = sample_regs[0]
        st = sr['extra']['sample_t']
        sw = 0.9
        ax_scl.annotate('',
            xy=(st + sw*0.5, 1.32), xytext=(st - sw*0.5, 1.32),
            arrowprops=dict(arrowstyle='<->', color='#4472C4', lw=1.2))
        ax_scl.text(st, 1.44, 'SCL=H\n采样窗口',
                    ha='center', va='bottom', fontsize=7, color='#4472C4')

    # "RC 上升沿 (开漏)" label
    ack_reg = next((r for r in b.regions if r['kind'] == 'ack'), None)
    if ack_reg:
        ax_sda.text(ack_reg['t1'] + 0.1, 0.6,
                    'RC 上升沿\n(开漏)', fontsize=7, color='#888',
                    ha='left', va='center')

    # "ACK: 接收方拉低" and "NACK: 接收方释放"
    ack_reg = next((r for r in b.regions if r['kind'] == 'ack'), None)
    nack_reg = next((r for r in b.regions if r['kind'] == 'nack'), None)
    if ack_reg:
        ax_sda.text((ack_reg['t0']+ack_reg['t1'])/2, -0.22,
                    '接收方拉低', ha='center', va='top', fontsize=7.5,
                    color='#1a7a3c', fontweight='bold')
    if nack_reg:
        ax_sda.text((nack_reg['t0']+nack_reg['t1'])/2, -0.22,
                    '接收方释放', ha='center', va='top', fontsize=7.5,
                    color='#b7950b', fontweight='bold')

    fig.suptitle('I²C 协议基础时序：START / STOP / 数据位 / ACK / NACK',
                 fontsize=12.5, fontweight='bold', y=0.99)
    plt.tight_layout(rect=[0, 0, 1, 0.97])

    out = os.path.join(IMG_DIR, '12.png')
    fig.savefig(out, dpi=160, bbox_inches='tight', facecolor='white')
    plt.close(fig)
    print(f"  [OK]  {out}")


# ══════════════════════════════════════════════════════════════════════════════
# DIAGRAM 3 – I²C AT24C02 RANDOM-READ TRANSACTION  (img/13.png)
# ══════════════════════════════════════════════════════════════════════════════
def gen_i2c_transaction():
    """
    Single combined random-read transaction (no STOP in the middle):
      S | 0xA0+W | ACK | 0x05 | ACK | RS | 0xA1+R | ACK | 0xAB | NACK | P

    Key features:
      - Write phase (blue bg): S → 0xA0 → ACK → 0x05 → ACK
      - Repeated START (purple): clearly visible SCL=H while SDA falls
      - Read phase (purple bg):  0xA1 → ACK → 0xAB → NACK → P
      - Actual bit patterns shown (hover over bytes to confirm values)
    """
    b = I2CBuilder(t_idle=0.8)

    # ── WRITE PHASE ───────────────────────────────────────────────────────────
    # 0xA0 = 1010_0000 (7-bit addr 1010_000 + W=0)
    b.start(label='S')
    b.byte(0xA0, label='0xA0 (addr+W)')
    b.ack(sender='slave', label='ACK')
    # 0x05 = 0000_0101 (register address inside EEPROM)
    b.byte(0x05, label='0x05 (内部地址)')
    b.ack(sender='slave', label='ACK')

    # ── REPEATED START ────────────────────────────────────────────────────────
    b.repeated_start(label='RS')

    # ── READ PHASE ────────────────────────────────────────────────────────────
    # 0xA1 = 1010_0001 (7-bit addr 1010_000 + R=1)
    b.byte(0xA1, label='0xA1 (addr+R)')
    b.ack(sender='slave', label='ACK')
    # 0xAB = 1010_1011 (data byte driven by slave)
    b.byte(0xAB, label='0xAB (数据)')
    b.nack(sender='master', label='NACK')
    b.stop(label='P', idle=0.8)

    scl_t, scl_v, sda_t, sda_v, T = b.get()

    fig, axes = plt.subplots(
        2, 1, figsize=(22, 5.2), sharex=True,
        gridspec_kw={'height_ratios': [1, 1], 'hspace': 0.06})
    ax_scl, ax_sda = axes

    # ── phase background bands ────────────────────────────────────────────────
    # Find RS region to split write / read phases
    rs_reg = next((r for r in b.regions if r['kind'] == 'rs'), None)
    start_reg = next((r for r in b.regions if r['kind'] == 'start'), None)
    stop_reg  = next((r for r in b.regions if r['kind'] == 'stop'), None)
    if rs_reg and start_reg and stop_reg:
        for ax in (ax_scl, ax_sda):
            ax.axvspan(start_reg['t0'], rs_reg['t0'],
                       color=WRITE_BG, alpha=0.35, zorder=0)
            ax.axvspan(rs_reg['t1'], stop_reg['t1'],
                       color=READ_BG,  alpha=0.35, zorder=0)

    _draw_regions(ax_scl, ax_sda, b.regions, T)

    # ── phase labels at top of figure ─────────────────────────────────────────
    if rs_reg and start_reg and stop_reg:
        write_mid = (start_reg['t0'] + rs_reg['t0']) / 2
        read_mid  = (rs_reg['t1'] + stop_reg['t1']) / 2
        ax_scl.text(write_mid, 1.48,
                    '>> WRITE Phase  (Master -> AT24C02)',
                    ha='center', va='bottom', fontsize=9.5,
                    color='#1a4480', fontweight='bold',
                    bbox=dict(boxstyle='round,pad=0.25', facecolor='#d6e8ff',
                              edgecolor='#4472C4', alpha=0.9))
        ax_scl.text(read_mid, 1.48,
                    '>> READ Phase  (AT24C02 -> Master)',
                    ha='center', va='bottom', fontsize=9.5,
                    color='#5b2c8a', fontweight='bold',
                    bbox=dict(boxstyle='round,pad=0.25', facecolor='#ede0f5',
                              edgecolor='#9b59b6', alpha=0.9))

    # ── RS annotation ─────────────────────────────────────────────────────────
    if rs_reg:
        ft = rs_reg['extra'].get('fall_t', rs_reg['t0'])
        ax_sda.annotate(
            'Repeated START\nSCL=H, SDA↓\n(不发 STOP,\n保持总线所有权)',
            xy=(ft + 0.1, 0.45),
            xytext=(ft + 0.05, -0.22),
            fontsize=7.8, color='#7b2d8b', ha='center',
            bbox=dict(boxstyle='round,pad=0.3', facecolor='#f8eeff',
                      edgecolor='#9b59b6', alpha=0.95),
            arrowprops=dict(arrowstyle='->', color='#9b59b6', lw=1.3))

    # ── NACK annotation ───────────────────────────────────────────────────────
    nack_reg = next((r for r in b.regions if r['kind'] == 'nack'), None)
    if nack_reg:
        ax_sda.text((nack_reg['t0'] + nack_reg['t1'])/2, -0.24,
                    '主机发 NACK\n告知从机停止', ha='center', va='top',
                    fontsize=7.5, color='#b7950b', fontweight='bold')

    _annotate_start_stop(ax_scl, ax_sda, b.regions)

    ax_scl.plot(scl_t, scl_v, color=SCL_C, lw=LW, zorder=3)
    ax_sda.plot(sda_t, sda_v, color=SDA_C, lw=LW, zorder=3)

    _style_ax(ax_scl, 'SCL', SCL_C, T)
    _style_ax(ax_sda, 'SDA', SDA_C, T)

    fig.suptitle(
        'I²C AT24C02：随机读事务时序  (0xA0/0xA1, 内部地址=0x05)',
        fontsize=12.5, fontweight='bold', y=0.99)
    plt.tight_layout(rect=[0, 0, 1, 0.97])

    out = os.path.join(IMG_DIR, '13.png')
    fig.savefig(out, dpi=160, bbox_inches='tight', facecolor='white')
    plt.close(fig)
    print(f"  [OK]  {out}")


# ══════════════════════════════════════════════════════════════════════════════
# MAIN
# ══════════════════════════════════════════════════════════════════════════════
if __name__ == '__main__':
    print("Generating timing diagrams …")
    gen_spi_shift_register()   # img/9.png
    gen_i2c_basics()           # img/12.png
    gen_i2c_transaction()      # img/13.png
    print("Done.")
