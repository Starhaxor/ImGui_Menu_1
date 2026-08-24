# HackManager ve Offset Rehberi

Bu dosya, projenin nasıl çalıştığını ve yeni hack / pointer zinciri eklemenin nasıl yapıldığını
sıfır bilgi varsayarak anlatır. Sonuna kadar okursan kendi hack'ini ekleyebilirsin.

---

## 1. Buyuk Resim: Proje Nasil Calisiyor?

```
+----------------+         +------------------+         +----------------------+         +----------------+
|    menu.cpp    | yazar   |     config.h     | kopya   |     HackManager      | okur/   |   Oyun bellegi |
| (arayuz, tik)  | ------->|    g_config      | ------> |  SyncFromConfig()    | yazar   |  (Pointer.cpp) |
|                |         |  (ayarlarin)     |         |  Update()            |         |                |
+----------------+         +------------------+         +----------------------+         +----------------+
```

Oyun acikken her frame'de (saniyede ~60 kez) sunlar olur:

1. Menudeki bir kutucuga tik atarsin -> ornegin `g_config.noRecoil = true` olur.
2. `overlay.cpp`, `CHackManager::Get().SyncFromConfig(g_config)` cagirir ->
   ayarlarin HackManager icindeki degiskenlere kopyalanir.
3. `Update(...)` calisir -> acik olan HER hack icin `ApplyXxx()` metodu devreye girer.
4. `ApplyXxx()` metotlari `CPointer` uzerinden oyunun bellegine deger yazar.

Yani senin tek yaptigin sey menude tik atmak. Tik -> config -> HackManager -> bellek.
Zincir otomatik isler.

### Dosya Haritasi

| Dosya               | Gorevi                                                              |
|---------------------|---------------------------------------------------------------------|
| `menu.cpp`          | Arayuz. Kutucuklar, sliderlar, butonlar. `g_config`'e yazar.         |
| `config.h`          | Tum ayar alanlarinin listesi (checkbox/slider degerleri).            |
| `HackManager.h/cpp` | Beyin. Hangi hack aciksa ne yapacagini bilir.                        |
| `Pointer.h/cpp`     | Eller. Bellege okuma/yazma yapan arac sinifi (`CPointer`).           |
| `Pointers.h`        | Oyunun modul basi adresi: `BASE_ADDRESS`.                            |
| `overlay.cpp`       | DirectX hook. Her frame'de donguyu baslatan yer.                     |

---

## 2. Hazine Avi Benzetmesi

Oyundaki bir degeri (ornegin canini) belleginde bulmak bir hazine avi gibidir:

- **BASE_ADDRESS** = oyunun ana programinin bellegdeki baslangic adresi.
  Oyun HER ACILISTA farkli yere yuklenir, bu yuzden sabit adres kullanamazsin,
  hep bu tabandan hesap yaparsin.
- **Statik adres** (tek offset) = `BASE + 0x12345678`. Oyun kapana kadar sabittir.
  Cheat Engine'de **yesil** gorunur.
- **Pointer** = icinde baska bir adres yazan kutu.
- **Deref** (cozumleme) = kutuyu acip icindeki adresi okumak.
- **Offset** = "su kadar ileri git" demek.

Neden zincire ihtiyac var? Cunku oyunlar dinamik bellek kullanir: can degeri her acilista
farkli adreste durabilir. AMA ona ulasan YOL hep aynidir: sabit bir kutudan baslayip
sirayla kutularin icine baktiginda sonunda hep ayni turda bir yere cikarsin.

---

## 3. Tek Offset vs Coklu Offset

Offsetleri `AddPointer`'a suslu parantez icinde virgulle yazarsin.
**Kac eleman yazarsan zincir o kadar derin olur.**

```cpp
// TEK HALKA -> statik adres. Icine bakilmaz, veri direkt oradadir.
m_pPointer->AddPointer("FallDamage", CPointers::BASE_ADDRESS, { 0x12345687 }, 1);

// UC HALKA -> pointer zinciri. Aradaki halkalar deref edilir.
m_pPointer->AddPointer("PlayerPosition", CPointers::BASE_ADDRESS, { 0x1234567A, 0x10, 0x20 }, 1);
```

### Ezberlenecek 3 Kural

1. **Ilk offset**: dogrudan `BASE`'e eklenir. Henuz icine bakilmaz.
2. **Sonraki her offset**: once o anki adresin ICINDEKI 8 bayt okunur (deref), sonra offset eklenir.
3. **En son ulasilan adres** = verinin ta kendisi. Artik icine bakilmaz, direkt okunur/yazilir.

---

## 4. Sayisal Ornek: `{ 0x1234567A, 0x10, 0x20 }`

Diyelim ki bu acilista oyun `0x7FF600000000` adresine yuklendi:

| Adim | Islem                                                        | Sonuc                       |
|------|--------------------------------------------------------------|-----------------------------|
| 1    | `0x7FF600000000 + 0x1234567A`                                | Adres **A** = `0x7FF61234567A` (statik kutu) |
| 2    | A'nin icindeki 8 bayti oku -> diyelim `0x02AB0000`; `+0x10`  | Adres **B** = `0x02AB0010`  |
| 3    | B'nin icindeki 8 bayti oku -> diyelim `0x05CD4000`; `+0x20`  | Adres **C** = `0x05CD4020`  |
| 4    | **C = verinin yasadigi yer**                                 | float x,y,z buradan okunur  |

Bu, Cheat Engine'deki su gosterimin birebir aynisidir:

```
[["game.exe+1234567A]+10]+20
```

Oyunu kapatip acsaydin A/B/C'nin DEGERLERI degisirdi ama YOL ayni kalirdi.
Iste bu yuzden offsetleri bir kere bulmak yeter.

---

## 5. Tip Parametresi (sondaki sayi)

Zincirin sonunda ulastigin adresten **kac bayt ve hangi bicimde** okunacagini soyler:

| Tip | Anlami          | Boyut   | Ne zaman kullanilir                    |
|-----|------------------|---------|----------------------------------------|
| `0` | int             | 4 bayt  | mermi sayisi, skor, flag'ler           |
| `1` | float           | 4 bayt  | can, zirh, hasar, pozisyon             |
| `2` | bool            | 1 bayt  | acik/kapiti durumlar                   |
| `3` | string          | 256 bayt| oyuncu adi gibi metinler               |
| `4` | qword / isaretci| 8 bayt  | icinde ADRES tutan degerler (entity liste gibi) |

Tip 4 ornegi (kodda mevcut):

```cpp
m_pPointer->AddPointer("EntityList", CPointers::BASE_ADDRESS, { OFF_ENTITY_LIST }, 4);
```

---

## 6. Offsetleri Cheat Engine ile Bulma (kisa yol)

1. Degere gore ara: mermin 17 ise First Scan 17, ates et, Next Scan 16...
2. Adres yesilse (statik): tek offset'in hazir. `game.exe+XXXX` seklindeki kismini yaz.
3. Adres siyahsa (dinamik): sag tik -> **Pointer scan for this address**.
4. Oyunu kapatip actiktan sonra hala gecerli olan sonucu sec.
   Base'i yesil adres olan sonucu istiyoruz.
5. Sonuctaki offsetleri **ayni sirayla** suslu paranteze yaz:
   CE'de `[["game.exe+02A9C88]+18]+34` goruyorsan ->
   `{ 0x02A9C88, 0x18, 0x34 }`

DIKKAT: Offset sirasi cok onemli. CE'de ustten alta dogru okudugun sirayla aynen yaz.

---

## 7. Uygulamali: Sifirdan Yeni Hack Ekleme

Ornek: "Sonsuz Stamina" hack'i ekliyoruz. Stamina `float` tipinde ve
statik adreste diyelim: `game.exe+02A9D10`.

### Adim 1 - Ayar alani: `config.h`

```cpp
bool infiniteStamina = false;   // diger bool'larin yanina ekle
```

### Adim 2 - State ve metod bildirimi: `HackManager.h`

```cpp
public:
    bool IsInfStaminaEnabled() const { return m_infStamina; }   // getter
    void SetInfStamina(bool enabled) { m_infStamina = enabled; } // setter
    void ApplyInfStamina();                                      // apply metodu

private:
    bool m_infStamina = false;
```

### Adim 3 - HackManager.cpp (4 kucuk dokunma)

a) Sabitler bolumune offset:

```cpp
constexpr uintptr_t OFF_STAMINA = 0x02A9D10;
```

b) Constructor'a pointer kaydi:

```cpp
m_pPointer->AddPointer("Stamina", CPointers::BASE_ADDRESS, { OFF_STAMINA }, 1);
```

c) `SyncFromConfig` icine kopya:

```cpp
m_infStamina = cfg.infiniteStamina;
```

d) Apply metodunu yaz ve `Update()` icine cagri ekle:

```cpp
void CHackManager::ApplyInfStamina() {
    if (m_infStamina) {
        float stamina = 100.0f;
        m_pPointer->WritePointer("Stamina", &stamina);
    }
}
```

```cpp
void CHackManager::Update(bool allowInput) {
    ...
    ApplyInfStamina();   // listenin sonuna ekle
}
```

### Adim 4 - Menuye bagla: `menu.cpp`

Ilgili sekmede (ornegin Aim):

```cpp
ClassicCheckbox("Infinite Stamina", &g_config.infiniteStamina);
```

### Adim 5 - Derle (F7) ve oyunda test et.

Hepsi bu. Tik attigin anda: menu -> config -> SyncFromConfig -> Update -> ApplyInfStamina -> bellek.

### Coklu Offset'li versiyonu olsaydi?

Sadece 3b'deki satir farkli olurdu:

```cpp
// [["game.exe+02A9C88]+28]+14 adresinde diyelim:
m_pPointer->AddPointer("Stamina", CPointers::BASE_ADDRESS, { 0x02A9C88, 0x28, 0x14 }, 1);
```

Geri kalan HICBIR sey degismez. Guzelligi de bu: offsetleri bir kere tanimla,
kullanimi her yerde ayni.

---

## 8. Crash Olur Mu? Guvenlik

- Tum okuma/yazmalar `ReadProcessMemory` / `WriteProcessMemory` ile yapilir.
  Geçersiz adres verilirse islem sessizce `false` doner, program patlamaz.
- Ornegin `GetEntities()` icinde bir entity adresi oluyorsa o entity atlanir, digerlerine devam edilir.
- YINE DE: gecerli ama YANLIS adrese yazmak oyun verisini bozabilir (oyun kendi icinde crash edebilir).
  Offsetleri yazmadan once CE'de okuma yaparak dogrulayin.

---

## 9. Sik Yapilan Hatalar

| Hata                                        | Dogrusu                                              |
|---------------------------------------------|------------------------------------------------------|
| Offset sirasini ters yazmak                  | CE'deki siranin AYNISINI yaz                          |
| float adrese tip 0 (int) ile yazmak          | Tipe dikkat: float=1, int=0                           |
| Tek offset gereken yere zincir yazmak        | Yesil adresler tek elemanli olur: `{ 0x... }`         |
| Son halkadan sonra bir deref daha beklemek   | Son adres VERININ kendisidir, icine bakilmaz          |
| BASE yerine kendi DLL adresini kullanmak     | Hep `CPointers::BASE_ADDRESS` (oyunun modulu) kullan  |

---

## 10. Mini Cheat Sheet

```cpp
// 1) Tanimla (constructor icinde)
m_pPointer->AddPointer("Isim", CPointers::BASE_ADDRESS, { off0, off1, off2 }, tip);

// 2) Oku
int deger = 0;
bool ok = m_pPointer->ReadPointer("Isim", &deger);

// 3) Yaz
m_pPointer->WritePointer("Isim", &deger);

// 4) Isimsiz, tek seferlik zincir
m_pPointer->ReadPointerChain(CPointers::BASE_ADDRESS, { 0xA, 0xB }, &out, 1);

// 5) Ham bellek (N bayt)
uintptr_t adres = m_pPointer->GetAddress("Isim");
m_pPointer->ReadRaw(adres, &buffer, sizeof(buffer));

// 6) Efektif adres ogren (0 = cozumleme basarisiz)
uintptr_t adr = m_pPointer->GetAddress("Isim");
```
