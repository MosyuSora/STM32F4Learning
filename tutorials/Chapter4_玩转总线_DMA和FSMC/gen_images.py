"""
Chapter 4 全部插图生成脚本
运行: python gen_images.py
输出: img/ 目录下 12 张 PNG
"""
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
from matplotlib.patches import FancyBboxPatch, FancyArrowPatch, ArrowStyle
import numpy as np
import os

OUT = os.path.join(os.path.dirname(__file__), "img")
os.makedirs(OUT, exist_ok=True)

# ── 全局字体 ──────────────────────────────────────────────────
plt.rcParams['font.sans-serif'] = ['Microsoft YaHei', 'SimHei', 'DejaVu Sans']
plt.rcParams['axes.unicode_minus'] = False

# 颜色方案
C_BG      = '#FFFFFF'
C_BLUE    = '#4A90D9'
C_LBLUE   = '#D6E9F8'
C_GREEN   = '#5CB85C'
C_LGREEN  = '#DFF0D8'
C_RED     = '#D9534F'
C_LRED    = '#F2DEDE'
C_ORANGE  = '#F0AD4E'
C_LORANGE = '#FCF8E3'
C_GRAY    = '#777777'
C_LGRAY   = '#F5F5F5'
C_DGRAY   = '#333333'
C_PURPLE  = '#9B59B6'
C_LPURPLE = '#EBD6F5'

DPI = 150

def save(fig, name):
    fig.savefig(os.path.join(OUT, name), dpi=DPI, bbox_inches='tight',
                facecolor='white', edgecolor='none')
    plt.close(fig)
    print(f"  ✓ {name}")

# ══════════════════════════════════════════════════════════════
# §1.1  轮询/中断/DMA 三种方式对比表
# ══════════════════════════════════════════════════════════════
def gen_s11_comparison_table():
    fig, ax = plt.subplots(figsize=(10, 4))
    ax.axis('off')

    cols = ['维度', '轮询', '中断', 'DMA']
    rows = [
        ['CPU 占用率',  '100%\n(全程阻塞)',   '~30%\n(每字节进出 ISR)',  '≈0%\n(启动+完成各一次)'],
        ['中断次数',    '0',                    '1000\n(每字节一次)',       '1\n(传输完成)'],
        ['延迟',        '最低\n(直接执行)',      '中等\n(ISR 响应延迟)',    '稍高\n(配置开销)'],
        ['吞吐量',      '低',                   '中',                      '高'],
        ['适用场景',    '少量调试数据',          '少量不定长数据',          '大块批量传输'],
        ['代码复杂度',  '最低',                  '中等',                    '中等'],
    ]

    header_colors = [C_LGRAY, C_LRED, C_LORANGE, C_LGREEN]
    cell_colors = [[C_LGRAY, '#FFF5F5', '#FFFBF0', '#F0FFF0'] for _ in rows]

    table = ax.table(cellText=rows, colLabels=cols, cellLoc='center', loc='center',
                     cellColours=cell_colors, colColours=header_colors)
    table.auto_set_font_size(False)
    table.set_fontsize(10)
    table.scale(1, 1.8)

    for (r, c), cell in table.get_celld().items():
        cell.set_edgecolor('#CCCCCC')
        if r == 0:
            cell.set_text_props(weight='bold', fontsize=11)
        if c == 0:
            cell.set_text_props(weight='bold')

    ax.set_title('发送 1000 字节（115200 baud）三种方式对比', fontsize=13, weight='bold', pad=20)
    save(fig, 's11_comparison_table.png')

# ══════════════════════════════════════════════════════════════
# §1.2  DMA 搬运三要素示意图
# ══════════════════════════════════════════════════════════════
def gen_s12_three_elements():
    fig, ax = plt.subplots(figsize=(10, 5))
    ax.set_xlim(0, 10)
    ax.set_ylim(0, 6)
    ax.axis('off')

    # Source box
    src = FancyBboxPatch((0.5, 1.5), 2.5, 3, boxstyle="round,pad=0.15",
                         facecolor=C_LBLUE, edgecolor=C_BLUE, linewidth=2)
    ax.add_patch(src)
    ax.text(1.75, 3.5, '[源地址]', ha='center', va='center', fontsize=13, weight='bold')
    ax.text(1.75, 2.7, '外设 DR\n或 存储器', ha='center', va='center', fontsize=10, color=C_GRAY)
    ax.text(1.75, 1.9, '地址可固定/自增', ha='center', va='center', fontsize=8, color=C_GRAY, style='italic')

    # DMA controller box
    dma = FancyBboxPatch((3.8, 1.0), 2.8, 4, boxstyle="round,pad=0.15",
                         facecolor=C_LORANGE, edgecolor=C_ORANGE, linewidth=2)
    ax.add_patch(dma)
    ax.text(5.2, 4.2, 'DMA 控制器', ha='center', va='center', fontsize=13, weight='bold')
    ax.text(5.2, 3.4, '计数器 NDTR', ha='center', va='center', fontsize=11, color=C_DGRAY)
    ax.text(5.2, 2.6, '每搬一次 NDTR−−', ha='center', va='center', fontsize=9, color=C_GRAY)
    ax.text(5.2, 1.8, 'NDTR == 0 → TC中断', ha='center', va='center', fontsize=9, color=C_RED)

    # Destination box
    dst = FancyBboxPatch((7.4, 1.5), 2.5, 3, boxstyle="round,pad=0.15",
                         facecolor=C_LGREEN, edgecolor=C_GREEN, linewidth=2)
    ax.add_patch(dst)
    ax.text(8.65, 3.5, '[目标地址]', ha='center', va='center', fontsize=13, weight='bold')
    ax.text(8.65, 2.7, '存储器\n或 外设 DR', ha='center', va='center', fontsize=10, color=C_GRAY)
    ax.text(8.65, 1.9, '地址可固定/自增', ha='center', va='center', fontsize=8, color=C_GRAY, style='italic')

    # Arrows
    ax.annotate('', xy=(3.8, 3.2), xytext=(3.0, 3.2),
                arrowprops=dict(arrowstyle='->', color=C_BLUE, lw=2.5))
    ax.text(3.45, 3.6, '读数据', ha='center', fontsize=9, color=C_BLUE, weight='bold')

    ax.annotate('', xy=(7.4, 3.2), xytext=(6.6, 3.2),
                arrowprops=dict(arrowstyle='->', color=C_GREEN, lw=2.5))
    ax.text(7.05, 3.6, '写数据', ha='center', fontsize=9, color=C_GREEN, weight='bold')

    # CPU notification
    ax.annotate('', xy=(5.2, 5.3), xytext=(5.2, 5.0),
                arrowprops=dict(arrowstyle='->', color=C_RED, lw=1.5, linestyle='dashed'))
    cpu = FancyBboxPatch((4.2, 5.3), 2.0, 0.6, boxstyle="round,pad=0.1",
                         facecolor=C_LRED, edgecolor=C_RED, linewidth=1.5)
    ax.add_patch(cpu)
    ax.text(5.2, 5.6, 'CPU', ha='center', va='center', fontsize=11, weight='bold')

    ax.set_title('DMA 搬运三要素', fontsize=14, weight='bold', pad=15)
    save(fig, 's12_dma_three_elements.png')

# ══════════════════════════════════════════════════════════════
# §1.2  直接模式 vs FIFO 模式对比表
# ══════════════════════════════════════════════════════════════
def gen_s12_direct_vs_fifo():
    fig, ax = plt.subplots(figsize=(10, 3.5))
    ax.axis('off')

    cols = ['维度', '直接模式', 'FIFO 模式']
    rows = [
        ['缓冲',           '不缓冲，外设一请求就搬',       '攒到阈值 (1/4~满) 再搬'],
        ['源/目标宽度',     '必须一致',                     '可以不同 (FIFO 做拼接)'],
        ['突发传输',        '不支持',                       '支持 (4/8/16 节拍)'],
        ['存储器→存储器',   '不支持',                       '必须启用 FIFO'],
        ['典型场景',        'UART 逐字节收发',              'ADC 多通道、音频流、大块拷贝'],
    ]
    header_colors = [C_LGRAY, C_LBLUE, C_LORANGE]
    cell_colors   = [[C_LGRAY, '#F0F7FF', '#FFF8EC'] for _ in rows]

    table = ax.table(cellText=rows, colLabels=cols, cellLoc='center', loc='center',
                     cellColours=cell_colors, colColours=header_colors)
    table.auto_set_font_size(False)
    table.set_fontsize(10)
    table.scale(1, 1.7)
    for (r, c), cell in table.get_celld().items():
        cell.set_edgecolor('#CCCCCC')
        if r == 0: cell.set_text_props(weight='bold', fontsize=11)
        if c == 0: cell.set_text_props(weight='bold')

    ax.set_title('直接模式 vs FIFO 模式', fontsize=13, weight='bold', pad=20)
    save(fig, 's12_direct_vs_fifo.png')

# ══════════════════════════════════════════════════════════════
# §1.3  DMA 功能框图
# ══════════════════════════════════════════════════════════════
def gen_s13_dma_block_diagram():
    fig, ax = plt.subplots(figsize=(14, 8))
    ax.set_xlim(0, 14)
    ax.set_ylim(0, 9)
    ax.axis('off')

    def box(x, y, w, h, text, fc, ec, fontsize=10, bold=False):
        r = FancyBboxPatch((x, y), w, h, boxstyle="round,pad=0.1", facecolor=fc, edgecolor=ec, lw=1.5)
        ax.add_patch(r)
        ax.text(x + w/2, y + h/2, text, ha='center', va='center', fontsize=fontsize,
                weight='bold' if bold else 'normal', wrap=True)
        return (x, y, w, h)

    # Title
    ax.set_title('STM32F407 DMA 控制器功能框图', fontsize=15, weight='bold', pad=15)

    # ─── AHB 总线矩阵 ───
    box(0.3, 7.5, 13.4, 0.8, 'AHB 总线矩阵（CPU / DMA1 / DMA2 共享）', C_LGRAY, C_GRAY, 12, True)

    # ─── DMA1 ───
    box(0.5, 0.5, 6, 6.5, '', '#F0F7FF', C_BLUE, 10)
    ax.text(3.5, 6.6, 'DMA1（外设端口 → APB1）', ha='center', fontsize=12, weight='bold', color=C_BLUE)

    # Streams for DMA1
    for i in range(8):
        y = 5.8 - i * 0.7
        # Stream box
        sx = 1.0
        r = FancyBboxPatch((sx, y), 2.2, 0.5, boxstyle="round,pad=0.05",
                           facecolor=C_LBLUE, edgecolor=C_BLUE, lw=1)
        ax.add_patch(r)
        ax.text(sx + 1.1, y + 0.25, f'Stream {i}', ha='center', va='center', fontsize=8)

        # Channel mux
        mx = 3.6
        r2 = FancyBboxPatch((mx, y), 1.2, 0.5, boxstyle="round,pad=0.05",
                            facecolor='#E8E8E8', edgecolor=C_GRAY, lw=1)
        ax.add_patch(r2)
        ax.text(mx + 0.6, y + 0.25, 'Ch0~7', ha='center', va='center', fontsize=7, color=C_GRAY)

        # FIFO
        fx = 5.1
        r3 = FancyBboxPatch((fx, y), 1.0, 0.5, boxstyle="round,pad=0.05",
                            facecolor=C_LORANGE, edgecolor=C_ORANGE, lw=1)
        ax.add_patch(r3)
        ax.text(fx + 0.5, y + 0.25, 'FIFO', ha='center', va='center', fontsize=7)

        # Connection lines
        ax.plot([sx + 2.2, mx], [y + 0.25, y + 0.25], color=C_GRAY, lw=0.8)
        ax.plot([mx + 1.2, fx], [y + 0.25, y + 0.25], color=C_GRAY, lw=0.8)

    # Arbiter DMA1
    arb1 = FancyBboxPatch((0.6, 0.6), 0.3, 5.5, boxstyle="round,pad=0.05",
                          facecolor=C_LPURPLE, edgecolor=C_PURPLE, lw=1)
    ax.add_patch(arb1)
    ax.text(0.75, 3.35, '仲\n裁\n器', ha='center', va='center', fontsize=8, color=C_PURPLE, weight='bold')

    # ─── DMA2 ───
    box(7.3, 0.5, 6, 6.5, '', '#F0FFF0', C_GREEN, 10)
    ax.text(10.3, 6.6, 'DMA2（双端口 → AHB 矩阵）', ha='center', fontsize=12, weight='bold', color=C_GREEN)

    for i in range(8):
        y = 5.8 - i * 0.7
        sx = 7.8
        r = FancyBboxPatch((sx, y), 2.2, 0.5, boxstyle="round,pad=0.05",
                           facecolor=C_LGREEN, edgecolor=C_GREEN, lw=1)
        ax.add_patch(r)
        ax.text(sx + 1.1, y + 0.25, f'Stream {i}', ha='center', va='center', fontsize=8)

        mx = 10.4
        r2 = FancyBboxPatch((mx, y), 1.2, 0.5, boxstyle="round,pad=0.05",
                            facecolor='#E8E8E8', edgecolor=C_GRAY, lw=1)
        ax.add_patch(r2)
        ax.text(mx + 0.6, y + 0.25, 'Ch0~7', ha='center', va='center', fontsize=7, color=C_GRAY)

        fx = 11.9
        r3 = FancyBboxPatch((fx, y), 1.0, 0.5, boxstyle="round,pad=0.05",
                            facecolor=C_LORANGE, edgecolor=C_ORANGE, lw=1)
        ax.add_patch(r3)
        ax.text(fx + 0.5, y + 0.25, 'FIFO', ha='center', va='center', fontsize=7)

        ax.plot([sx + 2.2, mx], [y + 0.25, y + 0.25], color=C_GRAY, lw=0.8)
        ax.plot([mx + 1.2, fx], [y + 0.25, y + 0.25], color=C_GRAY, lw=0.8)

    # Arbiter DMA2
    arb2 = FancyBboxPatch((7.4, 0.6), 0.3, 5.5, boxstyle="round,pad=0.05",
                          facecolor=C_LPURPLE, edgecolor=C_PURPLE, lw=1)
    ax.add_patch(arb2)
    ax.text(7.55, 3.35, '仲\n裁\n器', ha='center', va='center', fontsize=8, color=C_PURPLE, weight='bold')

    # Connection lines to bus matrix
    ax.annotate('', xy=(3.5, 7.5), xytext=(3.5, 7.1),
                arrowprops=dict(arrowstyle='->', color=C_BLUE, lw=2))
    ax.text(3.5, 7.3, '外设端口\n(APB1 only)', ha='center', fontsize=7, color=C_BLUE)

    ax.annotate('', xy=(9.3, 7.5), xytext=(9.3, 7.1),
                arrowprops=dict(arrowstyle='->', color=C_GREEN, lw=2))
    ax.annotate('', xy=(11.3, 7.5), xytext=(11.3, 7.1),
                arrowprops=dict(arrowstyle='->', color=C_GREEN, lw=2))
    ax.text(10.3, 7.2, '外设端口        存储器端口', ha='center', fontsize=7, color=C_GREEN)

    # Legend note
    ax.text(7, 0.15, '注意: DMA1 外设端口只连 APB1（不能做 M2M）    DMA2 双端口都经 AHB 矩阵（能做 M2M）',
            ha='center', fontsize=9, color=C_RED, style='italic')

    save(fig, 's13_dma_block_diagram.png')

# ══════════════════════════════════════════════════════════════
# §1.3  DMA 请求映射表
# ══════════════════════════════════════════════════════════════
def gen_s13_dma_request_map():
    fig, axes = plt.subplots(2, 1, figsize=(14, 10))

    # ── DMA2 (more relevant - has USART1) ──
    dma2_data = [
        ['Stream0', 'ADC1',      '—',     'TIM8_CH1/2/3', '—',          'ADC1',         '—',         'TIM1_TRIG',  '—'],
        ['Stream1', '—',         'DCMI',  'ADC2',         '—',          '—',            'USART6_RX', 'TIM1_CH1',   '—'],
        ['Stream2', 'TIM8_CH1',  '—',     'USART1_RX',    '—',          '—',            'USART6_RX', 'TIM1_CH2',   '—'],
        ['Stream3', '—',         '—',     'SPI1_RX',      '—',          '—',            '—',         'TIM1_CH1',   '—'],
        ['Stream4', 'ADC1',      '—',     'SPI4_TX',      '—',          '—',            'USART1_TX', '—',          '—'],
        ['Stream5', '—',         '—',     '—',            'SPI1_TX',    'USART1_RX',    '—',         'TIM1_UP',    'SPI5_TX'],
        ['Stream6', 'TIM1_CH1/2/3','—',   '—',            'USART1_TX',  '—',            '—',         'TIM1_CH3',   'USART6_TX'],
        ['Stream7', '—',         '—',     'TIM8_UP',      '—',          'USART1_TX',    '—',         '—',          '—'],
    ]
    cols2 = ['数据流', 'Ch0', 'Ch1', 'Ch2', 'Ch3', 'Ch4', 'Ch5', 'Ch6', 'Ch7']

    for idx, (ax, title, data) in enumerate([
        (axes[0], 'DMA2 请求映射表（精简版）', dma2_data),
    ]):
        ax.axis('off')
        cell_colors = []
        for row in data:
            row_colors = ['#F5F5F5']
            for cell in row[1:]:
                if 'USART1' in cell:
                    row_colors.append(C_LGREEN)
                elif cell == '—':
                    row_colors.append('#FAFAFA')
                else:
                    row_colors.append('#FFFFFF')
            cell_colors.append(row_colors)

        hdr_colors = [C_LGRAY] + [C_LBLUE] * 8
        table = ax.table(cellText=data, colLabels=cols2, cellLoc='center', loc='center',
                         cellColours=cell_colors, colColours=hdr_colors)
        table.auto_set_font_size(False)
        table.set_fontsize(8)
        table.scale(1, 1.5)
        for (r, c), cell in table.get_celld().items():
            cell.set_edgecolor('#DDDDDD')
            if r == 0: cell.set_text_props(weight='bold', fontsize=9)
            if c == 0 and r > 0: cell.set_text_props(weight='bold', fontsize=8)
        ax.set_title(title, fontsize=12, weight='bold', pad=10)

    # DMA1
    dma1_data = [
        ['Stream0', 'SPI3_RX',  '—',       'SPI3_RX',     'SPI2_RX',  '—',         '—',         '—',        'I2C1_RX'],
        ['Stream1', '—',        'I2C3_RX', 'TIM7_UP',     '—',        'TIM2_UP/CH3','—',        '—',        '—'],
        ['Stream2', 'SPI3_RX',  '—',       'I2S3ext_RX',  '—',        'I2C3_RX',   'TIM3_CH4/UP','—',       'I2C2_RX'],
        ['Stream3', 'SPI2_RX',  '—',       'SPI2_RX',     '—',        'I2S2ext_RX','—',         'TIM4_CH2', 'I2C2_RX'],
        ['Stream4', 'SPI2_TX',  '—',       'I2S2ext_TX',  '—',        'I2C3_TX',   'TIM3_CH1',  '—',        '—'],
        ['Stream5', 'SPI3_TX',  'I2C1_RX', 'I2S3ext_TX',  '—',        'USART2_RX', 'TIM3_CH2',  '—',        'DAC1'],
        ['Stream6', '—',        'I2C1_TX', 'TIM4_UP',     '—',        'USART2_TX', 'TIM3_TRIG', '—',        'DAC2'],
        ['Stream7', 'SPI3_TX',  '—',       'I2S3ext_TX',  'I2C1_TX',  'UART5_TX',  '—',         'TIM4_CH3', '—'],
    ]
    cols1 = ['数据流', 'Ch0', 'Ch1', 'Ch2', 'Ch3', 'Ch4', 'Ch5', 'Ch6', 'Ch7']

    ax1 = axes[1]
    ax1.axis('off')
    cell_colors1 = []
    for row in dma1_data:
        row_colors = ['#F5F5F5']
        for cell in row[1:]:
            if 'USART' in cell or 'UART' in cell:
                row_colors.append(C_LGREEN)
            elif cell == '—':
                row_colors.append('#FAFAFA')
            else:
                row_colors.append('#FFFFFF')
        cell_colors1.append(row_colors)

    hdr_colors1 = [C_LGRAY] + [C_LORANGE] * 8
    table1 = ax1.table(cellText=dma1_data, colLabels=cols1, cellLoc='center', loc='center',
                       cellColours=cell_colors1, colColours=hdr_colors1)
    table1.auto_set_font_size(False)
    table1.set_fontsize(8)
    table1.scale(1, 1.5)
    for (r, c), cell in table1.get_celld().items():
        cell.set_edgecolor('#DDDDDD')
        if r == 0: cell.set_text_props(weight='bold', fontsize=9)
        if c == 0 and r > 0: cell.set_text_props(weight='bold', fontsize=8)
    ax1.set_title('DMA1 请求映射表（精简版）', fontsize=12, weight='bold', pad=10)

    fig.text(0.5, 0.01, '* 绿色高亮 = 本教程会用到的外设      — = 该位置无映射',
             ha='center', fontsize=10, color=C_GRAY)
    fig.tight_layout(rect=[0, 0.03, 1, 1])
    save(fig, 's13_dma_request_map.png')

# ══════════════════════════════════════════════════════════════
# §1.3  DMA 传输流程图
# ══════════════════════════════════════════════════════════════
def gen_s13_dma_transfer_flow():
    fig, ax = plt.subplots(figsize=(10, 12))
    ax.set_xlim(0, 10)
    ax.set_ylim(0, 14)
    ax.axis('off')

    def flowbox(x, y, w, h, text, fc, ec, fontsize=10):
        r = FancyBboxPatch((x - w/2, y - h/2), w, h, boxstyle="round,pad=0.1",
                           facecolor=fc, edgecolor=ec, lw=2)
        ax.add_patch(r)
        ax.text(x, y, text, ha='center', va='center', fontsize=fontsize, wrap=True)

    def diamond(x, y, text, fc, ec, fontsize=9):
        d = 0.8
        verts = [(x, y+d), (x+d*1.5, y), (x, y-d), (x-d*1.5, y), (x, y+d)]
        from matplotlib.patches import Polygon
        poly = Polygon(verts, facecolor=fc, edgecolor=ec, lw=2)
        ax.add_patch(poly)
        ax.text(x, y, text, ha='center', va='center', fontsize=fontsize)

    def arrow(x1, y1, x2, y2, label='', side='right', color='#333'):
        ax.annotate('', xy=(x2, y2), xytext=(x1, y1),
                    arrowprops=dict(arrowstyle='->', color=color, lw=1.8))
        if label:
            mx = (x1 + x2) / 2
            my = (y1 + y2) / 2
            offset = (0.15, 0) if side == 'right' else (-0.15, 0)
            ax.text(mx + offset[0], my + offset[1], label, fontsize=8, color=color,
                    ha='left' if side == 'right' else 'right', va='center')

    ax.set_title('DMA 传输完整流程', fontsize=14, weight='bold', pad=15)

    # 1. Configure
    flowbox(5, 13, 4, 0.8, '配置数据流\n(源/目标/计数/方向/优先级)', C_LBLUE, C_BLUE)
    arrow(5, 12.6, 5, 12.1)

    # 2. Enable
    flowbox(5, 11.7, 3, 0.6, '使能 EN = 1', C_LBLUE, C_BLUE)
    arrow(5, 11.4, 5, 10.8)

    # 3. Direction?
    diamond(5, 10.4, '传输方向?', C_LORANGE, C_ORANGE)
    arrow(3.5, 10.4, 2.2, 10.4, '外设<->存储器', 'left', C_ORANGE)
    arrow(6.5, 10.4, 7.8, 10.4, 'M→M', 'right', C_ORANGE)

    # 4a. Wait for peripheral request
    flowbox(1.5, 9.2, 2.5, 0.7, '等待外设\nDMA 请求', C_LORANGE, C_ORANGE, 9)
    arrow(1.5, 10.0, 1.5, 9.55)

    # 4b. Start immediately
    flowbox(8.5, 9.2, 2, 0.7, '立即开始', C_LGREEN, C_GREEN, 9)
    arrow(8.5, 10.0, 8.5, 9.55)

    # Converge
    arrow(1.5, 8.85, 5, 8.2)
    arrow(8.5, 8.85, 5, 8.2)

    # 5. Arbitration
    flowbox(5, 7.8, 2.5, 0.6, '仲裁获胜', C_LPURPLE, C_PURPLE)
    arrow(5, 7.5, 5, 7.0)

    # 6. FIFO?
    diamond(5, 6.6, 'FIFO 模式?', C_LORANGE, C_ORANGE)
    arrow(3.5, 6.6, 2, 6.6, '直接模式', 'left', C_ORANGE)
    arrow(6.5, 6.6, 8, 6.6, 'FIFO', 'right', C_ORANGE)

    # 7a. Direct: move 1
    flowbox(1.5, 5.5, 2.2, 0.7, '搬 1 个数据\nNDTR−−', C_LBLUE, C_BLUE, 9)
    arrow(1.5, 6.2, 1.5, 5.85)

    # 7b. FIFO: burst
    flowbox(8.5, 5.5, 2.5, 0.7, '攒到阈值后\n突发搬运', C_LORANGE, C_ORANGE, 9)
    arrow(8.5, 6.2, 8.5, 5.85)

    # Converge
    arrow(1.5, 5.15, 5, 4.5)
    arrow(8.5, 5.15, 5, 4.5)

    # 8. NDTR == 0?
    diamond(5, 4.1, 'NDTR==0?', C_LRED, C_RED)

    # No → back to wait
    ax.annotate('', xy=(0.8, 9.2), xytext=(0.8, 4.1),
                arrowprops=dict(arrowstyle='->', color=C_GRAY, lw=1.5,
                                connectionstyle='arc3,rad=0'))
    ax.text(0.4, 6.6, '否', fontsize=9, color=C_GRAY, weight='bold')

    # Yes → TC
    arrow(5, 3.3, 5, 2.8, '是', 'right', C_RED)

    # 9. TCIF
    flowbox(5, 2.4, 3.5, 0.6, 'TCIF = 1，触发完成中断 (可选)', C_LRED, C_RED, 9)
    arrow(5, 2.1, 5, 1.6)

    # 10. Circular?
    diamond(5, 1.2, '循环模式?', C_LGREEN, C_GREEN)

    # Yes → reload
    ax.annotate('', xy=(9.5, 9.2), xytext=(9.5, 1.2),
                arrowprops=dict(arrowstyle='->', color=C_GREEN, lw=1.5,
                                connectionstyle='arc3,rad=0'))
    ax.text(9.7, 5.2, '是：重装 NDTR', fontsize=8, color=C_GREEN, weight='bold', rotation=90)

    # No → stop
    arrow(3.5, 1.2, 2, 1.2, '', 'left', C_RED)
    flowbox(1.3, 1.2, 2, 0.5, '停止，EN 清零', C_LRED, C_RED, 9)

    save(fig, 's13_dma_transfer_flow.png')

# ══════════════════════════════════════════════════════════════
# §1.4  轮询 vs 中断 vs DMA 时序对比
# ══════════════════════════════════════════════════════════════
def gen_s14_timing_comparison():
    fig, axes = plt.subplots(3, 1, figsize=(12, 8), sharex=True)
    fig.suptitle('发送 1000 字节：轮询 vs 中断 vs DMA 时序对比', fontsize=14, weight='bold')

    t = np.linspace(0, 87, 1000)  # 87ms total

    # ── Polling ──
    ax = axes[0]
    ax.set_ylabel('CPU 状态', fontsize=10)
    ax.set_title('方式一：轮询', fontsize=11, color=C_RED, weight='bold', loc='left')
    ax.fill_between(t, 0, 1, color=C_LRED, alpha=0.8)
    ax.text(43, 0.5, 'CPU 全程在 while(TXE) 循环等待\n占用率 = 100%', ha='center', va='center',
            fontsize=10, color=C_RED, weight='bold')
    ax.set_ylim(-0.1, 1.3)
    ax.set_yticks([])
    ax.axhline(y=0, color='black', lw=0.5)

    # ── Interrupt ──
    ax = axes[1]
    ax.set_ylabel('CPU 状态', fontsize=10)
    ax.set_title('方式二：中断', fontsize=11, color=C_ORANGE, weight='bold', loc='left')
    # Main loop (green)
    ax.fill_between(t, 0, 0.5, color=C_LGREEN, alpha=0.6)
    # ISR spikes
    isr_times = np.linspace(0, 87, 50)  # show ~50 ISR spikes
    for it in isr_times:
        ax.fill_between([it, it+0.5], 0.5, 1.0, color=C_LORANGE, alpha=0.8)
    ax.text(43, 0.22, '主循环运行', ha='center', va='center', fontsize=9, color=C_GREEN)
    ax.text(43, 0.75, '↑ 每字节触发一次 ISR（共 1000 次）', ha='center', va='center',
            fontsize=9, color=C_ORANGE, weight='bold')
    ax.set_ylim(-0.1, 1.3)
    ax.set_yticks([])
    ax.axhline(y=0, color='black', lw=0.5)
    ax.axhline(y=0.5, color=C_ORANGE, lw=0.5, linestyle='--')

    # ── DMA ──
    ax = axes[2]
    ax.set_ylabel('CPU 状态', fontsize=10)
    ax.set_title('方式三：DMA', fontsize=11, color=C_GREEN, weight='bold', loc='left')
    ax.fill_between(t, 0, 0.5, color=C_LGREEN, alpha=0.6)
    # Only 1 interrupt at the very start (config) and end (TC)
    ax.fill_between([0, 1], 0.5, 1.0, color=C_LBLUE, alpha=0.8)
    ax.fill_between([86, 87], 0.5, 1.0, color=C_LRED, alpha=0.8)
    ax.text(1, 0.75, '配置', fontsize=7, ha='center', color=C_BLUE)
    ax.text(86.5, 0.75, 'TC', fontsize=7, ha='center', color=C_RED)
    ax.text(43, 0.22, 'CPU 自由！跑算法 / 状态机 / UI 刷新...', ha='center', va='center',
            fontsize=10, color=C_GREEN, weight='bold')
    ax.text(43, 0.9, '(DMA 在后台自动搬完 1000 字节)', ha='center', va='center',
            fontsize=9, color=C_GRAY, style='italic')
    ax.set_ylim(-0.1, 1.3)
    ax.set_yticks([])
    ax.set_xlabel('时间 (ms)  —  115200 baud, 1000 字节 ≈ 87ms', fontsize=10)
    ax.axhline(y=0, color='black', lw=0.5)
    ax.axhline(y=0.5, color=C_GREEN, lw=0.5, linestyle='--')

    fig.tight_layout(rect=[0, 0, 1, 0.95])
    save(fig, 's14_timing_comparison.png')

# ══════════════════════════════════════════════════════════════
# §2.1  STM32F407 存储资源一览
# ══════════════════════════════════════════════════════════════
def gen_s21_storage_overview():
    fig, ax = plt.subplots(figsize=(10, 4))
    ax.axis('off')

    cols = ['存储类型', '大小', '特点', '访问速度']
    rows = [
        ['主 SRAM',              '112 KB',  'CPU/DMA 均可访问',            '0 等待 (≤168MHz)'],
        ['CCM SRAM',             '64 KB',   '仅 CPU 可访问（DMA 不能用！）', '0 等待'],
        ['备份 SRAM',            '4 KB',    '电池供电保持',                '低功耗域'],
        ['内部 Flash',           '1 MB',    '程序存储，写前要擦除',         '带 ART 加速'],
        ['外部 SRAM\n(FSMC)',    '1 MB',    '随机读写，指针直接访问',       '~55ns / 次'],
    ]
    header_colors = [C_LGRAY, C_LGRAY, C_LGRAY, C_LGRAY]
    cell_colors = [
        [C_LGREEN, C_LGREEN, C_LGREEN, C_LGREEN],
        [C_LORANGE, C_LORANGE, C_LORANGE, C_LORANGE],
        ['#F5F5F5', '#F5F5F5', '#F5F5F5', '#F5F5F5'],
        ['#F5F5F5', '#F5F5F5', '#F5F5F5', '#F5F5F5'],
        [C_LBLUE, C_LBLUE, C_LBLUE, C_LBLUE],
    ]
    table = ax.table(cellText=rows, colLabels=cols, cellLoc='center', loc='center',
                     cellColours=cell_colors, colColours=header_colors)
    table.auto_set_font_size(False)
    table.set_fontsize(10)
    table.scale(1, 1.8)
    for (r, c), cell in table.get_celld().items():
        cell.set_edgecolor('#CCCCCC')
        if r == 0: cell.set_text_props(weight='bold', fontsize=11)
        if c == 0: cell.set_text_props(weight='bold')

    ax.set_title('STM32F407 存储资源一览', fontsize=13, weight='bold', pad=20)
    ax.text(0.5, -0.05, '* CCM SRAM 不能被 DMA 访问！分配缓冲区时要注意。',
            transform=ax.transAxes, ha='center', fontsize=9, color=C_RED, style='italic')
    save(fig, 's21_storage_overview.png')

# ══════════════════════════════════════════════════════════════
# §2.2  SRAM 读写时序图
# ══════════════════════════════════════════════════════════════
def gen_s22_sram_timing():
    fig, axes = plt.subplots(2, 1, figsize=(12, 8))

    def draw_timing(ax, title, signals, annotations, color):
        ax.set_title(title, fontsize=13, weight='bold', color=color, loc='left')
        ax.set_xlim(0, 100)
        ax.set_ylim(-0.5, len(signals) * 2 + 0.5)
        ax.set_yticks([])
        ax.set_xticks([])
        ax.spines['top'].set_visible(False)
        ax.spines['right'].set_visible(False)
        ax.spines['bottom'].set_visible(False)
        ax.spines['left'].set_visible(False)

        for i, (name, waveform) in enumerate(signals):
            y_base = (len(signals) - 1 - i) * 2
            ax.text(-1, y_base + 0.5, name, ha='right', va='center', fontsize=10, weight='bold')
            # Draw waveform
            for j in range(len(waveform) - 1):
                x1, v1 = waveform[j]
                x2, v2 = waveform[j+1]
                y1 = y_base + v1
                y2 = y_base + v2
                # Horizontal line to transition point
                ax.plot([x1, x2], [y1, y1], color='black', lw=1.5)
                # Vertical transition
                if v1 != v2:
                    ax.plot([x2, x2], [y1, y2], color='black', lw=1.5)

            # Last segment
            lx, lv = waveform[-1]
            ax.plot([lx, 100], [y_base + lv, y_base + lv], color='black', lw=1.5)

        # Annotations
        for ann in annotations:
            ax.annotate('', xy=(ann['x2'], ann['y']), xytext=(ann['x1'], ann['y']),
                        arrowprops=dict(arrowstyle='<->', color=ann.get('color', C_RED), lw=1.5))
            ax.text((ann['x1'] + ann['x2'])/2, ann['y'] + 0.3, ann['text'],
                    ha='center', fontsize=9, color=ann.get('color', C_RED), weight='bold')

    # ── Read Timing ──
    read_signals = [
        ('地址 A',  [(0, 0), (10, 1), (85, 1), (85, 0)]),  # valid from 10-85
        ('CS#',     [(0, 1), (15, 0), (90, 0), (90, 1)]),
        ('OE#',     [(0, 1), (20, 0), (85, 0), (85, 1)]),
        ('WE#',     [(0, 1)]),  # stays high
        ('数据 D',  [(0, 0), (60, 0), (60, 1), (85, 1), (85, 0)]),  # valid from 60-85
    ]
    read_annotations = [
        {'x1': 10, 'x2': 85, 'y': 9.5, 'text': 'tRC ≥ 55ns', 'color': C_BLUE},
        {'x1': 10, 'x2': 60, 'y': -0.3, 'text': 'tAA ≤ 55ns', 'color': C_RED},
        {'x1': 20, 'x2': 60, 'y': 4.3, 'text': 'tDOE ≤ 25ns', 'color': C_ORANGE},
    ]

    # Add hatching for valid data region
    draw_timing(axes[0], 'SRAM 读时序 (Read Cycle)', read_signals, read_annotations, C_BLUE)
    axes[0].fill_between([10, 85], 8, 9, color=C_LGREEN, alpha=0.3)  # valid address
    axes[0].text(47, 8.5, '有效地址', ha='center', fontsize=8, color=C_GREEN)
    axes[0].fill_between([60, 85], 0, 1, color=C_LGREEN, alpha=0.3)  # valid data
    axes[0].text(72, 0.5, '数据有效', ha='center', fontsize=8, color=C_GREEN)

    # ── Write Timing ──
    write_signals = [
        ('地址 A',  [(0, 0), (10, 1), (85, 1), (85, 0)]),
        ('CS#',     [(0, 1), (15, 0), (85, 0), (85, 1)]),
        ('OE#',     [(0, 1)]),  # stays high
        ('WE#',     [(0, 1), (20, 0), (75, 0), (75, 1)]),
        ('数据 D',  [(0, 0), (15, 1), (80, 1), (80, 0)]),  # MCU drives data
    ]
    write_annotations = [
        {'x1': 10, 'x2': 85, 'y': 9.5, 'text': 'tWC ≥ 55ns', 'color': C_BLUE},
        {'x1': 20, 'x2': 75, 'y': 2.3, 'text': 'tPWE ≥ 40ns', 'color': C_RED},
    ]

    draw_timing(axes[1], 'SRAM 写时序 (Write Cycle)', write_signals, write_annotations, C_GREEN)
    axes[1].fill_between([10, 85], 8, 9, color=C_LGREEN, alpha=0.3)
    axes[1].text(47, 8.5, '有效地址', ha='center', fontsize=8, color=C_GREEN)
    axes[1].fill_between([15, 80], 0, 1, color=C_LORANGE, alpha=0.3)
    axes[1].text(47, 0.5, 'MCU 驱动数据', ha='center', fontsize=8, color=C_ORANGE)
    # WE# rising edge marker
    axes[1].annotate('WE# 上升沿\n= 采样沿', xy=(75, 3), xytext=(82, 4),
                     fontsize=8, color=C_RED, weight='bold',
                     arrowprops=dict(arrowstyle='->', color=C_RED, lw=1.5))

    fig.tight_layout()
    save(fig, 's22_sram_timing.png')

# ══════════════════════════════════════════════════════════════
# §2.3  FSMC 地址映射图
# ══════════════════════════════════════════════════════════════
def gen_s23_fsmc_address_map():
    fig, ax = plt.subplots(figsize=(12, 8))
    ax.set_xlim(0, 12)
    ax.set_ylim(0, 10)
    ax.axis('off')
    ax.set_title('FSMC 地址映射 — 从 Cortex-M4 地址空间到外部 SRAM', fontsize=14, weight='bold', pad=15)

    # ── Left: full address space bar ──
    blocks = [
        ('Block 0\nCode\n0x00000000', 0.5, '#E8E8E8'),
        ('Block 1\nSRAM\n0x20000000', 1.5, C_LGREEN),
        ('Block 2\nPeripheral\n0x40000000', 2.5, '#E8E8E8'),
        ('Block 3\nFSMC\nBank1-2\n0x60000000', 3.5, C_LBLUE),
        ('Block 4\nFSMC\nBank3-4\n0x80000000', 4.5, C_LBLUE),
        ('Block 5-7\n0xA0000000\n~0xFFFFFFFF', 6, '#E8E8E8'),
    ]

    x_left = 0.5
    bar_w = 2.5
    for (label, y_center, color) in blocks:
        h = 0.9 if y_center < 6 else 1.8
        y = y_center + 2
        r = FancyBboxPatch((x_left, y - h/2), bar_w, h, boxstyle="round,pad=0.05",
                           facecolor=color, edgecolor='#999', lw=1)
        ax.add_patch(r)
        ax.text(x_left + bar_w/2, y, label, ha='center', va='center', fontsize=7)

    ax.text(x_left + bar_w/2, 1.5, 'Cortex-M4\n4GB 地址空间', ha='center', fontsize=10,
            weight='bold', color=C_DGRAY)

    # ── Middle: zoom arrow ──
    ax.annotate('', xy=(4.5, 5.5), xytext=(3.2, 5.5),
                arrowprops=dict(arrowstyle='->', color=C_BLUE, lw=2.5))
    ax.text(3.85, 5.9, '展开\nBank1', ha='center', fontsize=8, color=C_BLUE, weight='bold')

    # ── Right: Bank1 detail ──
    ne_blocks = [
        ('NE1', '0x60000000', '0x63FFFFFF', '#D6E9F8'),
        ('NE2', '0x64000000', '0x67FFFFFF', '#C5DEF5'),
        ('NE3', '0x68000000', '0x6BFFFFFF', '#B4D3F2'),
        ('NE4', '0x6C000000', '0x6FFFFFFF', C_LGREEN),
    ]

    x_right = 5.0
    rw = 3.0
    for i, (name, addr_start, addr_end, color) in enumerate(ne_blocks):
        y = 7.5 - i * 1.5
        r = FancyBboxPatch((x_right, y - 0.5), rw, 1.0, boxstyle="round,pad=0.05",
                           facecolor=color, edgecolor=C_BLUE, lw=1.5)
        ax.add_patch(r)
        ax.text(x_right + 0.3, y, f'{name}', ha='left', va='center', fontsize=11, weight='bold')
        ax.text(x_right + rw - 0.2, y + 0.15, addr_start, ha='right', va='center', fontsize=8, family='monospace')
        ax.text(x_right + rw - 0.2, y - 0.2, f'~ {addr_end}', ha='right', va='center', fontsize=7,
                family='monospace', color=C_GRAY)
        ax.text(x_right + rw + 0.2, y, '64 MB', ha='left', va='center', fontsize=8, color=C_GRAY)

    # Highlight NE4 → external SRAM
    ax.annotate('', xy=(10, 4.5), xytext=(8.5, 4.5),
                arrowprops=dict(arrowstyle='->', color=C_GREEN, lw=2.5))

    # External SRAM box
    sram = FancyBboxPatch((10, 3.5), 1.8, 2.0, boxstyle="round,pad=0.15",
                          facecolor=C_LGREEN, edgecolor=C_GREEN, lw=2.5)
    ax.add_patch(sram)
    ax.text(10.9, 5.0, 'SRAM', ha='center', fontsize=10, color=C_GREEN)
    ax.text(10.9, 4.5, 'IS62WV51216', ha='center', va='center', fontsize=9, weight='bold')
    ax.text(10.9, 4.0, '1 MB SRAM', ha='center', va='center', fontsize=9, color=C_GRAY)

    # Annotation
    ax.text(5.0, 2.0,
            'CPU 写 *((uint16_t*)0x6C000000) = 0x1234\n'
            '   → FSMC 自动产生 A[18:0] + CS# + WE# 时序\n'
            '   → 数据写入外部 SRAM',
            fontsize=10, color=C_DGRAY,
            bbox=dict(boxstyle='round,pad=0.5', facecolor='#FFFFF0', edgecolor=C_ORANGE, lw=1.5))

    save(fig, 's23_fsmc_address_map.png')

# ══════════════════════════════════════════════════════════════
# §2.3  FSMC 寄存器速查表
# ══════════════════════════════════════════════════════════════
def gen_s23_fsmc_registers():
    fig, ax = plt.subplots(figsize=(12, 4.5))
    ax.axis('off')

    cols = ['寄存器', '关键位域', '功能说明']
    rows = [
        ['FSMC_BCRx', 'MBKEN, MTYP, MWID,\nWREN, EXTMOD', 'Bank 控制\n(存储器类型/宽度/写使能)'],
        ['FSMC_BTRx', 'ADDSET[3:0], DATAST[7:0],\nBUSTURN, ACCMOD', '读时序\n(地址建立/数据建立/访问模式)'],
        ['FSMC_BWTRx', '同 BTRx', '写时序\n(扩展模式下独立配置)'],
    ]
    header_colors = [C_LGRAY, C_LGRAY, C_LGRAY]
    cell_colors = [
        [C_LBLUE, '#F0F7FF', '#F0F7FF'],
        [C_LORANGE, '#FFF8EC', '#FFF8EC'],
        [C_LGREEN, '#F0FFF0', '#F0FFF0'],
    ]
    table = ax.table(cellText=rows, colLabels=cols, cellLoc='center', loc='center',
                     cellColours=cell_colors, colColours=header_colors)
    table.auto_set_font_size(False)
    table.set_fontsize(10)
    table.scale(1, 2.5)
    for (r, c), cell in table.get_celld().items():
        cell.set_edgecolor('#CCCCCC')
        if r == 0: cell.set_text_props(weight='bold', fontsize=11)
        if c == 0: cell.set_text_props(weight='bold')

    ax.set_title('FSMC 关键寄存器速查', fontsize=13, weight='bold', pad=20)
    ax.text(0.5, -0.05, '* ADDSET / DATAST 的单位是 HCLK 周期（非纳秒）！168MHz 下 1 HCLK ≈ 5.95ns',
            transform=ax.transAxes, ha='center', fontsize=9, color=C_RED, style='italic')
    save(fig, 's23_fsmc_registers.png')

# ══════════════════════════════════════════════════════════════
# §2.4  FSMC SRAM 引脚分配表
# ══════════════════════════════════════════════════════════════
def gen_s24_fsmc_pinout():
    fig, axes = plt.subplots(1, 3, figsize=(14, 9))

    # ── Address pins ──
    addr_data = [
        ['A0',  'PF0'],  ['A1',  'PF1'],  ['A2',  'PF2'],  ['A3',  'PF3'],
        ['A4',  'PF4'],  ['A5',  'PF5'],  ['A6',  'PF12'], ['A7',  'PF13'],
        ['A8',  'PF14'], ['A9',  'PF15'], ['A10', 'PG0'],  ['A11', 'PG1'],
        ['A12', 'PG2'],  ['A13', 'PG3'],  ['A14', 'PG4'],  ['A15', 'PG5'],
        ['A16', 'PD11'], ['A17', 'PD12'], ['A18', 'PD13'],
    ]
    ax = axes[0]
    ax.axis('off')
    ax.set_title('地址线 A0~A18\n(19 根)', fontsize=11, weight='bold', color=C_BLUE)
    table = ax.table(cellText=addr_data, colLabels=['FSMC', 'GPIO'], cellLoc='center', loc='center',
                     colColours=[C_LBLUE, C_LBLUE])
    table.auto_set_font_size(False)
    table.set_fontsize(9)
    table.scale(1, 1.35)
    for (r, c), cell in table.get_celld().items():
        cell.set_edgecolor('#CCCCCC')
        if r == 0: cell.set_text_props(weight='bold')

    # ── Data pins ──
    data_data = [
        ['D0',  'PD14'], ['D1',  'PD15'], ['D2',  'PD0'],  ['D3',  'PD1'],
        ['D4',  'PE7'],  ['D5',  'PE8'],  ['D6',  'PE9'],  ['D7',  'PE10'],
        ['D8',  'PE11'], ['D9',  'PE12'], ['D10', 'PE13'], ['D11', 'PE14'],
        ['D12', 'PE15'], ['D13', 'PD8'],  ['D14', 'PD9'],  ['D15', 'PD10'],
    ]
    ax = axes[1]
    ax.axis('off')
    ax.set_title('数据线 D0~D15\n(16 根)', fontsize=11, weight='bold', color=C_GREEN)
    table = ax.table(cellText=data_data, colLabels=['FSMC', 'GPIO'], cellLoc='center', loc='center',
                     colColours=[C_LGREEN, C_LGREEN])
    table.auto_set_font_size(False)
    table.set_fontsize(9)
    table.scale(1, 1.35)
    for (r, c), cell in table.get_celld().items():
        cell.set_edgecolor('#CCCCCC')
        if r == 0: cell.set_text_props(weight='bold')

    # ── Control pins ──
    ctrl_data = [
        ['NWE\n(写使能)',   'PD5'],
        ['NOE\n(读使能)',   'PD4'],
        ['NE4\n(片选)',     'PG12'],
        ['NBL0\n(低字节)',  'PE0'],
        ['NBL1\n(高字节)',  'PE1'],
    ]
    ax = axes[2]
    ax.axis('off')
    ax.set_title('控制线\n(5 根)', fontsize=11, weight='bold', color=C_ORANGE)
    table = ax.table(cellText=ctrl_data, colLabels=['FSMC', 'GPIO'], cellLoc='center', loc='center',
                     colColours=[C_LORANGE, C_LORANGE])
    table.auto_set_font_size(False)
    table.set_fontsize(9)
    table.scale(1, 2.8)
    for (r, c), cell in table.get_celld().items():
        cell.set_edgecolor('#CCCCCC')
        if r == 0: cell.set_text_props(weight='bold')

    fig.suptitle('FSMC SRAM 完整引脚分配表（全部 AF12）', fontsize=13, weight='bold')
    fig.text(0.5, 0.01, '共 40 个 GPIO，涉及 PD / PE / PF / PG 四个端口，全部配置为 AF12 复用推挽高速',
             ha='center', fontsize=9, color=C_GRAY)
    fig.tight_layout(rect=[0, 0.03, 1, 0.95])
    save(fig, 's24_fsmc_pinout.png')


# ══════════════════════════════════════════════════════════════
#   Main
# ══════════════════════════════════════════════════════════════
if __name__ == '__main__':
    print("正在生成 Chapter 4 全部插图...")
    gen_s11_comparison_table()
    gen_s12_three_elements()
    gen_s12_direct_vs_fifo()
    gen_s13_dma_block_diagram()
    gen_s13_dma_request_map()
    gen_s13_dma_transfer_flow()
    gen_s14_timing_comparison()
    gen_s21_storage_overview()
    gen_s22_sram_timing()
    gen_s23_fsmc_address_map()
    gen_s23_fsmc_registers()
    gen_s24_fsmc_pinout()
    print(f"\n全部完成！图片保存在 {OUT}/")
