# Сборка

## Требования

- **CMake** ≥ 3.21, **Ninja** (рекомендуется).
- **Компилятор с C++23**: GCC 13+, Clang 16+, MSVC 19.38+ (VS 2022 17.8+).
- **Qt 6.7+** с модулями: `Quick`, `QuickControls2`, `Concurrent`, `Svg`,
  `LinguistTools`, а также `WebEngineQuick` + `WebChannel` (десктоп) **или**
  `WebView` (Android/iOS).

## Ядро и тесты (без Qt)

Быстрый путь для проверки бизнес-логики — не требует Qt:

```bash
cmake --preset core-only
cmake --build --preset core-only
ctest --preset core-only
```

## Десктоп (Windows / Linux)

```bash
cmake --preset desktop-release          # или: cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build --preset desktop-release
./build/GeoBizUzbekistan
```

Задайте ключ карты: `export GEOBIZ_GOOGLE_MAPS_API_KEY="AIza..."` (или введите в
приложении).

### Windows: .exe + установщик

```powershell
pwsh installer/windows/deploy_windows.ps1 -BuildDir build -Config Release
# → build/*.exe (установщик NSIS), build/*.zip (portable)
```

Автоматизировано в `.github/workflows/windows.yml` (сборка + `windeployqt` +
CPack; иконка `.ico` генерируется из SVG через ImageMagick).

### Linux: AppImage

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
DESTDIR=AppDir cmake --install build --prefix /usr
linuxdeploy --appdir AppDir --plugin qt --output appimage \
  --desktop-file flatpak/uz.geobiz.GeoBiz.desktop \
  --icon-file assets/icons/app.svg
```

### Linux: Flatpak

```bash
flatpak install flathub org.kde.Platform//6.7 org.kde.Sdk//6.7
flatpak-builder --user --install --force-clean build-flatpak \
  flatpak/uz.geobiz.GeoBiz.yml
flatpak run uz.geobiz.GeoBiz
```

## Android (APK / AAB)

Нужны: JDK 17, Android SDK/NDK, desktop-Qt (хост-инструменты) и Android-Qt
(`android_arm64_v8a`) с модулем `qtwebview`.

```bash
$QT_ROOT_DIR/bin/qt-cmake -S . -B build-android -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DANDROID_ABI=arm64-v8a \
  -DQT_HOST_PATH="$QT_HOST_ROOT_DIR"
cmake --build build-android --target apk    # APK
cmake --build build-android --target aab    # AAB (для Google Play)
```

Автоматизировано в `.github/workflows/android.yml`. Манифест, gradle и иконка —
в каталоге `android/`.

> **Примечание.** На Android карта работает через `QtWebView` (системный
> web-view), а не `QtWebEngine` — последний на Android недоступен. Логика и
> страница карты общие; отличается только транспорт моста (см.
> [ARCHITECTURE.md](ARCHITECTURE.md)).

## Полезные опции CMake

| Опция | По умолчанию | Назначение |
|-------|--------------|------------|
| `GEOBIZ_BUILD_TESTS` | `ON` | Собирать модульные тесты ядра |
| `GEOBIZ_CORE_ONLY`   | `OFF` | Только ядро + тесты, без Qt-приложения |
