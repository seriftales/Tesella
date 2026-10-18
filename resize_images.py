import os
from PIL import Image

def make_square_and_resize(input_dir="assets/images/puzzle", target_size=600):
    if not os.path.exists(input_dir):
        print(f"Hata: '{input_dir}' klasörü bulunamadı!")
        return

    # Desteklenen görsel uzantıları
    valid_extensions = ('.png', '.jpg', '.jpeg', '.bmp')
    
    count = 0
    for filename in os.listdir(input_dir):
        if not filename.lower().endswith(valid_extensions):
            continue

        img_path = os.path.join(input_dir, filename)
        try:
            with Image.open(img_path) as img:
                # RGBA modundaki PNG'leri RGB'ye çevir (JPG uyumluluğu için)
                if img.mode in ('RGBA', 'P'):
                    img = img.convert('RGB')

                width, height = img.size
                
                # 1:1 Kare Kırpma (Center Crop) matematiği
                if width > height:
                    left = (width - height) / 2
                    top = 0
                    right = (width + height) / 2
                    bottom = height
                else:
                    left = 0
                    top = (height - width) / 2
                    right = width
                    bottom = (height + width) / 2

                # Resmi ortasından kare olacak şekilde kırp
                img_cropped = img.crop((left, top, right, bottom))
                
                # Standart boyuta (örn: 600x600) pürüzsüzçe ölçekle (LANCZOS filtresiyle kalite kaybını önle)
                img_resized = img_cropped.resize((target_size, target_size), Image.Resampling.LANCZOS)
                
                # Üzerine yazarak kaydet (veya orijinal kalitesi düşmesin diye .jpg olarak kaydet)
                base_name = os.path.splitext(filename)[0]
                output_path = os.path.join(input_dir, f"{base_name}.jpg")
                
                img_resized.save(output_path, "JPEG", quality=95)
                print(f"Dönüştürüldü ve Kare Yapıldı: {filename} -> {base_name}.jpg ({target_size}x{target_size})")
                count += 1
        except Exception as e:
            print(f"Hata olustu ({filename}): {e}")

    print(f"\nİşlem tamam! Toplam {count} görsel 1:1 kare standartına getirildi.")

if __name__ == "__main__":
    make_square_and_resize()