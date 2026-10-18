# Tesella Yapboz Oyunu

**Tesella**, modern C++ , SDL2 ve Dear ImGui kullanılarak sıfırdan geliştirilmiş, çapraz platform (Linux & Windows) destekli 2B bir yapboz oyunudur. Dinamik parça kesimi, kullanıcı profili tabanlı kayıt sistemi ve özelleştirilebilir görsel galerisi ile hem oyuncular hem de geliştiriciler için temiz bir oyun motoru mimarisi sunar.

---

## Temel Özellikler

* **Dinamik Yapboz Üretimi:** Seçilen herhangi bir görseli çalışma zamanında gerçek yapboz girinti ve çıkıntılarına sahip parçalara böler.
* **Ayarlanabilir Zorluk Seviyesi:** 2x2'den  8x8'e kadar dinamik ızgara boyutu seçimi.
* **Kendi Görsellerinle Oynama Desteği:** `assets/images/puzzle/` dizinine atılan `.png`, formatındaki resimleri otomatik olarak tarar ve oyun içi galeride önizlemeli olarak listeler.
* **Profil ve Kayıt Sistemi:** Kullanıcı adına özel oyun kaydetme.Kilitlenen parçaları, parçaların anlık koordinatlarını, kenar yapılarını ve geçen süreyi kaydederek oyuna kaldığınız yerden devam etme imkanı sağlar.
* **Oyun İçi Yardımcılar ve Efektler:** İpucu resmi  gösterme/gizleme, parçalar doğru yere oturduğunda mıknatıslanma ve sesli geri bildirim.

---

## İndirme ve Oynama 

Oyunu kaynak koddan derlemeden doğrudan oynamak isteyen Windows kullanıcıları için hazır çalıştırılabilir paket yayınlanmıştır:

1. Sağ taraftaki **[Releases](../../releases)** sekmesine gidin.
2. En güncel sürümün altındaki **`Tesella_v.1.0_Windows.zip`** dosyasını indirin.
3. ZIP arşivini klasöre çıkartın ve içerisindeki **`tesella.exe`** dosyasına çift tıklayarak oyuna başlayın. *(Gerekli tüm `.dll` bağımlılıkları pakete dahildir, ek kurulum gerektirmez).*

---

## Geliştiriciler İçin Kurulum ve Derleme

Projeyi kendi sisteminizde derlemek ve geliştirmek için aşağıdaki adımları takip edebilirsiniz.

### Gereksinimler
* **C++ Derleyicisi:** C++17 destekli bir derleyici 
* **Yapılandırma Aracı:** CMake 
* **Bağımlılıklar:** `SDL2`, `SDL2_image`, `SDL2_mixer`, `Dear ImGui` 

### Linux (Ubuntu / Debian) Kurulum ve Çalıştırma Adımları

**1. Gerekli kütüphaneleri ve derleme araçlarını kurun:**

```bash
sudo apt update
sudo apt install build-essential cmake libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev
```
**2. Projeyi klonlayın ve dizine girin:**

```bash
git clone [https://github.com/seriftales/Tesella.git](https://github.com/seriftales/Tesella.git)
cd Tesella
```
**3. CMake ile projeyi derleyin:**

```bash
mkdir build
cd build
cmake ..
make
```
**4. Oyunu çalıştırın:**

```bash
./tesella
```

### Windows Derleme Adımları
Windows ortamında bağımlılıkları yönetmek için vcpkg kullanılmaktadır:

**1. Gerekli kütüphaneleri vcpkg ile kurun:**
```bash 
vcpkg install sdl2:x64-windows sdl2-image:x64-windows sdl2-mixer:x64-windows
```

**2. Derleme klasörünü oluşturun ve CMake ile yapılandırın:**

```bash
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
```

**3. Release modunda derleyin:**
```bash
cmake --build . --config Release
```

### Proje Dizin Yapısı 
```text
Tesella/
├── assets/
│   ├── images/
│   │   ├── puzzle/              # Yapboz olarak oynanacak görsellerin eklendiği klasör
│   │   └── menu.png             # Ana menü arka plan görseli
│   └── sounds/
│       ├── menu.wav             # Ana menü arka plan müziği
│       ├── snap.wav             # Parça yerine oturduğunda çalan ses efekti
│       └── victory.wav          # Yapboz tamamlandığında çalan zafer sesi
├── include/
│   ├── game_engine.h            # Oyun döngüsü, durum yönetimi ve ImGui arayüz tanımları
│   ├── input_handler.h          # Fare etkileşimleri ve parça sürükleme/kilitleme mantığı
│   ├── puzzle_generator.h       # Görselleri parçalara ayırma ve kenar maskeleme algoritması
│   └── puzzle_piece.h           # Tekil yapboz parçası sınıfı ve veri yapısı
├── src/
│   ├── main.cpp                 # Uygulama giriş noktası 
│   ├── game_engine.cpp          # Oyun motoru, kayıt/yükleme ve arayüz implementasyonu
│   ├── input_handler.cpp        # Giriş işleme implementasyonu
│   ├── puzzle_generator.cpp     # Parça üretim algoritması implementasyonu
│   └── puzzle_piece.cpp         # Parça çizim  ve konum kontrol implementasyonu
├── saves/                       # Oyuncu profillerinin  otomatik kaydedildiği dizin
├── CMakeLists.txt               # CMake derleme yapılandırma dosyası
└── README.md                    # Proje dokümantasyonu
```
