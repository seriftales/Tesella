import os
import itertools
from PIL import Image, ImageDraw

# Yapboz mantıksal boyutu 150, taşma (overlap) payı her yönden 25 piksel
# Toplam maske boyutu: 150 + 25 + 25 = 200 piksel
GRID_SIZE = 150
OVERLAP = 25
MASK_SIZE = GRID_SIZE + (2 * OVERLAP)
RADIUS = 15 # Çıkıntı/Girinti yuvarlağının yarıçapı

def draw_piece(top, right, bottom, left):
    # Siyah arkaplan (Motorumuz siyahları şeffaf yapacak)
    img = Image.new('RGB', (MASK_SIZE, MASK_SIZE), color='black')
    draw = ImageDraw.Draw(img)

    # Temel kare alanı (Beyaz - Görünür olacak kısım)
    # Çerçeveyi tam merkeze oturtuyoruz (25'ten 175'e kadar)
    base_rect = [OVERLAP, OVERLAP, MASK_SIZE - OVERLAP, MASK_SIZE - OVERLAP]
    draw.rectangle(base_rect, fill='white')

    # Merkez koordinatları
    center_x = MASK_SIZE // 2
    center_y = MASK_SIZE // 2

    # ÜST KENAR (Top)
    if top == 1:
        # Çıkıntı (Beyaz daire ekle çerçevenin dışına)
        draw.ellipse([center_x - RADIUS, OVERLAP - RADIUS, center_x + RADIUS, OVERLAP + RADIUS], fill='white')
    elif top == -1:
        # Girinti (Siyah daire ekle çerçevenin içine)
        draw.ellipse([center_x - RADIUS, OVERLAP - RADIUS, center_x + RADIUS, OVERLAP + RADIUS], fill='black')

    # SAĞ KENAR (Right)
    if right == 1:
        draw.ellipse([MASK_SIZE - OVERLAP - RADIUS, center_y - RADIUS, MASK_SIZE - OVERLAP + RADIUS, center_y + RADIUS], fill='white')
    elif right == -1:
        draw.ellipse([MASK_SIZE - OVERLAP - RADIUS, center_y - RADIUS, MASK_SIZE - OVERLAP + RADIUS, center_y + RADIUS], fill='black')

    # ALT KENAR (Bottom)
    if bottom == 1:
        draw.ellipse([center_x - RADIUS, MASK_SIZE - OVERLAP - RADIUS, center_x + RADIUS, MASK_SIZE - OVERLAP + RADIUS], fill='white')
    elif bottom == -1:
        draw.ellipse([center_x - RADIUS, MASK_SIZE - OVERLAP - RADIUS, center_x + RADIUS, MASK_SIZE - OVERLAP + RADIUS], fill='black')

    # SOL KENAR (Left)
    if left == 1:
        draw.ellipse([OVERLAP - RADIUS, center_y - RADIUS, OVERLAP + RADIUS, center_y + RADIUS], fill='white')
    elif left == -1:
        draw.ellipse([OVERLAP - RADIUS, center_y - RADIUS, OVERLAP + RADIUS, center_y + RADIUS], fill='black')

    return img

def main():
    os.makedirs('assets/masks', exist_ok=True)
    states = [1, 0, -1]
    
    # 3x3x3x3 = 81 farklı kombinasyonu üret
    for t, r, b, l in itertools.product(states, repeat=4):
        filename = f"assets/masks/mask_{t}_{r}_{b}_{l}.png"
        img = draw_piece(t, r, b, l)
        img.save(filename)
        print(f"Uretildi: {filename}")

if __name__ == '__main__':
    main()