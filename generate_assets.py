import wave
import struct
import math
from PIL import Image, ImageDraw

def generate_debug_image():
    size = 800
    grid_count = 4
    step = size // grid_count
    
    # Saydamlık (Alpha) destekli RGBA formatında boş bir tuval oluştur
    img = Image.new('RGBA', (size, size), (220, 220, 220, 255))
    draw = ImageDraw.Draw(img)
    
    for row in range(grid_count):
        for col in range(grid_count):
            x1 = col * step
            y1 = row * step
            x2 = x1 + step
            y2 = y1 + step
            
            # Siyah çerçeveli kareler çiz
            draw.rectangle([x1, y1, x2, y2], outline=(50, 50, 50, 255), width=3)
            
            # Karenin numarasını hesapla (1'den 16'ya kadar)
            number = str(row * grid_count + col + 1)
            
            # Numarayı kabaca ortaya hizala (Font yüklemeden basit çizim)
            text_x = x1 + (step // 2) - 10
            text_y = y1 + (step // 2) - 10
            draw.text((text_x, text_y), number, fill=(0, 0, 0, 255))
            
    img.save('assets/images/debug_grid.png')
    print("Test görseli oluşturuldu: assets/images/debug_grid.png")

def generate_snap_sound():
    sample_rate = 44100
    duration = 0.05 # 50 milisaniyelik çok kısa bir ses
    
    with wave.open('assets/sounds/snap.wav', 'w') as wav_file:
        wav_file.setnchannels(1) # Mono
        wav_file.setsampwidth(2) # 16-bit
        wav_file.setframerate(sample_rate)
        
        for i in range(int(sample_rate * duration)):
            t = float(i) / sample_rate
            # Hızlıca düşen bir frekans (logaritmik) kullanarak tok bir "tık" sesi elde etme
            freq = 1500 * math.exp(-80 * t) 
            amplitude = math.exp(-40 * t) # Sesin sonuna doğru kısılarak bitmesi (Envelope)
            
            value = int(32767 * amplitude * math.sin(2.0 * math.pi * freq * t))
            data = struct.pack('<h', value)
            wav_file.writeframesraw(data)
            
    print("Snap ses efekti oluşturuldu: assets/sounds/snap.wav")

if __name__ == "__main__":
    generate_debug_image()
    generate_snap_sound()
